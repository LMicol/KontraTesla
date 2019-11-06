# IOT-controle

Repositório para os códigos do trabalho final da disciplina de IOT - UFSM

* [controle.py](controle.py): Lê a serial e, caso tenha recebido valores dos dispositivos, faz o processamento utilizando o cálculo de [teste.py](controle-teste.py) para publicar os valores via MQTT.
* [controle-teste.py](controle-teste.py): Recebe duas entradas (x e y), entre -128 e 128, para processar e mostrar a saída que será passada aos motores.

Arduino:

```
//Written by Ahmet Burkay KIRNIK
//TR_CapaFenLisesi
//Measure Angle with a MPU-6050(GY-521)

#include<Wire.h>

const int MPU_addr=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

int minVal=265;
int maxVal=402;

double x;
double y;
double z;
 


void setup(){
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);
}
void loop(){
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);
  AcX=Wire.read()<<8|Wire.read();
  AcY=Wire.read()<<8|Wire.read();
  AcZ=Wire.read()<<8|Wire.read();
    int xAng = map(AcX,minVal,maxVal,-90,90);
    int yAng = map(AcY,minVal,maxVal,-90,90);
    int zAng = map(AcZ,minVal,maxVal,-90,90);

       x= RAD_TO_DEG * (atan2(-yAng, -zAng)+PI);
       y= RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);
       z= RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);

     Serial.print("AngleX= ");
     Serial.println(x);

     Serial.print("AngleY= ");
     Serial.println(y);

     Serial.print("AngleZ= ");
     Serial.println(z);
     Serial.println("-----------------------------------------");
     delay(400);
}
```


DRAW LINE:

```
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
SPEED=115200

def open_serial(dev, speed, show_info=False):
	ser = serial.Serial(dev, speed, timeout=1)
	#time.sleep(0.5)
	if show_info:
		print ('\nStatus: %s ' % (ser.isOpen()))
		print ('Device: %s ' % (ser.name))
		print ('Settings:\n %s ' % (ser))
	return ser


def read_serial(ser, stop):
	while True:
		rec = ser.readline().rstrip()
		if rec != "":
			print(rec)
 		if stop():
			break

		global done, degree, screen, FPSCLOCK, SIZE
		screen.fill(0)
		for e in pygame.event.get():
			if e.type == QUIT or (e.type == KEYDOWN and e.key == K_ESCAPE):
				done = True
				break
		#angle = input('Enter angle: ')
		#angle = float(angle)
		
		angle = 0.00
		radar = (400.00,400.00)
		radar_len = 100.00
		x = radar[0] + math.cos(math.radians(angle)) * radar_len
		y = radar[1] + math.sin(math.radians(angle)) * radar_len

		x2 = radar[0] + math.cos(math.radians(angle + 180.00)) * radar_len
		y2 = radar[1] + math.sin(math.radians(angle + 180.00)) * radar_len

		# then render the line radar->(x,y)
		pygame.draw.line(screen, Color("red"), radar, (x,y), 1)
		pygame.draw.line(screen, Color("red"), radar, (x2,y2), 1)
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

```
