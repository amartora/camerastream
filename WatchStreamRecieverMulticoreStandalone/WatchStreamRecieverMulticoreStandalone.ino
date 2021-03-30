#include "config.h"
#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid     = "";
const char* password = "";

WiFiUDP UDPTestServer;
unsigned int UDPPort = 1337;



TTGOClass *ttgo;



uint16_t cattedPacket[721];

TaskHandle_t Task1;
TaskHandle_t Task2;

void setup(){
  Serial.begin(115200);
  
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected"); 
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  UDPTestServer.begin(UDPPort);

  ttgo->tft->setSwapBytes(true);

  
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */                  
  delay(500); 

}

void Task1code( void * pvParameters ){
  uint16_t finalRowGroup[720];
  const int packetSize = 1442;
  byte packetBuffer[packetSize];
  
  
  for(;;){
    delay(1);
    int cb = UDPTestServer.parsePacket();
    if (cb) {
      UDPTestServer.read(packetBuffer, packetSize);

      for (int j = 0; j < 721; ++j){
        cattedPacket[j] = packetBuffer[0+(2*j)] << 8 | packetBuffer[1+(2*j)];
      }
    }  
  }
}
 void Task2code( void * pvParameters ){
  for(;;){
    ttgo->tft->pushImage(-1, ((cattedPacket[0]-1)*3)+32, 240, 3, cattedPacket);
  }
 }
  
 
  
void loop(){
}
