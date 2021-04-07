/*
  -----------------------------------------------------------------------------------------------
 | BLE_IMU_PERIPHERAL - Wireless IMU Communication with central device
 |
 | Arduino Boards Tested: Nano 33 BLE Sense as a peripheral & Nano 33 BLE as central.
 | Code not tested for multiple peripherals

 | This sketch works alongside the BLE_IMU_CENTRAL sketch to communicate with an Arduino Nano 33 BLE. 
 | This sketch can also be used with a generic BLE central app, like LightBlue (iOS and Android) or
 | nRF Connect (Android), to interact with the services and characteristics created in this sketch.
 
 | This example code is adapted from the ArduinoBLE library, available in the public domain.
 | Authors: Aaron Yurkewich & Pilar Zhang Qiu
 | Latest Update: 25/02/2021
  -----------------------------------------------------------------------------------------------
*/

#include <ArduinoBLE.h>
//#include <Arduino_LSM9DS1.h>
#include <CapacitiveSensor.h>
#include <LiquidCrystal.h>
#include <Arduino_LSM6DS3.h> // Uncomment this if your peripheral is the Nano 33 IoT

// ------------------------------------------ BLE UUIDs ------------------------------------------
#define BLE_UUID_PERIPHERAL               "19B10004-E8F2-537E-4F6C-D104768A1214" //please chnage to a unique value that matches BLE_IMU_CENTRAL
#define BLE_UUID_CHARACT_LED              "19B10005-E8F2-537E-4F6C-E104768A1214" //please chnage to a unique value that matches BLE_IMU_CENTRAL
#define BLE_UUID_CHARACT_ACCX             "29B10004-E8F2-537E-4F6C-a204768A1215" //please chnage to a unique value that matches BLE_IMU_CENTRAL
#define BLE_UUID_CHARACT_ACCY             "39B10004-E8F2-537E-4F6C-a204768A1215" //please chnage to a unique value that matches BLE_IMU_CENTRAL
#define BLE_UUID_CHARACT_ACCZ             "49B10004-E8F2-537E-4F6C-a204768A1215" //please chnage to a unique value that matches BLE_IMU_CENTRAL

#define BLE_UUID_CHARACT_GYROX             "59B10004-E8F2-537E-4F6C-a204768A1215"  //please change to a unique value that matches BLE_IMU_PERIPHERAL
#define BLE_UUID_CHARACT_GYROY             "69B10004-E8F2-537E-4F6C-a204768A1215"  //please change to a unique value that matches BLE_IMU_PERIPHERAL
#define BLE_UUID_CHARACT_GYROZ             "79B10004-E8F2-537E-4F6C-a204768A1215"  //please change to a unique value that matches BLE_IMU_PERIPHERAL

BLEService LED_IMU_Service(BLE_UUID_PERIPHERAL); // BLE LED Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic(BLE_UUID_CHARACT_LED, BLERead | BLEWrite);
BLEFloatCharacteristic accXCharacteristic(BLE_UUID_CHARACT_ACCX, BLERead | BLENotify | BLEWrite);
BLEFloatCharacteristic accYCharacteristic(BLE_UUID_CHARACT_ACCY, BLERead | BLENotify | BLEWrite);
BLEFloatCharacteristic accZCharacteristic(BLE_UUID_CHARACT_ACCZ, BLERead | BLENotify | BLEWrite);

BLEFloatCharacteristic gyroXCharacteristic(BLE_UUID_CHARACT_GYROX, BLERead | BLENotify | BLEWrite);
BLEFloatCharacteristic gyroYCharacteristic(BLE_UUID_CHARACT_GYROY, BLERead | BLENotify | BLEWrite);
BLEFloatCharacteristic gyroZCharacteristic(BLE_UUID_CHARACT_GYROZ, BLERead | BLENotify | BLEWrite);

const int ledPin = LED_BUILTIN; // pin to use for the LED
float xAcc, yAcc, zAcc;
float xGyro, yGyro, zGyro;
float xGyro_cal, yGyro_cal, zGyro_cal;

//CapSense
CapacitiveSensor cs_13_12 = CapacitiveSensor(13, 12);
CapacitiveSensor cs_8_7 = CapacitiveSensor(8, 7);

////LED
int red_light_pin= 11;
int green_light_pin = 10;
int blue_light_pin = 9;

