'''
UVNet Universal Decompiler (uvudec)
Unit test daemon since I haven't found a CI server I'm happy with yet and I have simple requirements
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
'''

import json
import smtplib
import string
import shutil
import subprocess
import os
from os import path
import sys
import time
import filecmp

debug_email = False 
ever_send_email = True 
force_build = False

def simple_shell_exec(cmd):
	print 'cmd in: %s' % cmd
	if True:
		return os.system(cmd)
	else:
		cmd = "/bin/bash " + cmd 
		output = ''
		to_exec = cmd.split(' ')
		print 'going to execute: %s' % to_exec
		subp = subprocess.Popen(to_exec)
		while subp.returncode is None:
			# Hmm how to treat stdout  vs stderror?
			com = subp.communicate()[0]
			if com:
				print com
			com = subp.communicate()[1]
			if com:
				print com
			time.sleep(0.05)
			subp.poll()
	
		return subp.returncode

def shell_exec(cmd):
	# ugly...but simple
	# ((false; true; true) 2>&1; echo "***RC_HACK: $?") |tee temp.txt
	rc = simple_shell_exec('(' + cmd + ') 2>&1 |tee file.tmp; exit $PIPESTATUS')
	output = open('file.tmp').read()
	# print 'OUTPUT: %d, %s' % (rc, output)
	return (rc, output)
	
	'''
	print 'cmd in: %s' % cmd
	#rc = os.system(cmd)
	#output = ''
	#cmd = "/bin/bash " + cmd 
	output = ''
	#subp = subprocess.Popen(cmd.split(' '))
	#subp = subprocess.Popen(cmd, stdin=stdin)
	# Hmm okay why don't I get the output to stdout/stderr
	subp = subprocess.Popen(cmd, shell=True)
	while subp.returncode is None:
		# Hmm how to treat stdout  vs stderror?
		com = subp.communicate()[0]
		if com:
			output += com
		com = subp.communicate()[1]
		if com:
			output += com		 
		time.sleep(0.05)
		subp.poll()
	
	return (subp.returncode, output)
	'''

