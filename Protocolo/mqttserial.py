# -*- coding: iso-8859-1 -*-
import sys
import time
import serial
import threading
import paho.mqtt.client as mqtt

DEVICE='/dev/ttyACM0'
# DEVICE= 'COM3'
SPEED=115200
TOPICS=[("/c0/eng", 2),("/c0/servo",2)]
HOSTNAME= "10.1.1.110"
# HOSTNAME="192.168.1.109"
# HOSTNAME= "localhost"

def open_serial(dev, speed, show_info=False):
	ser = serial.Serial(dev, speed, timeout=1)
	time.sleep(0.5)
	if show_info:
		print ('\nStatus: %s ' % (ser.isOpen()))
		print ('Device: %s ' % (ser.name))
		print ('Settings:\n %s ' % (ser))
	return ser

def read_serial(ser, stop):
	while True:
		topic = ""
		rec = ser.readline()
		if rec != b'':
			msg=rec.decode('utf-8')
			print(msg)
			if msg[0] == "t":
				pay = msg[2:]
				topic = "/c0/temp"
			if msg[0] == "i":
				pay = msg[2:]
				topic = "/c0/ir"
			if msg[0] == "u":
				pay = msg[2:]
				topic = "/c0/ultra"
			if msg[0] == "a":
				pay = msg[2:]
				topic = "/c0/acel"
			if (topic != ""):
				client.publish(topic, pay, 2, 0)
			#print (rec.decode('utf-8'))
		if stop():
			break

def on_connect(client, userdata, flags, rc):
    # O subscribe fica no on_connect pois, caso perca a conexão ele a renova
    # Lembrando que quando usado o #, você está falando que tudo que chegar após a barra do topico, será recebido
	print("Conectou no Broker")
	client.subscribe(TOPICS)
	# client.subscribe("#",0)

def on_subscribe(client, userdata, mid, granted_qos):
	print("Inscrito em: ", TOPICS)

def on_message(client, userdata, msg):
	print("Mensagem recebida: ")
	print(msg.topic+" -  "+str(msg.payload))
	# if(msg.topic == "/c0/eng"):
		# id="m,"
		# if(msg.payload.decode()=="s"):
			# id=""
	# if(msg.topic == "/c0/servo"):
		# id="v,"
	# print(id + msg.payload.decode())
	# snd=id + msg.payload.decode() + "\n"
	snd=msg.payload.decode() + "\n"
	print(snd)
	ser.write(snd.encode())

if __name__ == "__main__":
	try:
		ser = open_serial(DEVICE, SPEED, True)
		print("Porta serial conectada.")
	except:
		print("Erro ao conectar na porta serial.")
		sys.exit()
	if len(sys.argv) == 2:
		DEVICE = sys.argv[1]
	elif  len(sys.argv) == 3:
		DEVICE = sys.argv[1]
		SPEED = sys.argv[2]

	stop=False
	threading.Thread(target=read_serial, args =(ser, lambda : stop, )).start()

	client = mqtt.Client()
	client.on_connect = on_connect
	client.on_subscribe = on_subscribe
	client.on_message = on_message
	# Conecta no MQTT Broker, no meu caso, o Mosquitto
	client.connect(HOSTNAME,1883,6000)
	# Inicia o loop
	client.loop_forever()
