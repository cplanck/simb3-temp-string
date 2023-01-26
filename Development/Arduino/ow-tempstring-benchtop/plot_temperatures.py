import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style
import numpy as np
import random
import serial
import matplotlib.pyplot as plt
from matplotlib.widgets import Button



#initialize serial port
ser = serial.Serial()
ser.port = '/dev/cu.usbmodem113401' #Arduino serial port
ser.baudrate = 9600
ser.timeout = 10 #specify timeout when using readline()
ser.open()
if ser.is_open==True:
	print("\nAll right, serial port now open. Configuration:\n")
	print(ser, "\n") #print serial parameters


def callSerial():
    line=ser.readline()
    return line.decode('utf-8').strip()

flag = 1
temp_readings = []
start_read = False
while flag:

    serial_output = callSerial()

    if serial_output == 'END READ':
        plt.close('all')
        start_read = False
        temp_readings.pop(0)
        sensors = list(range(0,80))
        stringless_temp_readings = [float(i) for i in temp_readings]
        plt.plot(stringless_temp_readings)
        plt.ylabel('Temperatures')
        plt.ylim([-40,40])
        plot = plt.show()
        temp_readings = []

    if serial_output == 'START READ':
        start_read = True
        pass
    
    if start_read:
        start_index = len(serial_output) - 5
        temp_readings.append(serial_output[start_index:28])
   
