#! /usr/bin/env python
###############################################################################
#
# simulavr - A simulator for the Atmel AVR family of microcontrollers.
# Copyright (C) 2001, 2002  Theodore A. Roth
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
###############################################################################
#
# $Id: gdb_rsp.py,v 1.1 2004/07/31 00:59:32 rivetwa Exp $
#

import socket, struct, array, sys

from registers import Reg

class GdbRSP_Exception(Exception):   pass

class ErrCheckSum(GdbRSP_Exception): pass
class ErrPacket(GdbRSP_Exception):   pass
class ErrReply(GdbRSP_Exception):    pass
class ErrMemRead(GdbRSP_Exception):  pass

class GdbRemoteSerialProtocol:
	"""GDB Remote Serial Protocol client implemntation.

	Simulates what GDB does. This class only implements a minimal subset of
	the remote serial protocol as needed to perform regression testing.

	The following packets types are implemented:
	  g   read registers
	  G   write registers
	  p   read single register
	  P   write single register
	  k   kill request
	  m   read memory
	  M   write memory
	  c   continue
	  C   continue with signal
	  s   step
	  S   step with signal
	  z   remove break or watchpoint
	  Z   insert break or watchpoint
	"""
	
	def __init__(self, host='localhost', port=1212, ofile=None):
		"""Open a connection to a remote target.
		"""
		# where to write the output of print statements
		self.ofile = ofile
		
		# connect to remote target
		self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.socket.connect((host,port))
		self.ack()

	def __del__(self):
		try:
			self.close()
		except:
			# close() most likely already called
			pass

	def close(self):
		self.send('k')
		reply = self.recv()
		self.out('Recv: "%s"' % (reply))
		self.socket.close()

	def out(self, s):
		if self.ofile:
			print >> self.ofile, s

	def cksum(self,pkt):
		sum = 0
		for c in pkt:
			sum += ord(c)

		return sum & 0xff

	def ack(self):
		self.out( 'Send: "+" (Ack)' )
		self.socket.send('+')
		
	def send(self, msg):
		s = '$'+msg+'#'+'%02x'%(self.cksum(msg))
		self.out( 'Sent: "%s"' % (s) )
		self.socket.send(s)
		reply = self.socket.recv(1)
		if reply != '+':
			raise ErrReply, reply
		else:
			self.out( '-> Ack' )

	def recv(self):
		c = self.socket.recv(1)
		if c == '$':
			max_buf = 400
			pkt = ''
			while max_buf:
				c = self.socket.recv(1)
				if c == '#':
					break
				pkt += c
				max_buf -= 1
				
			sum = self.cksum(pkt)
			cccc = self.socket.recv(1)
			cccc += self.socket.recv(1)
			csum = int( cccc, 16 )
			if sum != csum:
				raise ErrCheckSum, 'pkt="%s#%s", %02x : %02x' %(pkt,cccc,sum,csum)

			return pkt
		elif c == '+':
			self.out( 'Ack' )
		else:
			raise ErrPacket, c

		return None

	def str2bin(self, s):
		"""Convert a string of ascii hex digit pairs to an array of 8-bit binary values.
		"""
		arr = array.array('B')
		for i in range( 0, len(s), 2 ):
			arr.append( int(s[i:i+2], 16) )

		return arr

	def bin2str(self, arr):
		"""Convert an array of 8-bit binary values to a string of ascii hex digit pairs.
		"""
		ss = ''
		for i in arr:
			ss += '%02x' % (i)

		return ss

	def read_regs(self):
		self.send('g')
		reply = self.recv()
		self.out( 'Recv: "%s"' % (reply) )

		# little endian + 32 reg (B=8-bit) + SREG (B=8-bit) + SP (H=16-bit) + PC (L=32-bit)
		regs = [ i for i in struct.unpack( '<33BHL', self.str2bin(reply) ) ]
		return regs
	
	def write_regs(self, regs):
		arr = array.array('B')
		arr.fromstring(struct.pack('<33BHL', *regs))
		self.send('G'+self.bin2str(arr))
		reply = self.recv()
		if reply != 'OK':
			raise ErrReply
		self.out( 'Recv: "%s"' % (reply) )

	def read_reg(self, reg):
		self.send('p%x'%(reg))
		reply = self.str2bin(self.recv())
		if reg < Reg.SP:
			val = struct.unpack( '<B', reply )
		elif reg < Reg.PC:
			val = struct.unpack( '<H', reply ) # SP is 16 bit
		else:
			val = struct.unpack( '<L', reply ) # PC is 32 bit

		return val[0]

	def write_reg(self, reg, val):
		arr = array.array('B')
		if reg < Reg.SP:
			arr.fromstring(struct.pack('<B', val))
		elif reg < Reg.PC:
			arr.fromstring(struct.pack('<H', val)) # SP is 16 bit
		else:
			arr.fromstring(struct.pack('<L', val)) # PC is 32 bit

		self.send('P%x=%s' % (reg, self.bin2str(arr)))
		reply = self.recv()
		if reply != 'OK':
			raise ErrReply
		self.out( 'Recv: "%s"' % (reply) )

	def read_mem(self, addr, _len):
		self.send('m%x,%x' % (addr,_len))
		reply = self.recv()
		if reply[0] == 'E':
			raise ErrMemRead
		self.out( 'Recv: "%s"' % (reply) )

		return self.str2bin(reply)

	def write_mem(self, addr, _len, buf):
		self.send( 'M%x,%x:' %(addr,_len) + self.bin2str(buf) )
		reply = self.recv()
		if reply != 'OK':
			raise ErrReply
		self.out( 'Recv: "%s"' % (reply) )

	def handle_reply(self):
		"""The C, c, S, s and ? packets all expect the same reply.
		"""
		while 1:
			reply = self.recv()
			if reply[0] != 'O':
				break

		return reply

	def cont(self, addr=None):
		pkt = 'c'
		if addr != None:
			pkt += '%x' % (addr)
		self.send( pkt )
		return self.handle_reply()

	def cont_with_signal(self, signo=0, addr=None):
		pkt = 'C%02x' % (signo)
		if addr != None:
			pkt += ';%x' % (addr)
		self.send( pkt )
		return self.handle_reply()

	def step(self, addr=None):
		pkt = 's'
		if addr != None:
			pkt += '%x' % (addr)
		self.send( pkt )
		return self.handle_reply()

	def step_with_signal(self, signo=0, addr=None):
		pkt = 'S%02x' % (signo)
		if addr != None:
			pkt += ';%x' % (addr)
		self.send( pkt )
		return self.handle_reply()

	def break_insert(self, _type, addr, _len):
		pkt = 'Z%d,%x,%x' % (_type, addr, _len)
		self.send( pkt )
		reply = self.recv()
		if reply == '':
			self.out( 'Z packets are not supported by target.' )
		elif reply != 'OK':
			raise ErrPacket

	def break_remove(self, _type, addr, _len):
		pkt = 'z%d,%x,%x' % (_type, addr, _len)
		self.send( pkt )
		reply = self.recv()
		if reply == '':
			self.out( 'Z packets are not supported by target.' )
		elif reply != 'OK':
			raise ErrPacket

	def interrupt(self):
		"""Send target an interrupt.
		"""
		self.socket.send(chr(3))

if __name__ == '__main__':
	import signal
	rsp = GdbRemoteSerialProtocol()

	# Try to read/write registers
	regs = rsp.read_regs()
	regs[10] = 10
	rsp.write_regs(regs)
	print rsp.read_regs()

	print rsp.read_reg(12)
	rsp.write_reg(12,0xaa)
	print rsp.read_reg(12)
	print rsp.read_reg(34)				# PC

	# Try to read/write memory
	addr = 0x0
	_len  = 10
	mem = rsp.read_mem(addr,_len)
	print mem
	mem[4] = 10
	rsp.write_mem(addr,len(mem),mem)
	print rsp.read_mem(addr,_len)

	# Write zero's (NOP's) to first 60 bytes of flash
	arr = array.array('B', [ 0x0 for i in range(60) ])
	rsp.write_mem(addr, 60, arr)

	# Set a breakpoint at 8th instruction
	rsp.break_insert(0,0x10,0)
	rsp.cont()
	rsp.break_remove(0,0x10,0)
	rsp.step()
	rsp.step()
	rsp.step()
	rsp.step()

	rsp.break_insert(0,0x10,0)
	rsp.cont_with_signal(signal.SIGHUP)
	rsp.cont()
	
	rsp.close()
