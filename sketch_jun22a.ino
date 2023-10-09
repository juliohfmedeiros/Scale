#include "Firebase_Arduino_WiFiNINA.h"
#include <HX711_ADC.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

#define FIREBASE_HOST "*********.firebasedatabase.app"
#define FIREBASE_AUTH "*********"
#define WIFI_SSID "*********"
#define WIFI_PASSWORD "*********"

const int HX711_dout = 2;
const int HX711_sck = 3;

FirebaseData fbdo;

FirebaseData stream;

int status = WL_IDLE_STATUS;
unsigned long sendDataPrevMillis = 0;
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
unsigned long t = 0;
int prevValue = 1;

void setup()
{
  while (status != WL_CONNECTED)
  {
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(300);
  }
  //Provide the autntication data
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, WIFI_SSID, WIFI_PASSWORD);
    Firebase.reconnectWiFi(true);
    LoadCell.begin();
  float calibrationValue;
  calibrationValue = 374.96;
#if defined(ESP8266)|| defined(ESP32)
  EEPROM.begin(512);
#endif
  EEPROM.get(calVal_eepromAdress, calibrationValue);
  unsigned long stabilizingtime = 2000;
  boolean _tare = true;
  LoadCell.start(stabilizingtime, _tare);
  LoadCell.setCalFactor(calibrationValue);

}

void loop()
{
  if (millis() - sendDataPrevMillis > 300 || sendDataPrevMillis == 0)
  {
    status=WiFi.status();
    static boolean newDataReady = 0;
    sendDataPrevMillis = millis();

    if (status==WL_DISCONNECTED || status==WL_CONNECTION_LOST){
      while ( status != WL_CONNECTED){
        status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        delay(10000);
      }
      Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, WIFI_SSID, WIFI_PASSWORD);
    }
    if (LoadCell.update()) newDataReady = true;
    if (newDataReady){
      int value = LoadCell.getData();
      if (prevValue != value) {
        Firebase.setFloat(fbdo, "/scale", value);
        newDataReady = 0;
        prevValue = value;
        LoadCell.powerDown();
        LoadCell.powerUp();
        fbdo.clear();
      }
    }

  }
}