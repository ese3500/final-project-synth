/*
 WiFi Web Server LED Blink

 A simple web server that lets you blink an LED via the web.
 This sketch will print the IP address of your WiFi Shield (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to turn on and off the LED on pin 5.

 If the IP address of your shield is yourAddress:
 http://yourAddress/H turns the LED on
 http://yourAddress/L turns it off

 This example is written for a network using WPA2 encryption. For insecure
 WEP or WPA, change the Wifi.begin() call and use Wifi.setMinSecurity() accordingly.

 Circuit:
 * WiFi shield attached
 * LED attached to pin 5

 created for arduino 25 Nov 2012
 by Tom Igoe

ported for sparkfun esp32 
31.01.2017 by Jan Hendrik Berlin
 
 */

#include <WiFi.h>
#include <Wire.h>

const char* ssid     = "Angelas hotspot";
const char* password = "fbry1038";
//const char* ssid     = "simone phone";
//const char* password = "12345678";
int readInt = 0;

WiFiServer server(80);

void setup()
{
    Serial.begin(115200);
    pinMode(5, OUTPUT);      // set the LED pin mode

    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.begin();
    pinMode(12, OUTPUT);
    pinMode(13, OUTPUT);
}

void loop(){
  WiFiClient client = server.available();   // listen for incoming clients
  //delay(500);
  //Serial.println("workin"); 

  if (client) {                             // if you get a client,
    Serial.print("New Client.\n");           // print a message out the serial port
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        readInt = (int) client.read() - 48;
        //Serial.print((int) (client.read() - 48));
        if (readInt == 1) {
          digitalWrite(12, 0);
          digitalWrite(13, 1);
          Serial.println("1");
        } else if (readInt == 2) {
          digitalWrite(12, 1);
          digitalWrite(13, 0);
          Serial.println("2");
        } else if (readInt == 3) {
          digitalWrite(12, 1);
          digitalWrite(13, 1);
          Serial.println("3");
        } else {
          digitalWrite(12, 0);
          digitalWrite(13, 0);
          Serial.println("0");
        }
      }
    }
    // close the connection:
    //client.stop();
    Serial.println("Client Disconnected.");
  }
}
