#!/usr/bin/python
# uvbinstat: look for patterns in binary images
# Intended to find patterns indicating a bad rip
# Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>

"""
WARNING: assumes all ROMs are length multiples of 2
If they are not, will be truncated down to next lowest multiple

Thing to check for
Blank or nearly blank EPROM
	Repeating value for large portion of ROM
	Trick though...some companies use much larger EPROMs than they need
	Possibly because they can buy in bulk?
	Due to non-zero start vectors, starting area might also be blank
	So, try to find at least one cluster with lots of varying data
	Common blank values are FF (EPROM, flash?), 00, and 7F (PIC?)
27C in flash socket
	Observed from Quantum hard drive
	8 numbers with nonzero frequencies followed by 8 numbers with 0 frequencies
	256 bytes / 16 => 16 such blocks
	Might try several diff block sizes, although the large 0 freq areas should be distintive enough
Missing address pin
	Data will repeat in powers of 2
	Check each block of 2, discarding blank blocks
	See if they repeat
"""

import sys
import copy
import math

VERSION = "0.1.0"

plotAvailible = None
g_doPlot = False

g_binFileName = None
g_binFile = None
g_binString = None
g_bits = None

g_freqs = None
g_printInfo = True

# If there are not more than at least this many different bytes,
# the ROM will be considered mostly blank
g_mostlyBlankThreshold = 16

def printError(s):
	if g_binFileName is not None:
		print '%s: ' % (g_binFileName),
	print 'ERROR: %s' % (s)

def printInfo(s):
	if g_printInfo:
		print s

# End not included
def calcFreqsOverRange(start=0, end=None, data=None):
	freqsCur = [0] * 256
	
	if data is None:
		data = g_binString
	if end is None:
		end = len(data)
		
	for i in range(start, end):
		b = ord(data[i])
		freqsCur[b] += 1
	
	return freqsCur

def calcFreqs():
	global g_freqs
	g_freqs = calcFreqsOverRange()	

def printFreqs():
	global g_freqs
	for i in range(0, 256):
		b = g_freqs[i]
		print '0x%.2X: 0x%.4X' % (i, b)

def plotFreqs():
	plotPoints = list()
	for i in range(0, 256):
		b = g_freqs[i]
		# (x, y)
		plotPoints.append((i, b))

	g = Gnuplot.Gnuplot(debug=g_printInfo)
	g('set logscale y 10')
	g('set xlabel "Byte value"')
	# Otherwise it gets scaled to 300
	g('set xrange [0:256]')
	g('set xtics 0,16,256')
	g('set ylabel "log10 frequency"')
	# The plot
	g.plot(plotPoints)
	raw_input('Please press return to continue...\n')

# Absolutly blank: all the exact same number
def isCompletlyBlank(start=0, end=None, data=None):
	if data is None:
		data = g_binString
	if end is None:
		end = len(data)
	
	freqs = g_freqs
	
	# Purly blank?
	nonzeros = 0
	for freq in freqs:
		if freq > 0:
			# Did we get a second non-zero number?
			if nonzeros > 0:
				return False
			nonzeros += 1
	return True	

def blankCheck(start=0, end=None, data=None):
	if data is None:
		data = g_binString
	if end is None:
		end = len(data)
	
	# Purly blank?
	nonzeros = 0
	for freq in g_freqs:
		if freq > 0:
			nonzeros += 1
	zeros = 256 - nonzeros

	# We should prob have quite a big distribution
	# Random noise might set a few g_bits
	print 'Nonzreo frequency locations: %d / 256' % nonzeros
	print 'Zero frequency locations: %d / 256' % zeros
	# All the same?
	if nonzeros == 1:
		print 'ERROR: completly blank!'
		return False
	# Very poor randomness?
	if nonzeros <= g_mostlyBlankThreshold:
		printError('few distinct numbers, expect nearly blank')
		return False
	return True
	