// ------------------------------------------ VOID SETUP ------------------------------------------
void setup() {
//  //CapSense
  cs_13_12.set_CS_AutocaL_Millis(0xFFFFFFFF); // turn off autocalibrate on channel 1 - just as an example Serial.begin(9600);
  cs_8_7.set_CS_AutocaL_Millis(0xFFFFFFFF);
  Serial.begin(9600); 
  //while (!Serial); //uncomment to view the IMU data in the peripheral serial monitor

  // begin IMU initialization
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  
//  //LED
  pinMode(red_light_pin, OUTPUT);
  pinMode(green_light_pin, OUTPUT);
  pinMode(blue_light_pin, OUTPUT);
//  //LCD screen
//  //lcd.begin(16,2); //size of screen 
//  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);

  // begin BLE initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("BLE_IMU");
  BLE.setAdvertisedService(LED_IMU_Service);

  // add the characteristic to the service
  LED_IMU_Service.addCharacteristic(switchCharacteristic);
  LED_IMU_Service.addCharacteristic(accXCharacteristic);
  LED_IMU_Service.addCharacteristic(accYCharacteristic);
  LED_IMU_Service.addCharacteristic(accZCharacteristic);

  LED_IMU_Service.addCharacteristic(switchCharacteristic);
  LED_IMU_Service.addCharacteristic(gyroXCharacteristic);
  LED_IMU_Service.addCharacteristic(gyroYCharacteristic);
  LED_IMU_Service.addCharacteristic(gyroZCharacteristic);

  // add service
  BLE.addService(LED_IMU_Service);

  // set the initial value for the characeristic:
  switchCharacteristic.writeValue(0);


  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");
}

// ------------------------------------------ VOID LOOP ------------------------------------------
void loop() {
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();


  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written()) {
        if (switchCharacteristic.value()) {   // any value other than 0
          Serial.println("LED on");
          digitalWrite(ledPin, HIGH);         // will turn the LED on
        } else {                              // a 0 value
          Serial.println(F("LED off"));
          digitalWrite(ledPin, LOW);          // will turn the LED off
        }
      }

  long start = millis();
  long csValue1 = cs_13_12.capacitiveSensor(30);
  long csValue2 = cs_8_7.capacitiveSensor(30);
  
        if (IMU.accelerationAvailable() &&
              IMU.gyroscopeAvailable()) {
 
          // read accelerometer &and gyrometer:
          IMU.readAcceleration(xAcc, yAcc, zAcc);
          IMU.readGyroscope(xGyro, yGyro, zGyro);

          // calibration of gyroscope
          xGyro_cal = xGyro - 2.01;
          yGyro_cal = yGyro + 3.23;
          zGyro_cal = zGyro + 2.44;

  Serial.print("\t"); // tab character for debug window spacing
  
  if (csValue1>500 && csValue2>500){             //cant yet differentiate 2 touches from 1
    Serial.print("Touched ");
    RGB_colour(0, 255, 0); // Green 
              
        accXCharacteristic.writeValue(xAcc);
        accYCharacteristic.writeValue(yAcc);
        accZCharacteristic.writeValue(zAcc);

        gyroXCharacteristic.writeValue(xGyro_cal);
        gyroYCharacteristic.writeValue(yGyro_cal);
        gyroZCharacteristic.writeValue(zGyro_cal);

        Serial.print(xAcc); 
        Serial.print('\t');
        Serial.print(yAcc); 
        Serial.print('\t');         
        Serial.print(zAcc); 
        Serial.println('\t'); 

        Serial.print(xGyro_cal); 
        Serial.print('\t');
        Serial.print(yGyro_cal); 
        Serial.print('\t');         
        Serial.print(zGyro_cal); 
        Serial.println('\t'); 

          }
    else {

        accXCharacteristic.writeValue(0);
        accYCharacteristic.writeValue(0);
        accZCharacteristic.writeValue(0);

        gyroXCharacteristic.writeValue(10000);
        gyroYCharacteristic.writeValue(10000);
        gyroZCharacteristic.writeValue(10000);
      
      Serial.print(csValue1); // print sensor output 1
      Serial.print(csValue2);
      RGB_colour(10,0,0); // dark green 
      }
  delay(10); // arbitrary delay to limit data to serial port
  }
      }

    }

      // when the central disconnects, print it out:
      Serial.print(F("Disconnected from central: "));
      Serial.println(central.address());
    }

void RGB_colour(int red_light_value, int green_light_value, int blue_light_value)
 {
  analogWrite(red_light_pin, red_light_value);
  analogWrite(green_light_pin, green_light_value);
  analogWrite(blue_light_pin, blue_light_value);
 }
