# -*- coding: utf-8 -*-
"""
Created on Fri Jul 26 23:43:39 2024

@author: JPGLC
"""

import socket
import datetime
import pandas as pd
import matplotlib.pyplot as plt

UDP_IP = ""  # Escucha en todas las interfaces
UDP_PORT = 12345

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

time_values = []
ax_values = []
ay_values = []
az_values = []
gx_values = []
gy_values = []
gz_values = []
temp_values = []
print("Codigo de Recepcion -> Iniciado")
while True:
    
    ##print("Intentando Recibir")
    
    data = sock.recv(1024)
    if data:
        #print("Recibido de: ",addr)
        values = data.decode().split(',')

        time_values.append(datetime.datetime.strptime(values[0], "%Y-%m-%d %H:%M:%S"))
        ax_values.append(float(values[1]))
        ay_values.append(float(values[2]))
        az_values.append(float(values[3]))
        gx_values.append(float(values[4]))
        gy_values.append(float(values[5]))
        gz_values.append(float(values[6]))
        temp_values.append(float(values[7]))


        print(time_values[-1], ax_values[-1], ay_values[-1], az_values[-1], 
            gx_values[-1], gy_values[-1], gz_values[-1], temp_values[-1])
        
    else:
        print("Recepcion Fallida")
        break
    

    
    