class Builder:
	def __init__(self):
		# A VCS URL and either a unprotected SMTP server or gmail user/pass must be specified
		self.VCS = 'git'
		self.VCS_URL = None
		# gmail SMTP default
		self.email_protocol = 'SMTP'
		self.email_server = "smtp.gmail.com"
		self.email_port = 587
		self.email_username = ''
		self.email_password = ''
		self.email_from = ''
		self.email_to = ''
		self.email_subject_prefix = ''
		self.email_subject = "results"
		self.make_dir = None
		self.make_build_args = ()
		self.make_test_args = ()
		self.file_previous = 'build_0.txt'
		self.file_current = 'build_1.txt'
		# Text representation of the build output as would display in a terminal
		self.build_result_text = None
		self.project_dir = None
		self.new_version = True
		self.should_send_email = False
		self.revision = None
	
	def send_email(self, message):
		global debug_email
		
		fromaddr = self.email_from
		toaddr  = self.email_to
		subject = self.email_subject_prefix + self.email_subject
		# Later we might add some standard footer or something
		body = ''
		body += 'Revision: %s\n' % self.revision
		body += message

		# Credentials (if needed)
		username = self.email_username
		password = self.email_password

		msg = string.join((
			"From: %s" % fromaddr,
			"To: %s" % toaddr,
			"Subject: %s" % subject,
			"",
			body), "\r\n")

		print "Message length is " + repr(len(msg))

		# The actual mail send
		server = smtplib.SMTP('%s:%d' % (self.email_server, self.email_port))
		if debug_email:
			server.set_debuglevel(1)
		server.starttls()
		server.login(username,password)
		server.sendmail(fromaddr, [toaddr], msg)
		server.quit()

	def email_results(self):
		if ever_send_email and self.should_send_email:
			self.send_email(self.build_result_text)
		else:
			print 'NOT sending email'

	def load_config(self, file_name):
		file_content = open(file_name).read()
		j = json.loads(file_content)
		
		# Cause you probably would have used CDash if you had SVN/CVS
		if 'VCS' in j:
			self.VCS = j['VCS']
		
		# Required
		self.VCS_URL = j['VCS_URL']

		if 'email_protocol' in j:
			self.email_protocol = j['email_protocol']
			
		if 'email_server' in j:
			self.email_server = j['email_server']
		
		if 'email_port' in j:
			self.email_port = j['email_port']
		
		if 'email_username' in j:
			self.email_username = j['email_username']
		
		if 'email_password' in j:
			self.email_password = j['email_password']
		
		if 'email_from' in j:
			self.email_from = j['email_from']
		
		if 'email_to' in j:
			self.email_to = j['email_to']
		
		if 'email_subject_prefix' in j:
			self.email_subject_prefix = j['email_subject_prefix']

		if 'file_current' in j:
			self.file_current = j['file_current']
		
		if 'file_previous' in j:
			self.file_previous = j['file_previous']
			
		if 'make_build_args' in j:
			self.make_build_args = j['make_build_args']
			
		if 'make_test_args' in j:
			self.make_test_args = j['make_test_args']

		self.project_dir = j['project_dir']
			
		if 'make_dir' in j:
			self.make_dir = j['make_dir']
		else:
			self.make_dir = self.project_dir

	def checkout(self, revision = None):
		if self.VCS == 'git':
			if os.path.exists(self.project_dir):
				(rc, output) = shell_exec("cd %s && git pull" % self.project_dir)
				if rc:
					print 'ERROR: failed update'
					sys.exit(1)
				self.new_version = output.find('Already up-to-date.') < 0
			else:
				if simple_shell_exec("git clone %s %s" % (self.VCS_URL, self.project_dir)):
					print 'ERROR: failed clone'
					sys.exit(1)
			(rc_temp, revision) = shell_exec("cd %s && git rev-parse HEAD" % self.project_dir)
			self.revision = revision.strip()
		else:
			raise Exception('unknown VCS: %s' % self.VCS)
		
		# This was getting mucked up into build output
		print 'Revision %s' % revision
		sys.stdout.flush()

	def run_make(self):

		# We need to make it only e-mail if the result changed
		if os.path.exists(self.file_current):
			# Rotate log files
			shutil.move(self.file_current, self.file_previous)
		else:
			print 'Doing initial build'
		
		'''
		if simple_shell_exec("(cd %s && make %s)2>&1 |tee %s" % (self.make_dir, string.join(self.make_args, ' '), self.file_current)):
			print 'WARNING: failed build'
		self.build_result_text = open(self.file_current).read()
		'''
		
		output_file = open(self.file_current, 'w')
		(rc, self.build_result_text) = shell_exec("cd %s && make %s" % (self.make_dir, string.join(self.make_build_args, ' ')))
		output_file.write(self.build_result_text)
		if rc:
			print 'ERROR: failed build'
			self.email_subject = "build failure"
			self.should_send_email = True
			output_file.close()
			return

		(rc, self.build_result_text) = shell_exec("cd %s && make %s" % (self.make_dir, string.join(self.make_test_args, ' ')))
		print 'main test rc: %d' % rc
		output_file.write(self.build_result_text)
		status_line = None
		ok_line = None
		severe_error = False
		for line in self.build_result_text.split('\n'):
			# Run: 11   Failure total: 3   Failures: 3   Errors: 0
			if line.find('Run: ') >= 0:
				status_line = line
				break
			# OK (11)
			elif line.find('OK (') >= 0:
				ok_line = line
				break
			elif line.find('SEVERE ERROR') >= 0:
				severe_error = True
				break
		if status_line:
			# Run: 11   Failure total: 3   Failures: 3   Errors: 0
			total_tests = int(status_line.split()[1])
			failed_tests = int(status_line.split()[4])
		elif ok_line:
			# OK (11)
			total_tests = int(status_line.split("()")[1])
			failed_tests = 0
		if rc or failed_tests > 0:
			print 'ERROR: failed test run'
			if status_line:
				self.email_subject = "failed %d / %d tests" % (failed_tests, total_tests)
			elif severe_error:
				self.email_subject = 'severe error (crash)'
			else:
				self.email_subject = 'unknown failure'
			print 'subject: %s' % self.email_subject
			self.should_send_email = True
			output_file.close()
			return
		
		output_file.close()

		# This is so a failure count can be seen that the bugs were fixed
		if os.path.exists(self.file_previous):
			self.should_send_email = filecmp.cmp(self.file_current, self.file_previous)
		else:
			self.should_send_email = True

	def run(self):
		b.load_config(config_file)
		b.checkout()
		if self.new_version or force_build:
			b.run_make()
			b.email_results()
		elif not force_build:
			print 'Skipping build: not a new version'
			

def help():
	print "uvudec build daemon"
	print "Copyright (C) 2010 John McMaster"
	print "Usage:"
	print "%s [args]" % sys.argv[0]
	print "--help: this message"
	print "--debug-email: print verbose information when sending email"
	print "--send-email: send the build results to config'd email address"
	print "--force-build: build even if we are already at latest revision"

if __name__ == "__main__":
	for arg_index in range (1, len(sys.argv)):
		arg = sys.argv[arg_index]
		arg_key = None
		arg_value = None
		if arg.find("--") == 0:
			arg_value_bool = True
			if arg.find("=") > 0:
				arg_key = arg.split("=")[0][2:]
				arg_value = arg.split("=")[1]
				if arg_value == "false" or arg_value == "0" or arg_value == "no":
					arg_value_bool = False
			else:
				arg_key = arg[2:]
				
		if arg_key == "help":
			help()
			sys.exit(0)
		elif arg_key == "debug-email":
			debug_email = arg_value_bool
		elif arg_key == "send-email":
			ever_send_email = arg_value_bool
		elif arg_key == "force-build":
			force_build = arg_value_bool
		else:
			print 'Unrecognized argument: %s' % arg
			help()
			sys.exit(1)

	config_file = "config.json"
	b = Builder()
	b.run()

