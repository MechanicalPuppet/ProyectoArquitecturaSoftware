import csv
import random
import datetime
import socket
import time
from datetime import date


# Configurar la conexión serie UDP
UDP_PORT = 8889
today = date.today()
nombre_archivo = str(today)+ ".csv"
 

#Intervalo de tiempo en segundos para guardar los datos
interval = 10
start_time = time.time()

# Generación de datos aleatorios
datos = []
 
try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('', UDP_PORT))
    
    while True:
        data, addr = sock.recvfrom(1024)
        print("received message:")
        print(data.decode('utf-8')) 
        datos.append(data)
        current_time = time.time()
        if current_time - start_time >= interval:
            # Escritura en el archivo CSV
            with open(nombre_archivo, "w", newline="") as archivo_csv:
                escritor_csv = csv.writer(archivo_csv)
                escritor_csv.writerow(["Tiempo", "Giroscopio (X, Y, Z)", "Acelerómetro (X, Y, Z)"])  # Encabezado
                escritor_csv.writerows(datos)
                start_time = current_time
except KeyboardInterrupt:
    print("Se ha interrumpido el guardado de datos.")

finally:
    print("Datos guardados correctamente.")
    
print(f"Se ha creado el archivo '{nombre_archivo}' con las muestras de datos.")