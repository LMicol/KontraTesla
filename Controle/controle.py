"""

Le os valores da serial (joystick/volante), manipula e publica via MQTT

"""

import serial, sys, time
import paho.mqtt.client as mqtt

output_history = ''

# mapeia o valor 'value' do intervalo 'in' para o intervalo 'out'
def map(value, in_min, in_max, out_min, out_max):
    return round(((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min))

#
DEVICE='/dev/ttyUSB0'
SPEED=115200
BROKER='10.1.1.110'

#
def open_serial(dev, speed, show_info=False):
    ser = serial.Serial(dev, speed, timeout=1)
    time.sleep(0.5)
    if show_info:
        print ('\nStatus: %s ' % (ser.isOpen()))
        print ('Device: %s ' % (ser.name))
        print ('Settings:\n %s ' % (ser))
    return ser

def arruma_re(val):
    if (val<0):
        return map(val,-1,-127,128,255)
    else:
        return val
#
if __name__ == "__main__":
    if len(sys.argv) == 2:
        DEVICE = sys.argv[1]
    elif  len(sys.argv) == 3:
        DEVICE = sys.argv[1]
        SPEED = sys.argv[2]

    ser = open_serial(DEVICE, SPEED)

    stop=False

    #
    pub = mqtt.Client('controle')
    pub.connect(BROKER, port=1883)

    #try:
    counts=0
    while True:
        rec = ser.readline().rstrip()

        try:
            rec = str(rec, 'utf-8')
            # print(rec)
        except:
            print('Trying again')
            rec = ''

        if rec != '': # rec -> 'x,y'
            x = int(rec.split(',')[0])
            y = int(rec.split(',')[1])
            # calcula os valores para os motores
            # virar para direita (reto = 0)
            if (x >= 0):
                left = y
                right = y - map(x, 0, 127, 0, y)
            # virar para esquerda
            else:
                left = y + map(x, 0, 128, 0, y)	
                right = y

            left=arruma_re(left)
            right=arruma_re(right)

            output = str(left) + ',' + str(right)

            # envia para o broker mqtt
            if output != output_history:
                pub.publish('/c0/eng', "m,"+output)
                print(output)
                output_history = output
                counts = 0
            if left == 0 and right == 0 and counts < 10:
                pub.publish('/c0/eng', "s")
                print('s')
                counts+=1
                

        if stop:
            break

    #except:
    print("Except:")
    #print(e)
    stop = True
    ser.close()
    #pub.disconnect()