def misrip27CCheck():
	"""
	Look for alternating blocks of 0's in the freq map
	There are a lot of patterns here, but this was the simplest:
		In quantum data, only half the data had anything represewnted at all in half block sizes of 8
	Also, it alternates these semi random looking sections (upper) with the highly repetative sections (lower)

	Excerpt:
	bin:
		000003f0  e3 e3 00 00 e4 e4 00 00  e5 e5 00 00 e6 e6 00 00  |................|
		00000400  04 06 04 06 04 06 04 06  04 06 04 06 04 06 04 06  |................|
		*
		00000500  14 16 14 16 14 16 14 16  14 16 14 16 14 16 14 16  |................|
		*
	frequency
		0xF0: 0x0072
		0xF1: 0x0042
		0xF2: 0x003E
		0xF3: 0x001E
		0xF4: 0x00D7
		0xF5: 0x0070
		0xF6: 0x00C7
		0xF7: 0x00D0
		0xF8: 0x0000
		0xF9: 0x0000
		0xFA: 0x0000
		0xFB: 0x0000
		0xFC: 0x0000
		0xFD: 0x0000
		0xFE: 0x0000
		0xFF: 0x0000
	"""

	for blockExponent in range(0, 8):
		halfBlockSize = 2**blockExponent
		fullBlockSize = halfBlockSize * 2
		
		# See if we have all 0's in all of the blocks
		lowerTotal = 0
		upperTotal = 0
		
		# Each expected alternating section
		# On the quantum set, first 8 had data, second 8 didn't
		for freqIndex in range(0, 256, fullBlockSize):
			for freqOffset in range (0, halfBlockSize):
				lowerTotal += g_freqs[freqIndex + freqOffset]
			for freqOffset in range (halfBlockSize, fullBlockSize):
				upperTotal += g_freqs[freqIndex + freqOffset]
		
		# Did we get an alternating block of size 0?
		if lowerTotal == 0 or upperTotal == 0:
			printError('expect did not position for rip correctly! (27C check)')
			printError('block size: %d' % fullBlockSize)
			return False

	return True

def missingAddressPinCheck():
	# Dice into chunks and see if we have repeats
	# A missing address pin will cause the same data to repeat over and over again
	# Increase in powers of 2 to check for each possible fault

	# Only check the first two blocks in each range
	# Assuming a clean rip, they should be perfectly repeated and other checks aren't necessary
	# Only check the rest if we expect a bad address pin
	# Exponent of 0 is blank check, so skip that
	adjacentMatchFrequencies = [0] * g_bits
	for blockExponent in range(1, g_bits):
		blockSize = 2**blockExponent
		
		matches = 0
		
		# Check adjacent blocks
		# If all match, we have an address pin issue
		# Blank sections shouldn't present any issues
		# Easier to just count
		checked = 0
		for blockIndex in range(blockSize, len(g_binString), blockSize):			
			lower = g_binString[blockIndex - blockSize:blockIndex]
			upper = g_binString[blockIndex:blockIndex + blockSize]
			checked += 1
			if lower == upper:
				matches += 1
		adjacentMatchFrequencies[blockExponent] = matches
		if matches == checked:
			printError('an address pin is probably missing')
			printError('repeated block size: 0x%.4X (%d), bits: 0x%.4X (%d)' % (blockSize, blockSize, int(math.log(blockSize, 2)), int(math.log(blockSize, 2))))

def help():
	version()
	print
	usage()

def version():
	print 'uvbinstat version %s' % (VERSION)
	print 'Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>'

def usage():
	print 'usage: %s <args>' % sys.argv[0]
	print 'Args:' 
	print "--help: this message"
	print "--plot-freqs: plot (using gnuplot) a byte frequency distribution"
	print "--print-freqs: print a byte frequency distribution"
	print "--quiet: suppress normal info and only print in event of error"
	print "--version: version info"
		

def main():
	global g_binFileName
	global g_binFile
	global g_binString
	global g_bits
	global g_doPlot
	
	binFileNames = list()
	doPrintFreqs = False
	
	for i in range(1, len(sys.argv)):
		arg = sys.argv[i]

		if arg.find("--") == 0:
			if arg == "--quiet":
				g_printInfo = False
			elif arg == "--plot-freqs":
				if not g_plotAvailible:
					printError('plotting not availible')
					return
				g_doPlot = True
			elif arg == "--print-freqs":
				doPrintFreqs = True
			elif arg == "--help":
				help()
				return
			elif arg == "--version":
				version()
				return
			else:
				printError('unknown arg: %s' % arg)
				help()
				return
		else:
			binFileNames.append(arg)

	for g_binFileName in binFileNames:
		printInfo('Checking %s...' % g_binFileName)
		g_binFile = open(g_binFileName, "rb")
		g_binString = g_binFile.read()
		printInfo('Length: 0x%.4X (%d)' % (len(g_binString), len(g_binString)))
		g_bits = int(math.log(len(g_binString), 2))
		
		calcFreqs()
		if doPrintFreqs:
			printFreqs()
		blankCheck()
		misrip27CCheck()
		missingAddressPinCheck()
		if g_doPlot:
			plotFreqs()

if __name__ == "__main__":
	try:
		import Gnuplot, Gnuplot.funcutils
		g_plotAvailible = True
	except ImportError:
		# Override any previous setting
		g_doPlot = False
		g_plotAvailible = False

	main()
