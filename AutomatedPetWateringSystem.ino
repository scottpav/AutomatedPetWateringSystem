#include <ESP8266WiFi.h>
#include "Gsender.h"

#pragma region Globals

const char* ssid = "YOUR_WIFI_SSID";                           // WIFI network name
const char* password = "YOUR_WIFI_PASSWORD";                       // WIFI network password
uint8_t connection_state = 0;                    // Connected to WIFI or not
uint16_t reconnect_interval = 10000;             // If not connected wait time to try again

#pragma endregion Globals
#define SWITCH_PIN 16
#define RELAY_PIN 5
int switchState = 0;         // variable for reading the switch

uint8_t WiFiConnect(const char* nSSID = nullptr, const char* nPassword = nullptr)
{
    static uint16_t attempt = 0;
    Serial.print("Connecting to ");
    if(nSSID) {
        WiFi.begin(nSSID, nPassword);  
        Serial.println(nSSID);
    } else {
        WiFi.begin(ssid, password);
        Serial.println(ssid);
    }

    uint8_t i = 0;
    while(WiFi.status()!= WL_CONNECTED && i++ < 50)
    {
        delay(200);
        Serial.print(".");
    }
    ++attempt;
    Serial.println("");
    if(i == 51) {
        Serial.print("Connection: TIMEOUT on attempt: ");
        Serial.println(attempt);
        if(attempt % 2 == 0)
            Serial.println("Check if access point available or SSID and Password\r\n");
        return false;
    }
    Serial.println("Connection: ESTABLISHED");
    Serial.print("Got IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void Awaits()
{
    uint32_t ts = millis();
    while(!connection_state)
    {
        delay(50);
        if(millis() > (ts + reconnect_interval) && !connection_state){
            connection_state = WiFiConnect();
            ts = millis();
        }
    }
}
void sendAlert(){
   connection_state = WiFiConnect();
    if(!connection_state)  // if not connected to WIFI
        Awaits();          // constantly trying to connect
    Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
    String subject = "Water Supply";
    if(gsender->Subject(subject)->Send("YOUR_ALERT_EMAIL_ADDRESS@GMAIL.COM", "The dogs water was low. A refill cycle was initiated. Please check the resivor ASAP.")) {
        Serial.println("Message send.");
    } else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError());
    }
}

void setup()
{
   Serial.begin(115200);
   pinMode(RELAY_PIN,OUTPUT);
   pinMode(SWITCH_PIN, INPUT);
   digitalWrite(RELAY_PIN, HIGH);
}

void loop(){
    // read the state of the switch value:
    switchState = digitalRead(SWITCH_PIN);
    // check if the switch is tripped.
      if (switchState == HIGH) {
        Serial.print("Switch is open.");
        // water level is low, pump on
        Serial.print("Turning on pump.");
        digitalWrite(RELAY_PIN, LOW);
        //Send an email alert
        sendAlert();
        delay(1000);
      } else {
        Serial.print("Switch is closed.");
        // let the pump run for 30 more seconds to account for switch height
        delay(30000);
        Serial.print("Turning off pump.");
        digitalWrite(RELAY_PIN, HIGH);
      }
  }
