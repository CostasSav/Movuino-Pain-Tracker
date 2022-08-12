#include <bluefruit.h>
#include "RTClib.h"
#include "Wire.h"

uint8_t uvindexvalue = 0x0;
uint8_t temp_uvindexvalue = 0x0;
uint32_t timeSent = 0;
#define UUID16_SVC_ENVIRONMENTAL_SENSING 0x181A
#define UUID16_CHR_UV_INDEX 0x2A76

uint8_t painTab[255][4]; //can store 255 pain values  -> painTab[n'th_value][0] = intensity and painTab[n'th_value][1] = date

int i = 0;  //index to fill the painTab
int i_max = 0;  //to stop sending values when reaching last painTab value

RTC_PCF8523 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

BLEService environmental_sensing_service = BLEService(UUID16_SVC_ENVIRONMENTAL_SENSING);
BLECharacteristic uv_index_characteristic = BLECharacteristic(UUID16_CHR_UV_INDEX);

BLEService ledService("19B10010-E8F2-537E-4F6C-D104768A1214");

BLECharCharacteristic ledCharacteristic("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
//create button
//BLECharCharacteristic buttonCharacteristic("19B10012-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
 



void setup() {
  Serial.begin(115200);
  delay(500);
  //Serial.println("Start!");

  while (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
  //  abort();
  }

  if (rtc.lostPower()) {
    //Serial.println("RTC is NOT initialized, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    //
    // Note: allow 2 seconds after inserting battery or applying external power
    // without battery before calling adjust(). This gives the PCF8523's
    // crystal oscillator time to stabilize. If you call adjust() very quickly
    // after the RTC is powered, lostPower() may still return true.

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));

  // When the RTC was stopped and stays connected to the battery, it has
  // to be restarted by clearing the STOP bit. Let's do this to ensure
  // the RTC is running.
  rtc.start();
 }

  Bluefruit.begin();
  Bluefruit.setName("Palm");

  setupESService();
  startAdv();
}

void loop() {
  DateTime now = rtc.now();
  temp_uvindexvalue = analogRead(A0);
  /*if (temp_uvindexvalue > 65410){
    temp_uvindexvalue = temp_uvindexvalue - 65400;
    painTab[i][0] = temp_uvindexvalue;
    painTab[i][1] = now.unixtime() - 16590;
    i++;
  }*/
  //Serial.println(now.unixtime());
  timeSent = now.unixtime() - 1659000000;
  //Serial.println(timeSent);
  painTab[i][0] = temp_uvindexvalue - 65770;
  painTab[i][1] = (uint8_t)((timeSent & 0xFF00)>> 8) ; //keep first 8 bits
  //Serial.print(painTab[i][1]);
  painTab[i][2] = (uint8_t)(timeSent & 0xFF); //keep second 8 bits
  //Serial.print(painTab[i][2]);
  
  
  uvindexvalue = painTab[i][0];
  Serial.print("Intensity: ");
  Serial.println(uvindexvalue);
  if (uv_index_characteristic.indicate(&uvindexvalue, sizeof(uvindexvalue))) {
    //Serial.print("Updated Intensity: ");
    //Serial.println(uvindexvalue);
  } else {
    //Serial.println("Intensity Indicate not set");
  }

  uvindexvalue = (int)painTab[i][1];
  //Serial.print("Binary date (first 8 bits): ");
  //Serial.println(uvindexvalue);
  if (uv_index_characteristic.indicate(&uvindexvalue, sizeof(uvindexvalue))) {
    //Serial.print("Updated date1: ");
    //Serial.println(uvindexvalue);
  } else {
    //Serial.println("date1 Indicate not set");
  }

  uvindexvalue = (int)painTab[i][2];
  //Serial.print("Binary date (second 8bits): ");
  //Serial.println(uvindexvalue);
  if (uv_index_characteristic.indicate(&uvindexvalue, sizeof(uvindexvalue))) {
    //Serial.print("Updated date2: ");
    //Serial.println(uvindexvalue);
  } else {
    //Serial.println("date2 Indicate not set");
  }

  if(i==255){
    i=0;
  }
  /* if (connected to the server) {
   *  i_max = i;
   *  i = 0;
   *  for(int j; j<=i_max; j++){
   *    send painTab[j] to server
   *    painTab[j] = 0;
   *  }
   *}
   *  
   * 
   
  Serial.print(" since midnight 1/1/1970 = ");
  Serial.print(now.unixtime());
  Serial.println("s");
*/

  //Serial.println("rtc");

  delay(4000);
}

void startAdv(void) { //start showing on bluetooth receiver devices
  Bluefruit.Advertising.addService(environmental_sensing_service);

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0);
}

void setupESService(void) {
  environmental_sensing_service.begin();
  uv_index_characteristic.setProperties(CHR_PROPS_INDICATE);
  uv_index_characteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  uv_index_characteristic.setFixedLen(1);
  uv_index_characteristic.begin();
  uv_index_characteristic.write(&uvindexvalue, sizeof(uvindexvalue));
}
