#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <time.h>
#include <NTPClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>

const char* ssid = ""; // Tu nombre de red (SSID)
const char* password = ""; // Tu contraseña de Wi-Fi
#define TASKS_PERIODICITY 1000

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
const long utcOffsetInSeconds = -21600; // CST offset

WiFiUDP Udp;
IPAddress remoteIP(, , , ); // IP de la PC
unsigned int localPort = 2390;        
unsigned int remotePort = 12345;    

Adafruit_MPU6050 mpu;

// Define two tasks
void Task1(void *pvParameters);
void Task2(void *pvParameters);

// Task handles
TaskHandle_t task1Handle = NULL;
TaskHandle_t task2Handle = NULL;

// Timer handles
TimerHandle_t timer1;

// Timer callback functions
void Timer1Callback(TimerHandle_t xTimer);

// Define a semaphore
SemaphoreHandle_t initSemaphore;

// Queue handle
QueueHandle_t myQueue;

// Structure for queue data
typedef struct {
  sensors_event_t a;
  sensors_event_t g;
  sensors_event_t temp;
  char timeStr[20];
} Data_t;

void setup() {
    Serial.begin(115200);
    Serial.println("MENSAJE INICIAL");
    // Create a binary semaphore
    //initSemaphore = xSemaphoreCreateBinary();

    // Create a queue capable of containing 10 Data_t structures
    myQueue = xQueueCreate(10, sizeof(Data_t));

    // Create tasks
    if (/*(initSemaphore != NULL) && */(myQueue != NULL)) {
      
      xTaskCreatePinnedToCore(
        Task1,    // Task function
        "Task1",  // Name of the task
        10000,    // Stack size
        NULL,     // Task input parameter
        1,        // Priority of the task
        &task1Handle,     // Task handle
        0);       // Core where the task should run
      vTaskSuspend(task1Handle); // Suspend itself
      xTaskCreatePinnedToCore(
        Task2,    // Task function
        "Task2",  // Name of the task
        10000,    // Stack size
        NULL,     // Task input parameter
        2,        // Priority of the task
        &task2Handle,     // Task handle
        0);       // Core where the task should run
      vTaskSuspend(task2Handle); // Suspend itself
    }

    // Create software timers
    
    timer1 = xTimerCreate(
      "Timer1",                // Timer name
      pdMS_TO_TICKS(TASKS_PERIODICITY),     // Timer period in ticks (1 second)
      pdTRUE,                  // Auto-reload (periodic)
      (void*)0,                // Timer ID
      Timer1Callback);         // Callback function
    
    // Perform initialization
    Serial.println("Initialization started.");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Conectado  a WiFi");

    
    Serial.print("IPv4 Local:");
    Serial.println(WiFi.localIP());
    Serial.print("IP Broadcast: ");
    Serial.println(WiFi.broadcastIP());
    Serial.print("IP DNS: ");
    Serial.println(WiFi.dnsIP());
    Serial.print("IP Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("IP SubnetMask: ");
    Serial.println(WiFi.subnetMask());
    
    Udp.begin(localPort);

    if (!mpu.begin()) {
        Serial.println("No se pudo encontrar el MPU6050");
        while (1);
    }
    Serial.println("MPU6050 encontrado!");
    Serial.println("");

    timeClient.begin();
    timeClient.setTimeOffset(utcOffsetInSeconds); 

    Serial.println("...Initialization complete...");

    // Signal that initialization is complete
    //xSemaphoreGive(initSemaphore);
    // Start the timers
    if(timer1 != NULL) {
      xTimerStart(timer1, 0);
    }
    //Serial.println("Task1: Resuming  >>");
    //vTaskResume(task1Handle); // Resume Task1
}

void loop() {
    ;;
}

// Task1 function
void Task1(void *pvParameters) {
 

  // Wait for initialization to complete
  //if (xSemaphoreTake(initSemaphore, portMAX_DELAY) == pdTRUE) {

    sensors_event_t a, g, temp;
    time_t rawtime;
    Data_t dataToSend;
    struct tm * ti;
    char timeStr[20];

    while (true) {
      
      /*PROCESS OF THE TASK*/
      Serial.println("Task1: <<Running>>");
      //vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second

      mpu.getEvent(&dataToSend.a, &dataToSend.g, &dataToSend.temp);
      timeClient.update(); // Actualizar la hora ANTES de usarla

      rawtime = timeClient.getEpochTime();
      ti = localtime (&rawtime);
      
      // Enviar datos por UDP
      strftime(dataToSend.timeStr, sizeof(dataToSend.timeStr), "%Y-%m-%d %H:%M:%S", ti);
      Serial.println("Task1: Sending Queue data...");
      //Serial.println((String)dataToSend);
      // Send data to the queue
      if (xQueueSend(myQueue, (void *)&dataToSend, portMAX_DELAY) != pdPASS) {
        Serial.println("Task1: Failed to send data to the queue.");
      }
      //Serial.println("Task1: Suspending ##");
      Serial.println("Task2: Resuming  >>");
      vTaskResume(task2Handle); // Resume Task2
      //vTaskSuspend(NULL); // Suspend itself
    
      /*END OF PROCESS OF THE TASK*/  
    }
  //}else{
  //    Serial.println("Task1: Failed to get Semaphore");
  //}   
}

// Task2 function
void Task2(void *pvParameters) {

  //if (xSemaphoreTake(initSemaphore, portMAX_DELAY) == pdTRUE) {
    Data_t receivedData;

    while (true) {
        vTaskSuspend(task1Handle); // Suspend itself
        Serial.println("Task2: <<Running>>");
        //vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay for 2 seconds
      

        if (xQueueReceive(myQueue, (void *)&receivedData, portMAX_DELAY) == pdPASS) {
          Serial.println("Task2: Received Queue data");
          
          
          String data = String(receivedData.timeStr) + "," + 
                        String(receivedData.a.acceleration.x, 3) + "," + String(receivedData.a.acceleration.y, 3) + "," + String(receivedData.a.acceleration.z, 3) + "," +
                        String(receivedData.g.gyro.x, 3) + "," + String(receivedData.g.gyro.y, 3) + "," + String(receivedData.g.gyro.z, 3) + "," +
                        String(receivedData.temp.temperature); // Incluye temperatura

          Udp.beginPacket(remoteIP, remotePort);
          Udp.print(data);
          Udp.endPacket();  
          Serial.println("Task2: Sending Data Frame by UDP Protocol");
          Serial.println(data);

        } else {
          Serial.println("Task2: Failed to receive data from the queue.");
        }
        
        Serial.println("Task2: Suspending ##");
        //Serial.println("Task1: Resuming  >>");
        //vTaskResume(task1Handle); // Resume Task1
        xTimerStart(timer1, 0);
        vTaskSuspend(NULL); // Suspend itself
    } 
  //}else{
  //  Serial.println("Task2: Failed to get Semaphore");
  //}
}

// Timer1 callback function

void Timer1Callback(TimerHandle_t xTimer) {
  // Perform the alarm action for Timer1
  xTimerStop(timer1, 0);
  Serial.println("Timer1 alarm triggered!");
  Serial.println("Task1: Resuming  >>");
  vTaskResume(task1Handle); // Resume Task1 

}

