#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <time.h>
#include <NTPClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const char* ssid = ""; // Tu nombre de red (SSID)
const char* password = ""; // Tu contraseña de Wi-Fi

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
const long utcOffsetInSeconds = -21600; // CST offset

WiFiUDP Udp;
IPAddress remoteIP(, , , ); // IP de la PC
unsigned int localPort = 2390;        
unsigned int remotePort = 12345;    

Adafruit_MPU6050 mpu;

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Conectado  a WiFi");
    Udp.begin(localPort);

    if (!mpu.begin()) {
        Serial.println("No se pudo encontrar el MPU6050");
        while (1);
    }
    Serial.println("MPU6050 encontrado!");

    timeClient.begin();
    timeClient.setTimeOffset(utcOffsetInSeconds); 
}

void loop() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    timeClient.update(); // Actualizar la hora ANTES de usarla

    time_t rawtime = timeClient.getEpochTime();
    struct tm * ti;
    ti = localtime (&rawtime);
    
    // Enviar datos por UDP
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", ti);

    String data = String(timeStr) + "," + 
                  String(a.acceleration.x, 3) + "," + String(a.acceleration.y, 3) + "," + String(a.acceleration.z, 3) + "," +
                  String(g.gyro.x, 3) + "," + String(g.gyro.y, 3) + "," + String(g.gyro.z, 3) + "," +
                  String(temp.temperature); // Incluye temperatura

    Udp.beginPacket(remoteIP, remotePort);
    Udp.print(data);
    Udp.endPacket();

    delay(100); 
}