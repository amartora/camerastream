#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>


#include "camera_config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_camera.h"

#include <WiFi.h>
#include <WiFiUdp.h>

WiFiUDP Udp;

const char *ssid = "";
const char *password = "";

const char * udpAddress = "192.168.x.xx";
const int udpPort = 1337;

uint8_t packet[1442];


static const char *TAG = "example:take_picture";

static esp_err_t init_camera()
{
    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        Serial.println("Camera Init Failed");
        return err;
    }

    return ESP_OK;
}

void setup() {

  psramInit();
  
  init_camera();

  Serial.begin(115200);

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print(" connected at");
  Serial.println (WiFi.localIP());

  Udp.begin(WiFi.localIP(),udpPort);

}

void loop() {
 
  //allocating the space in PSRAM for the pixel array, this also counts as declaring the array, locally in void loop, since doing so globally didn't work with the PSRAM
  unsigned char *stream = (unsigned char *) ps_malloc(90000 * sizeof(unsigned char));

 
  Serial.println ("Taking picture...");
  camera_fb_t *pic = esp_camera_fb_get();

  // use pic->buf to access the image
  
  Serial.print("Picture taken! Its size was: ");
  Serial.print(pic->len);
  Serial.println("bytes");
  
  Serial.print("Width is: ");
  Serial.println(pic->width);
  
  Serial.print("Height is: ");
  Serial.println(pic->height);
 
  //transferring the pixel array to stream[]
  for (int i = 0; i <84480; ++i){
    stream[i] = pic->buf[i];
  }
 //spliting the image into packets and adding the marker
 //there should be 58 packets, each being a total of 1442 8-bit values: 1 byte of 0, in the beginning, 1 byte making up the marker after, and 720 16-bit pixels split into bytes 
 //( i could actually fit 59 packets but it messd up the image, and i dont feel like figuring out how to deal with the last packet
 for (int g = 1; g <59; ++g){

  
    //adding the 0 and the marker
    packet[0] = 0;
    packet[1] = g;
    
    int counter = 2;
    
   for (int y = 1440*(g-1); y < 1440*(g); ++y){
     //adding the 1440 pixels after the marker
     packet[counter] = stream[y];
     counter++;
     
   }
    Serial.print("Size of packet: ");
    Serial.println(sizeof(packet));
    
    Udp.beginPacket(udpAddress,udpPort);
    Serial.print("Size of packet transmitting: ");
    Serial.println(Udp.write(packet, 1442));
    Udp.endPacket();
    
 }

  //freeing up the PSRAM, since without doing so, after a few loops, the PSRAM fills up and the ESP will crash
  free(stream);
  
  
}
