'''
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
'''

import sys
import traceback
import uvudec

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


print
try:
	print 'out return test: %s' % uvudec.returns_string()
except:
	print 'ERROR: failed out string test'
	traceback.print_exc(file=sys.stdout)

try:
	print 'output return test: %s' % uvudec.returns_string_output()
except:
	print 'ERROR: failed output string test'
	traceback.print_exc(file=sys.stdout)

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

print
if False:
	# Manually wrapped
	print 'Manually wrapped'
	uvd = uvudec.uvd.getUVDFromFileName('/home/mcmaster/document/build/uvudec/candela_pltl1_rev_3.bin')
	print 'got uvd'
	print type(uvd)
else:
	# Automatically wrapped
	print 'Automatically wrapped'
	uvd = uvudec.UVD.getUVDFromFileName('/home/mcmaster/document/build/uvudec/candela_pltl1_rev_3.bin')
	print 'got UVD'
	print type(uvd)

try:
	disassembly = uvd.disassemble()
	print 'Total length: %d' % len(disassembly)
	print 'Sample:'
	print disassembly[0:min(200, len(disassembly))]

except:
	print 'failed to disassemble'
	traceback.print_exc(file=sys.stdout)


