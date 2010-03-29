#!/usr/bin/env python
# uvbpEEPROM: rip an I2C EEPROM using a bus pirate
# Generates C++ interface suitble for DLL injection
# Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>

import sys
import timefrom pyBusPirate.BinaryMode.I2C import *from curses.ascii import isprint

def hexdump_half_row(data, start):
	g_bytesPerHalfRow = 8
	col = 0
	size = len(data)

	while col < g_bytesPerHalfRow and start + col < size:
		index = start + col
		sys.stdout.write("%.2X " % ord(data[index]))
		col = col + 1

	while col < g_bytesPerHalfRow:
		sys.stdout.write("   ")
		col = col + 1

	sys.stdout.write(" ")

	return start + g_bytesPerHalfRow

def hexdump(data):
	g_bytesPerRow = 16
	pos = 0
	size = len(data)
	while  pos < size:
		row_start = pos

		print '%.4X:  ' % pos,
		
		pos = hexdump_half_row(data, pos)
		pos = hexdump_half_row(data, pos)

		sys.stdout.write("|")

		i = row_start
		while i < row_start + g_bytesPerRow and i < size:
			c = data[i]
			if isprint(c):
				sys.stdout.write("%c" % c)
			else:
				sys.stdout.write("%c" % '.')
			i = i + 1
				
		for i in range(i, row_start + g_bytesPerRow):
			sys.stdout.write(" ")

		sys.stdout.write("|\n")

class EEPROMUtil:
	def __init__(self):
		self.i2c = None
	def i2c_write_data(self, data):		self.i2c.send_start_bit()		self.i2c.bulk_trans(len(data),data)		self.i2c.send_stop_bit()
	def i2c_read_bytes(self, address, numbytes):		print 'read from address 0x%.2X' % address[0]
		data_out=[]		self.i2c.send_start_bit()		self.i2c.bulk_trans(len(address), address)		while numbytes > 0:			data_out.append(self.i2c.read_byte())			self.i2c.send_ack()			numbytes -= 1		self.i2c.send_stop_bit()
		return data_out
	
	def readBytes(self, readAddress, numBytes):
		# return self.i2c_read_bytes([readAddress], numBytes)
			# like
		# {0xA0,0x00{0xA1,R:numBytes}
		writeAddress = readAddress - 1
		
		data_out = []
		
		self.i2c.send_start_bit()
		bulkBytes = [writeAddress, numBytes]
		print bulkBytes
		self.i2c.bulk_trans(2, bulkBytes)
		# rstart
		self.i2c.send_start_bit()
		self.i2c.bulk_trans(1, [readAddress])		
		# Eat 'em up!
		while numBytes > 0:			data_out.append(self.i2c.read_byte())			self.i2c.send_ack()			numBytes -= 1
		self.i2c.send_stop_bit()
		
		return data_out

	def setup(self):
		self.i2c = I2C("/dev/ttyUSB0", 115200)		
		print "Entering binmode: ",		if not self.i2c.BBmode():			raise "failed."		print "OK."		print "Entering raw I2C mode: ",		if not self.i2c.enter_I2C():			raise "failed."		print "OK."				print "Configuring self.i2c."		if not self.i2c.cfg_pins(I2CPins.POWER | I2CPins.PULLUPS):			raise "Failed to set I2C peripherals."
		if not self.i2c.set_speed(I2CSpeed._50KHZ):			raise "Failed to set I2C Speed."
		self.i2c.timeout(0.2)
		print 'Setup completed'

	def origTest(self):
		self.setup()
		writeAddress = 0xa0
		readAddress = writeAddress + 1
				print "Reading EEPROM."		self.i2c_write_data([writeAddress, 0,0, 1, 2,3,4,5,6,7,8,9])		
		self.i2c_write_data([writeAddress, 0,0])		self.i2c_read_bytes([readAddress],5)		print "Reset Bus Pirate to user terminal: "		if not self.i2c.resetBP():			raise "failed."
	# 24CXX where XX is arg
	# XX is how many k bits it is
	def dumpEEPROM24(self, writeAddress, chipNo):
		self.dumpEEPROMBytes(writeAddress, chipNo * 1024 / 8)

	def dumpEEPROMBytes(self, writeAddress, numBytes):
		readAddress = writeAddress + 1

		self.setup()
				print "Grabbing EEPROM @ 0x%.2X (0x%.4X / %d bytes, 0x%.4X / %d bits)" % (readAddress, numBytes, numBytes, numBytes * 8, numBytes * 8)	
		startTime = time.time()
		bytes = self.readBytes(readAddress, numBytes)
		endTime = time.time()
		deltaTime = endTime - startTime
		print "EEPROM grabbed, %lf seconds, %lf bytes / second" % (deltaTime, (numBytes / deltaTime))
		hexdump(bytes)

		print "Reset Bus Pirate to user terminal: ",		if not self.i2c.resetBP():			raise "failed."
			
		print 'Done'

def getEEPROMUtil():
	return EEPROMUtil()

def toy():
	# 24C04
	# 0xA0(0x50 W) 0xA1(0x50 R) 0xA2(0x51 W) 0xA3(0x51 R)
	
	util = getEEPROMUtil()

	# util.origTest()
	# Lower half?
	# util.dumpEEPROM24(0xA0, 4)
	# Upper half?
	# util.dumpEEPROM24(0xA2, 4)
	
	util.dumpEEPROMBytes(0xA0, 32)
	#print
	util.dumpEEPROMBytes(0xA2, 32)

def IBMT2X():
	# 24C01
	# 0x40(0x20 W) 0x41(0x20 R) 0xA0(0x50 W) 0xA1(0x50 R)
	
	util = getEEPROMUtil()
	if True:
		util.dumpEEPROM24(0x40, 1)
		print
		# Not sure what this is
		#util.dumpEEPROM24(0x50, 1)
	else:
		util.dumpEEPROMBytes(0x40, 32)
		util.dumpEEPROMBytes(0x40, 32)
		util.dumpEEPROMBytes(0x50, 32)
		util.dumpEEPROMBytes(0x50, 32)

def main():
	# toy()
	IBMT2X()
if __name__ == '__main__':
	main()
