#!/usr/bin/env python

import random
import argparse
import socket
import threading
import socket
import time 
from scapy.all import sniff
from scapy.all import Ether, IP, IPv6, TCP
NUM_PACKETS = 1
parser = argparse.ArgumentParser(description='run_test.py')
parser.add_argument('--first',
		help='If set pkts are send through veth0, else veth3',
		action="store_true",
		default=False)
args = parser.parse_args()

class PacketQueue:
	def __init__(self):
		self.pkts = []
		self.lock = threading.Lock()
		self.ifaces = set()

	def add_iface(self, iface):
		self.ifaces.add(iface)

	def get(self):
		self.lock.acquire()
		if not self.pkts:
			self.lock.release()
			return None, None
		pkt = self.pkts.pop(0)
		self.lock.release()
		return pkt

	def add(self, iface, pkt):
		if iface not in self.ifaces:
			return
		self.lock.acquire()
		self.pkts.append( (iface, pkt) )
		self.lock.release()

queue = PacketQueue()

def pkt_handler(pkt, iface):
	if IPv6 in pkt:
		return
	queue.add(iface, pkt)

class SnifferThread(threading.Thread):
	def __init__(self, iface, handler = pkt_handler):
		threading.Thread.__init__(self)
		self.iface = iface
		self.handler = handler

	def run(self):
		sniff(
				iface = self.iface,
				prn = lambda x: self.handler(x, self.iface)
				)

class PacketDelay:
	def __init__(self, bsize, bdelay, imin, imax, num_pkts = 100):
		self.bsize = bsize
		self.bdelay = bdelay
		self.imin = imin
		self.imax = imax
		self.num_pkts = num_pkts
		self.current = 1

	def __iter__(self):
		return self

	def next(self):
		if self.num_pkts <= 0:
			raise StopIteration
		self.num_pkts -= 1
		if self.current == self.bsize:
			self.current = 1
			return random.randint(self.imin, self.imax)
		else:
			self.current += 1
			return self.bdelay

#pkt = Ether()/IP(dst='10.0.0.1', ttl=64)/TCP()


port_map = {
		1: "veth0",
		2: "veth3",
		}

#print "port_map[1]",port_map[1],"port_map[2]",port_map[2] 

iface_map = {}
for p, i in port_map.items():
	iface_map[i] = p

queue.add_iface("veth0")
queue.add_iface("veth3")

for p, iface in port_map.items():
	t = SnifferThread(iface)
	t.daemon = True
	t.start()

send_socket = socket.socket(socket.AF_PACKET, socket.SOCK_RAW,
		socket.htons(0x03))

if args.first:
	pkt = Ether(dst='a0:36:9f:3e:94:ea',src='a0:36:9f:3e:94:e8')/IP(dst='192.168.1.1',src='192.168.0.1')
	port2send = port_map[1]
else:
	pkt = Ether(dst='a0:36:9f:3e:94:e8',src='a0:36:9f:3e:94:ea')/IP(dst='192.168.0.1',src='192.168.1.1')
	port2send = port_map[2]

send_socket.bind((port2send, 0))
delays = PacketDelay(10, 5, 25, 100, NUM_PACKETS)
ports = []
print "Sending", NUM_PACKETS, "packets to ", port2send ,"..."
for d in delays:
	# sendp is too slow...
	# sendp(pkt, iface=port_map[3], verbose=0)
#    if args.random_dport:
#        pkt["TCP"].dport = random.randint(1025, 65535)
	time.sleep(d / 1000.)
	send_socket.send(str(pkt))

time.sleep(2)
iface, pkt = queue.get()
while pkt:
	ports.append(iface_map[iface])
	iface, pkt = queue.get()
'''
print "Pkt sent on ports.."
print ports
'''
print "DISTRIBUTION: "
#print "pkts sent from",port2send,"to", (int(port2send%2)+1)
for p in port_map:
	c = ports.count(p)
	print "port {}: {:>3} [ {:>5}% ]".format(port_map[p], c, 100. * c / NUM_PACKETS)
