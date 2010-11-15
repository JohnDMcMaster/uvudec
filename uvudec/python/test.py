'''
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
'''

import sys
import traceback
import unittest
import uvudec

class UnitTest(unittest.TestCase):
	def setUp(self):
		print
		print 'setUp()'
		if False:
			# Manually wrapped
			print 'Manually wrapped'
			self.uvd = uvudec.uvd.getUVDFromFileName('/home/mcmaster/document/build/uvudec/candela_pltl1_rev_3.bin')
			print 'got uvd'
			print type(self.uvd)
		else:
			# Automatically wrapped
			print 'Automatically wrapped'
			self.uvd = uvudec.UVD.getUVDFromFileName('/home/mcmaster/document/build/uvudec/candela_pltl1_rev_3.bin')
			print 'got UVD'
			print type(self.uvd)
	
	def tearDown(self):
		print 'tearDown()'
		self.uvd = None

	'''
	def test_shuffle(self):
		# make sure the shuffled sequence does not lose any elements
		random.shuffle(self.seq)
		self.seq.sort()
		self.assertEqual(self.seq, range(10))

		# should raise an exception for an immutable sequence
		self.assertRaises(TypeError, random.shuffle, (1,2,3))

	def test_choice(self):
		element = random.choice(self.seq)
		self.assertTrue(element in self.seq)

	def test_sample(self):
		with self.assertRaises(ValueError):
			random.sample(self.seq, 20)
		for element in random.sample(self.seq, 5):
			self.assertTrue(element in self.seq)
	'''

	def test_always_return_rc(self):
		print
		try:
			uvudec.always_return_rc(0)
			print 'no exception as expected'
		except:
			print 'ERROR: should not have caught exception'
			traceback.print_exc(file=sys.stdout)

		try:
			uvudec.always_return_rc(-1)
			print 'ERROR: should not have gotten here'
		except:
			print 'caught exception as expected'

	def test_out_return(self):
		print
		try:
			print 'out return test: %s' % uvudec.returns_string()
		except:
			print 'ERROR: failed out string test'
			traceback.print_exc(file=sys.stdout)

	def test_output_return(self):
		try:
			print 'output return test: %s' % uvudec.returns_string_output()
		except:
			print 'ERROR: failed output string test'
			traceback.print_exc(file=sys.stdout)

	def test_string_return(self):
		try:
			print 'string return test: %s' % uvudec.returns_string_other()
			print 'should not be here'
		except:
			print 'failed other named string test as expected'

		try:
			uvudec.takes_string('down the hatch')
			print 'Took string'
		except:
			print 'ERROR: failed input string test'
			traceback.print_exc(file=sys.stdout)

	def test_disassemble(self):
		try:
			disassembly = uvd.disassemble()
			print 'Total length: %d' % len(disassembly)
			print 'Sample:'
			print disassembly[0:min(200, len(disassembly))]

		except:
			print 'failed to disassemble'
			traceback.print_exc(file=sys.stdout)

	def test_iterator(self):
		print
		try:
			itr = self.uvd.begin()
			while itr is not self.uvd.end() and itr.getPosition() < 0x10:
				print '0x%04X: %s' % (itr.getPosition(), itr.getCurrent())
				itr.next()

		except:
			print 'failed to iter disassemble'
			traceback.print_exc(file=sys.stdout)
			raise
	
if __name__ == '__main__':
	unittest.main()


