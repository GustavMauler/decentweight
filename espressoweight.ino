#include <EEPROM.h>
#include <Wire.h>

#include "SparkFun_Qwiic_Relay.h"
#define RELAY_ADDR 0x18 // default address from docs. Alternate is 0x19
Qwiic_Relay relay(RELAY_ADDR); 

#include "SparkFun_Qwiic_Twist_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_Twist
TWIST twist;                                      //Create instance of this object

#include <SparkFun_Alphanumeric_Display.h> //Click here to get the library: http://librarymanager/All#SparkFun_Qwiic_Alphanumeric_Display by SparkFun
HT16K33 display;

#include <ArduinoBLE.h>

const String SCALE_NAME="Decent Scale";
const int GRAMS_LOCATION = 0; //just picking the first one. If you use this for something else, move it.
int grams = 0;
const int GRAMS_EPSILON = 1;
const byte TARE_COMMAND[7] = {(byte)0x03,(byte)0x0F,(byte)0x00,(byte)0x00,(byte)0x00,(byte)0x00,(byte)0x0C};//from decent docs

void printWeight(){
  if (grams < 10){
    display.print("0"+String(grams)+" g");
  }
  else{
    display.print(String(grams)+" g");
  }
}


void resetBLE(){
  if(BLE.connected()){
    BLE.disconnect();
  }

  if (!BLE.begin()) {
    dispErr("starting Bluetooth® Low Energy module failed!");
    while (1);
  }
  Serial.println("Scanning for Scale");
  twist.setColor(0,0,255);
  BLE.scan();
}

void dispErr(const String& errmsg){
  Serial.println(errmsg);
  if(twist.isConnected()){
    twist.setColor(255,0,0);
  }
  if(display.isConnected(0)){
    display.println("ERR");
  }
}

//runs once at startup
void setup() {
  Wire.begin(); 
  Serial.begin(115200);

  EEPROM.init(); //artemis is technically flash, and init does the routing
  grams = EEPROM.read(GRAMS_LOCATION);
  //Initially eeprom could be anything 0-255. Should be fine after setting once, though
  if(grams > 99){ 
    grams = 99;
  }

  //we'll do the knob first because it's our easiest error indicator  
  if (twist.begin() == false)
  {
    Serial.println("Twist does not appear to be connected. Please check wiring. Freezing...");
    while (1);
  }
  twist.setColor(0,255,0);

  if (display.begin() == false)
  {
    dispErr("Display device did not acknowledge! Freezing.");
    while (1);
  }
  printWeight();

  if(!relay.begin()){
    dispErr("Check connections to Qwiic Relay.");
    while(1);
  }
  relay.turnRelayOff(); //Should be a no-op

  resetBLE();
}

void connectedLoop(BLEDevice scale){
  if (scale.connect()) {
    Serial.println("Connected to scale");
  }
  else {
    dispErr("Failed to connect to scale");
    return;//go retry
  }

  //This allows characteristic discovery, too
  if (!scale.discoverAttributes()) {
    dispErr("Attribute discovery failed!");
    return;
  }
  
  //read-characteristic for finding scale data
  BLECharacteristic weight = scale.characteristic("fff4");//from decent docs
  if(!weight){
    dispErr("Weight not found...");
    return;
  }
  else{
    weight.subscribe();
  }

  //write-characteristic for sending commands to the scale
  BLECharacteristic command_scale = scale.characteristic("36f5");//from decent docs
  if(!command_scale){
    dispErr("Unable to command scale");
    return;
  }
  Serial.println("Ready to go");

  bool brewing=false;
  while(scale.connected()){
      if(brewing){
        //this happens.... all the time. It "updates" even if it hasn't actually updated.
        if(weight.valueUpdated()){
          //bytes 0,1: 03 (decent) CE or CA (Equalized or Altered)
          //bytes 2,3: (grams weight *10 as 2 byte signed short int)
          //bytes 4+: time, xor checksum etc.

          
          //should be 7-10 bytes.Making it 16 in case they up size again on a newer scale version
          byte wVal[16];
          weight.readValue(&wVal,16);

          //since they send us altered vs equal, let's filter to lower spamminess
          if(wVal[1] == 0xCA){
            const short curWeight = (wVal[2]<<8) | (wVal[3] & 0xff);
            Serial.print(curWeight/10.0);
            Serial.println("g");
            
            if (curWeight > (grams - GRAMS_EPSILON) * 10 ){
              Serial.println("OK STOP");
              relay.turnRelayOff(); 
              twist.setColor(255,255,255);
              brewing=false;
            }
          }
        }
    }
    else{
      if (twist.isClicked()){
        //indicate something is happening
        Serial.println("GO GO GO!");
        twist.setColor(255,0,128);

        //send a tare command to the scale
        command_scale.writeValue(TARE_COMMAND,7);

        //start espresso
        relay.turnRelayOn();
        brewing=true;

        EEPROM.write(GRAMS_LOCATION,grams); //save grams
        //sleep for a second. 
        //  Behavior of the Steel is that a quick switch press will use its volumetric mode. Stopping that mode early is a second switch press.
        //  To avoid this, we will just hold for at least 1 second before we start paying attention to the scale. This way we can just turn the relay on until we're ready to be done.
        delay(1000);
      }
      else if (twist.isMoved()){
        grams += twist.getDiff();
        if (grams > 99){
          grams = 99;
        }
        if (grams <1){
          grams = 1;
        }
        printWeight();
      }
    }
    delay(1);
  }
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    //there is a BLE.scanForName, but it seemed to hang...
    if(peripheral.localName() == SCALE_NAME){
      Serial.println("Found a scale");
      BLE.stopScan();
      twist.setColor(255,255,255);

      connectedLoop(peripheral);

      resetBLE(); // scale not connected
    }
    else{
      BLE.scan(); //wrong device. Resume scanning
    }
  }
}
