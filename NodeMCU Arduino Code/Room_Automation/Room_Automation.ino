#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <SoftwareSerial.h> 

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include "secrets.h"    // Copy secrets_example.h to secrets.h and fill in your own credentials

#define AIO_SERVER      "io.adafruit.com"                     // Server Address
#define AIO_SERVERPORT  1883                                  // Standard Port of the Server
// AIO_USERNAME and AIO_KEY come from secrets.h

/* We are using two Micro-controllers(ESPRESSIF IoT and PIC) and communicationg with each other using Serial Communication. 
 * One Interface is used for Programming of ESPRESSIF Micro-controller, other one is used for communiation with the PIC Micro-controller.
 * Hardware UART Port of NodeMCU (RX,TX = GPIO3,GPIO1) is permenantly engaged with the Serial Monitor and IDE. So we're leaving this.
 * Instead we are using Software UART Port (SUART Port: GPIO4/D2, GPIO5/D1 = SRX, STX) to communicate with NodeMCU.
 */

#define SRX 4                       // Software Serial Pin for Receiving is on GPIO4/D2
#define STX 5                       // Software Serial Pin for Transmission is on GPIO5/D1

SoftwareSerial SUART(12,13);          // SRX = DPin-6; STX = DPin-7
SoftwareSerial btSerial(4, 5);    // BT_RX = DPin-2; BT_TX = DPin-1

#ifndef STASSID
#define STASSID WIFI_SSID       // from secrets.h
#define STAPSK  WIFI_PASSWORD   // from secrets.h
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;


WiFiClient client;     // Create an ESP8266 WiFiClient class to connect to the MQTT server.

/* Once device is connected to the WiFi, it has to connect to Adafruit Cloud,
   We have this in-built function, we pass the Client name, Server Address, Port, User ID and Password
   After giving these, device gets connected to Adafruit Cloud. 

   For Publishing the information, we have to give the name of the FEED on which we r publishing the information.
*/
 
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);        // Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Subscribe Room_Light = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Room Light");
Adafruit_MQTT_Subscribe Cooling_System = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Cooling System");

void MQTT_connect();

void setup() 
{

  Serial.begin(115200);   // Establishing Communication channel for ESPRESSIF Microcontroller for Data Transmission, Enables Serial Monitor by engaging with Hardware UART GPIO pins - 1,3 
  btSerial.begin(9600);     // bluetooth module baudrate 
  delay(1000); 
  Serial.println("Bluetooth Comunication Started");
  
  WiFi.disconnect();
  delay(20);              // 20ms Delay   

  SUART.begin(9600);    // NodeMCU prefers high Bd to work
  
 // Connect to WiFi access point.

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
   /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);  // In-built function to start the WiFi Communication, Passing SSID and Password

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");    // Print "..." till the time connection is established            
  }

  Serial.println();
  Serial.println("WiFi connected");   // Once the Communication is established, it'll write this part
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());

  mqtt.subscribe(&Room_Light);    // Once Communication is Successful, we will subscribe to a particular Feed for receiving the data
  mqtt.subscribe(&Cooling_System);     // Subscribing to next Feed
  
}

uint32_t x=0;

void loop()
{
     if (btSerial.available() > 0)  // check if bluetooth module sends some data to esp8266
     {   
       char data = btSerial.read();  // read the data from HC-05
       switch (data)
       {
         case 'a':         // if receive data is 'a'
           SUART.print('a');
           Serial.println("a is received");
           break;
         case 'b':              // if receive data is 'b'
           SUART.print('b');
           Serial.println("b is received");
           break;
         case 'c':         // if receive data is 'c'
           SUART.print('c');
           Serial.println("c is received");
           break;
         case 'd':              // if receive data is 'd'
           SUART.print('d');
           Serial.println("d is received");
           break;
         default:
           break;
       }
     }
    
     //~~~~~~~~~~~~~~~~~~~~~~~ Again we'll keep checking the WiFi connectivity ~~~~~~~~~~~~~~~~~~~~~~~~//
     // Usually if your device is connected and somehow connection gets distrupted because of network problems then this loop will be working
     
     if(WiFi.status() != WL_CONNECTED) 
     {
       
       WiFi.begin(ssid, password);

      while (WiFi.status() != WL_CONNECTED)
      {
  
        delay(500);
      
        Serial.print(".");

      }

      Serial.println();

      Serial.println("WiFi connected");

      Serial.println("IP address: "); 
      
      Serial.println(WiFi.localIP());

     }
     //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

  MQTT_connect();   // Connecting to the Server

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ For Subscription Part ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

  Adafruit_MQTT_Subscribe *subscription;

  while ((subscription = mqtt.readSubscription(5000)))
  {
    //////////////////////////////////////// Room Light Subscription /////////////////////////////////////////////////////
    
    if(subscription == &Room_Light) // If we Subscribed to the Devices_Switch Feed,
    {

      Serial.print(F("Got: "));

      Serial.println((char *)Room_Light.lastread);      // The last read value of the Feed on Device cloud will be printed

      if (!strcmp((char*)Room_Light.lastread, "ON"))   // In the Feed, if we're getting the data as "ON"
      {
        SUART.print('A');    // We will Turn ON the Device using PIC Micro-controller by sending the Character through Serial Communication, PIC will receive this character using USART Protocol
        Serial.println("Channel 1 ON");       
      }
      else
      {
        SUART.print('B');  // We will Turn OFF the Device using PIC Micro-controller
        Serial.println("Channel 1 OFF");           
      }

    }

    //////////////////////////////////////// Dim Light Subscription /////////////////////////////////////////////////////

    if(subscription == &Cooling_System) // If we Subscribed to the Devices_Switch Feed,
    {

      Serial.print(F("Got: "));

      Serial.println((char *)Cooling_System.lastread);      // The last read value of the Feed on Device cloud will be printed

      if (!strcmp((char*)Cooling_System.lastread, "ON"))   // In the Feed, if we're getting the data as "ON"
      {
        SUART.print('C');    // We will Turn ON the Device using PIC Micro-controller by sending the Character through Serial Communication, PIC will receive this character using USART Protocol
        Serial.print("Cooling System is Turned ON");       
      }
      else
      {
        SUART.print('D');  // We will Turn OFF the Device using PIC Micro-controller
        Serial.print("Cooling System is Turned OFF");           
      }

    }
    
}

}

void MQTT_connect() 
{
  /* If some problems occurs while connectiong to Adafruit Server through MQTT Protocol (maybe because of network connectivity or server down), 
   * if it keeps disonnecting, we have to try few times for the connection till the time we are not get connected.  
   */  
  int8_t ret;

  if (mqtt.connected()) 
  {
    return;   // Stop if already connected.
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;

  while ((ret = mqtt.connect()) != 0)  // If mqtt is not connected    
  {
       Serial.println(mqtt.connectErrorString(ret));

       Serial.println("Retrying MQTT connection in 5 seconds...");

       mqtt.disconnect();   // First Disconnect and wait then try connecting

       delay(5000);   // wait 5 seconds

       retries--;

       if (retries == 0) 
       {
        while (1);    // basically die and wait for WDT to reset me
       }

  }

  Serial.println("MQTT Connected!");

}
