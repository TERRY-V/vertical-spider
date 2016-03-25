##################################################################################
#
# @All Right Reserved (C), 2015
# Filename:	__client.py
# Version:	ver1.0
# Author:	TERRY-V
# Support:	http://blog.sina.com.cn/terrynotes
# Date:		2014/06/11
#
##################################################################################

#!/usr/bin/env python

from socket import *
import struct

HOST = '192.168.0.101'
PORT = 2222
BUFSIZ = 10240

ADDR = (HOST, PORT)

def main():
	while True:
		tcpCliSock=socket(AF_INET, SOCK_STREAM)
		tcpCliSock.connect(ADDR)
		xml=open('entry.xml').read()
		print xml
		data=struct.pack('<8si2h14s2hi%dsi' % len(xml), 'YST1.0.0', len(xml)+30, 1, 0, '0'*14, 0, 0, len(xml), xml, 0)
		tcpCliSock.send(data)
		nHeadBytes=tcpCliSock.recv(32)
		head, length, save, cmd, status=struct.unpack('<8si14shi', nHeadBytes)
		if not status:
			print 'Success'
		else:
			print 'Faied, status = %d' % status
		tcpCliSock.close()
		break
	return

if __name__ == '__main__':
	main()
