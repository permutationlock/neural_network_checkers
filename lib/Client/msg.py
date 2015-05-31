#!/usr/bin/python
import sys
import re


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
		self.statusByte=int(brd[0])
		for i in range(8):
			for j in range(1,9):
				self.board[i][j-1]=int(brd[i*8+j])

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


