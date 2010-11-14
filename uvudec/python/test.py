import uvudec

try:
	uvudec.always_return_rc(0)
	print 'no exception as expected'
except:
	print 'ERROR: should not have caught exception'

try:
	uvudec.always_return_rc(-1)
	print 'ERROR: should not have gotten here'
except:
	print 'caught exception as expected'

try:
	print 'string return test: %s' % uvudec.returns_string()
except:
	print 'ERROR: failed string test'

if False:
	# Manually wrapped
	uvd = uvudec.uvd.getUVDFromFileName('/home/mcmaster/document/build/uvudec/candela_pltl1_rev_3.bin')
	print 'got uvd'
	print type(uvd)

if True:
	# Automatically wrapped
	uvd = uvudec.UVD.getUVDFromFileName('/home/mcmaster/document/build/uvudec/candela_pltl1_rev_3.bin')
	print 'got UVD'
	print type(uvd)

