#!/usr/bin/python
'''
uvstructoffset: structure offset computer
Parses C header files for structure offsets
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Dependencies:
	pycparser
		http://code.google.com/p/pycparser/
This is intended to be a test app to later be able to load C structs for analysis although the main app is C++, so not sure
'''

import sys
from pycparser import c_parser, c_ast, parse_file
from pycparser.portability import printme
import tempfile
import shutil
import subprocess
import os

g_printCProg = False
g_debug = False

def printDebug(s):
	if g_debug:
		print s

class StructDefPrinterVisitor(c_ast.NodeVisitor):	
	def __init__(self, structPrinter):
		self.structPrinter = structPrinter
	
	def visit_Struct(self, node):
		self.structPrinter.CProg += '\tprintf("struct %s\\n");\n' % node.name
		for member in node.children():
			self.structPrinter.CProg += '\tprintf("\\t%s @ 0x%%.4X\\n", offsetof(struct sig_header_t, %s));\n' % (str(member.name).ljust(40), member.name)

class StructPrinter:
	def __init__(self):
		self.CProg = ""
		self.indent = ""
			
	def getIndent():
		return self.indent

	def incIndent():
		self.indent += '\t'

	def decIndent():
		self.indent = self.indent[0:-1]

	def printStructOffsets(self, fileName):
		#print 'opening on %s' % fileName
		ast = parse_file(fileName, use_cpp=True, cpp_args=["-D__extension__=", "-D__attribute__(...)="])
	
		self.CProg = """
			#include <stdio.h>
			#include <stddef.h>
			#include "%s"
			
			int main()
			{
			""" % (fileName)
		v = StructDefPrinterVisitor(self)
		v.visit(ast)
		self.CProg += """
				return 0;
			}
			"""
		
		if g_printCProg:
			print self.CProg
		
		tempDir = tempfile.mkdtemp()
		try:
			CProgFile = tempDir + "/main.c"
			progFile = tempDir + "/prog"
			open(CProgFile, "w").write(self.CProg)
			commandLine = ["gcc", "-I%s" % os.getcwd(), CProgFile, "-o", progFile]
			printDebug("Command line: %s" % commandLine)
			retcode = subprocess.call(commandLine)
			printDebug('recode: %d' % retcode)
			if retcode != 0:
				raise 'bad compile'
			
			retcode = subprocess.call(progFile)
			if retcode != 0:
				raise 'bad run'
			
		finally:
			shutil.rmtree(tempDir)
			pass

def usage():
	print 'Usage: %s [args] <file name>' % sys.argv[0]
	print 'args:'
	print '--cprog: print the intermediate C program'
	print '--debug: print debug statements'

if __name__ == "__main__":
	fileName = None
	for i in range(1, len(sys.argv)):
		arg = sys.argv[i]
		if arg.find("--") < 0:
			fileName = arg
		elif arg == "--cprog":
			g_printCProg = True
		elif arg == "--debug":
			g_debug = True
		else:
			print 'Unrecognized arg: %s' % arg
			usage()
			sys.exit(1)	
	
	if fileName is None:
		usage()
		sys.exit(1)	

	printer = StructPrinter()
	printer.printStructOffsets(fileName)

