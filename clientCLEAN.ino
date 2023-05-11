/*
 *  This sketch sends a message to a TCP server
 *
 */

#include <WiFi.h>
#include <WiFiMulti.h>

WiFiMulti WiFiMulti;
WiFiClient client;

double initTime;
double deltaTime;
bool marked = false;

unsigned volatile long pulseHigh, pulseLow, pulseTotal;
unsigned volatile long send;

void setup()
{
    pinMode(12, INPUT);
    Serial.begin(115200);
    delay(10);

    // We start by connecting to a WiFi network
    WiFiMulti.addAP("Angelas hotspot", "fbry1038");

    Serial.println();
    Serial.println();
    Serial.print("Waiting for WiFi... ");

    while(WiFiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    delay(500);

    const uint16_t port = 80;
    const char * host = "192.168.81.100"; // ip or dns

    Serial.print("Connecting to ");
    Serial.println(host);

    // Use WiFiClient class to create TCP connections
    // WiFiClient client;

    if (!client.connect(host, port)) {
        Serial.println("Connection failed.");
        Serial.println("Waiting 5 seconds before retrying...");
        delay(5000);
        return;
    }

}


void loop()
{
  pulseHigh = pulseIn(12, 1);
  pulseLow = pulseIn(12, 0);
  pulseTotal = pulseHigh + pulseLow;

    if (pulseTotal > 33.5) { //less than 29.9kHz
    client.print(0);
  } else if (pulseTotal <= 33.5 && pulseTotal > 32.787) { //btwn 29.9k and 30.5k
    client.print(1);
  } else if (pulseTotal <= 32.787 && pulseTotal > 31.25) { //btwn 30.5k and 32.5k
    client.print(2);
  } else if (pulseTotal <= 31.25) { //greater 32.5k
    client.print(3);
  }
  delay(1000);
}
