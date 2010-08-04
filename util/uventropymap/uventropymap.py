#!/usr/bin/python
'''
uvstructoffset: structure offset computer
Plot entropy map
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Released under version 3 GPL+
Dependencies:
	gnuplot
		http://code.google.com/p/pycparser/
'''

import pefile
import sys
import Gnuplot, Gnuplot.funcutils
import math

g_debug = False

class EntropyMapper:
	def __init__(self):
		self.fileName = None

	def mapEntropy(self, fileName):
		print 'Mapping: %s' % fileName
		self.fileName = fileName
		pe = pefile.PE(fileName)
		for section in pe.sections:
			sectionName = section.Name.split('\x00')[0]
			print 'Section: %s' % sectionName
			data = section.data
			
			points = list()
			
			# Divide into blocks
			for i in range(0, len(data), 0x100):
				dataSubset = data[i : i+0x100]
				points.append((i, self.calculateEntropy(dataSubset)))
			
			self.plot(points, sectionName)

		raw_input('Please press return to continue...\n')
	
	def frequencies(self, data):
		ret = [0] * 256
		for b in data:
			ret[ord(b)] += 1
		return ret
	
	def calculateEntropy(self, data):
		'''
		http://stackoverflow.com/questions/990477/how-to-calculate-the-entropy-of-a-file
		Shannon's entropy
		'''
		
		frequencies = self.frequencies(data)
		
		# Must divide by 8 to get in range 0...1
		entropy = 0.0
		for freq in frequencies: 
			p = 1.0 * freq / len(data)
			# More for the approx case if p ~= 0
			if p > 0:
				# lgN is the logarithm with base 2
				entropy -= p * math.log(p, 2)
		return entropy
		
	def plot(rangeXAxis, entropyPoints, title):
		global g_plots
		
		#plotPoints = list()
		#for point in entropyPoints:
		#	plotPoints.append(point)
		plotPoints = entropyPoints

		g = Gnuplot.Gnuplot(debug=g_debug)
		#g('set logscale y 10')
		g('set title "%s"' % title)
		g('set xlabel "Address"')
		g('set format x "%X"')
		#g('set label "test" at "25/8/93",1')

		# Otherwise it gets scaled to 300
		#g('set xrange [0:256]')
		g('set yrange [0:8]')
		#g('set xtics 0,16,256')
		g('set ylabel "Entropy"')
		# The plot
		g.plot(plotPoints)
		
		g_plots.add(g)
	
g_plots = set()

def usage():
	print 'Usage: %s <file name>' % sys.argv[0]

if __name__ == "__main__":
	fileNames = list()
	for i in range(1, len(sys.argv)):
		arg = sys.argv[i]
		if arg.find("--") < 0:
			fileNames.append(arg)
		elif arg == "--debug":
			g_debug = True
		else:
			print 'Unrecognized arg: %s' % arg
			usage()
			sys.exit(1)	
	
	if len(fileNames) < 1:
		usage()
		exit(0)
	
	for fileName in fileNames:
		mapper = EntropyMapper()
		mapper.mapEntropy(fileName)

