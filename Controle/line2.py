# -*- coding: iso-8859-1 -*-

import sys
import time
import serial
import threading
import sys
import pygame
import math

from pygame.locals import *
SIZE = 800, 800
pygame.init()
screen = pygame.display.set_mode(SIZE)
FPSCLOCK = pygame.time.Clock()
done = False
screen.fill((0, 0, 0))
degree=0

DEVICE='/dev/ttyUSB0'
SPEED=9600

def open_serial(dev, speed, show_info=False):
	ser = serial.Serial(dev, speed, timeout=1)
	#time.sleep(0.5)
	if show_info:
		print ('\nStatus: %s ' % (ser.isOpen()))
		print ('Device: %s ' % (ser.name))
		print ('Settings:\n %s ' % (ser))
	return ser


def read_serial(ser, stop):
	angleZ = 0.00
	angleY = 0.00
	angleX = 0.00

	angleDir = 0.00
	while True:
		rec = ser.readline().rstrip()
		if rec != "":
			print(rec)
			if('Z' in rec):
				angleZ = rec.split('=')[1]
				print(angleZ)
				angleZ = float(angleZ)
				angleZ = math.ceil(angleZ)
				print(angleZ)
			elif("Y" in rec):
				angleY = rec.split('=')[1]
				angleY = float(angleY)
				angleY = math.ceil(angleY)
			elif("X" in rec):
				angleX = rec.split('=')[1]
				angleX = float(angleX)
				angleX = math.ceil(angleX)

			#angle = float(rec)
		if stop():
			break

		global done, degree, screen, FPSCLOCK, SIZE
		screen.fill(0)
		for e in pygame.event.get():
			if e.type == QUIT or (e.type == KEYDOWN and e.key == K_ESCAPE):
				done = True
				break

		if(angleY > 45 and angleY < 135):
			angleDir = angleZ
		#elif((angleY <= 45 or angleY >= 315 or (angleY >= 135 and angleY <= 225)):
		else:
			angleDir = angleX

		radar = (400.00,400.00)
		radar_len = 100.00
		x = radar[0] + math.cos(math.radians(angleZ)) * radar_len
		y = radar[1] + math.sin(math.radians(angleZ)) * radar_len

		x2 = radar[0] + math.cos(math.radians(angleZ + 180.00)) * radar_len
		y2 = radar[1] + math.sin(math.radians(angleZ + 180.00)) * radar_len



		radar2 = (625.00,400.00)
		radar_len2 = 100.00
		xb = radar2[0] + math.cos(math.radians(angleY + 180.00)) * radar_len
		yb = radar2[1] + math.sin(math.radians(angleY + 180.00)) * radar_len

		x2b = radar2[0] + math.cos(math.radians(angleY)) * radar_len
		y2b = radar2[1] + math.sin(math.radians(angleY)) * radar_len

		radar3 = (175.00,400.00)
		radar_len2 = 100.00
		xc = radar3[0] + math.cos(math.radians(angleX + 180.00)) * radar_len
		yc = radar3[1] + math.sin(math.radians(angleX + 180.00)) * radar_len

		x2c = radar3[0] + math.cos(math.radians(angleX)) * radar_len
		y2c = radar3[1] + math.sin(math.radians(angleX)) * radar_len

		radar4 = (400.00,600.00)
		radar_len2 = 100.00
		xd = radar4[0] + math.cos(math.radians(angleDir + 180.00)) * radar_len
		yd = radar4[1] + math.sin(math.radians(angleDir + 180.00)) * radar_len

		x2d = radar4[0] + math.cos(math.radians(angleDir)) * radar_len
		y2d = radar4[1] + math.sin(math.radians(angleDir)) * radar_len

		# then render the line radar->(x,y)
		pygame.draw.line(screen, Color("red"), radar, (x,y), 1)
		pygame.draw.line(screen, Color("red"), radar, (x2,y2), 1)
		pygame.draw.line(screen, Color("yellow"), radar2, (xb,yb), 1)
		pygame.draw.line(screen, Color("yellow"), radar2, (x2b,y2b), 1)
		pygame.draw.line(screen, Color("white"), radar3, (xc,yc), 1)
		pygame.draw.line(screen, Color("white"), radar3, (x2c,y2c), 1)

		pygame.draw.line(screen, Color("green"), radar4, (xd,yd), 1)
		pygame.draw.line(screen, Color("green"), radar4, (x2d,y2d), 1)

		pygame.display.flip()
		degree+=5
		FPSCLOCK.tick(40)



if __name__ == "__main__":
	if len(sys.argv) == 2:
		DEVICE = sys.argv[1]
	elif  len(sys.argv) == 3:
		DEVICE = sys.argv[1]
		SPEED = sys.argv[2]

	ser = open_serial(DEVICE, SPEED)

	stop=False

	threading.Thread(target=read_serial, args =(ser, lambda : stop, )).start()

	try: 
		while True:
			x = raw_input("Say something: ");
			ser.write(x + '\n')
	except:
		stop = True
ser.close()
