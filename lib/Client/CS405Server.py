#!/usr/bin/env python
import socket
import os
import sys
import re

PORT = 42069
HOST = ''

class checkersMessage:
	
	statusByte=0
	board=[]
	def __init__(self):
		

		for i in range(8):
			temp=[]
			for j in range(8):
				temp.append(0)
			self.board.append(temp)

		#initialize the red side of the board
		for i in range(3):
			for j in range(8):
				
				if i%2==0:
					#red piece on black square
					if j%2==0:
						self.board[i][j]=1
					else:
						self.board[i][j]=8
				else:
					if j%2==1:
						self.board[i][j]=1
					else:
						self.board[i][j]=8
		for i in range(3,5):
			for j in range(8):
				
				if i%2==0:
					#empty squares

					if j%2==0:
						self.board[i][j]=8
					else:
						self.board[i][j]=0
				else:
					#empty squares

					if j%2==1:
						self.board[i][j]=8
					else:
						self.board[i][j]=0

		for i in range(5,8):
			for j in range(8):
				
				if i%2==0:
					#empty squares

					if j%2==1:
						self.board[i][j]=8
					else:
						self.board[i][j]=3
				else:
					if j%2==0:
						self.board[i][j]=8
					else:
						self.board[i][j]=3
	

	def toString(self):
		temp=""
		temp+=str(self.statusByte)
		for i in range(8):
			for j in range(8):
				temp+=str(self.board[i][j])
		return temp


	def fromString(self,brd):
		print(len(brd))
		self.statusByte=int(brd[0])
		
		for i in range(8):
			for j in range(8):
				self.board[i][j]=int(brd[i*8+j])

	def printBoard(self):

		for i in range(8):
			for j in range(8):

				if self.board[i][j]&8==8:
					sys.stdout.write( " ")
				elif self.board[i][j]&1==0:
					sys.stdout.write( "_")
				elif self.board[i][j]&2==0 and self.board[i][j]&4==0:
					sys.stdout.write( "r")
				elif self.board[i][j]&2==0 and self.board[i][j]&4==4:
					sys.stdout.write( "R")
				elif self.board[i][j]&2==2 and self.board[i][j]&4==4:
					sys.stdout.write( "B")
				else:
					sys.stdout.write( "b")
		print("\n")

	def getStatus(self):
		return self.statusByte

if __name__=='__main__':
	

	msg=checkersMessage()
	msg.printBoard()

	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.bind((HOST, PORT))
	s.listen(1)

	conn1, addr1 = s.accept()

	startFlag=0

	print 'Connected by', addr1
	data=conn1.recv(1024)
	if data=="Ready to Play":
		startFlag+=1

	conn2, addr2 = s.accept()
	print 'Connected by', addr2
	data=conn2.recv(1024)
	
	if data=="Ready to Play":
		startFlag+=1	

	while True:
		conn1.sendall(msg.toString())
		data=conn1.recv(1024)
		msg.fromString(data)
		
		sts=msg.getStatus()
		if sts==1:
			conn2.sendall("Game over")
			print("Game over man")
			s.close()
			break
		elif sts!=0:
			conn2.sendall("Stopping game due to illegal move")
			print("Game over man")
			s.close()
			break
		conn2.sendall(msg.toString())
		data=conn2.recv(1024)
		msg.fromString(msg.toString())
		
		if sts==1:
			conn1.sendall("Game over")
			print("Game over man")
			s.close()
			break
		elif sts!=0:
			conn1.sendall("Stopping game due to illegal move")
			print("Game over man")
			s.close()
			break		
