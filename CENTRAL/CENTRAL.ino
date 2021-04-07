/*
  -----------------------------------------------------------------------------------------------
 | BLE_IMU_CENTRAL - Wireless IMU Communication with peripheral device
 |
 | Arduino Boards Tested: Nano 33 BLE Sense as a peripheral & Nano 33 BLE as central.
 | Code not tested for multiple peripherals
 |
 | This sketch works alongside the BLE_IMU_PERIPHERAL sketch to communicate with another Arduino BLE. 
 | This sketch can also be used with a generic BLE central app, like LightBlue (iOS and Android) or
 | nRF Connect (Android), to interact with the services and characteristics created in this sketch.
 |
 | This example code is adapted from the ArduinoBLE library, available in the public domain.
 | Authors: Aaron Yurkewich & Pilar Zhang Qiu
 | Latest Update: 25/02/2021
  -----------------------------------------------------------------------------------------------
*/

#include <ArduinoBLE.h>

// ------------------------------------------ BLE UUIDs ------------------------------------------
#define BLE_UUID_PERIPHERAL               "19B10004-E8F2-537E-4F6C-D104768A1214"  //please change to a unique value that matches BLE_IMU_PERIPHERAL
#define BLE_UUID_CHARACT_LED              "19B10005-E8F2-537E-4F6C-E104768A1214"  //please change to a unique value that matches BLE_IMU_PERIPHERAL
#define BLE_UUID_CHARACT_ACCX             "29B10004-E8F2-537E-4F6C-a204768A1215"  //please change to a unique value that matches BLE_IMU_PERIPHERAL
#define BLE_UUID_CHARACT_ACCY             "39B10004-E8F2-537E-4F6C-a204768A1215"  //please change to a unique value that matches BLE_IMU_PERIPHERAL
#define BLE_UUID_CHARACT_ACCZ             "49B10004-E8F2-537E-4F6C-a204768A1215"  //please change to a unique value that matches BLE_IMU_PERIPHERAL

#define BLE_UUID_CHARACT_GYROX             "59B10004-E8F2-537E-4F6C-a204768A1215"  //please change to a unique value that matches BLE_IMU_PERIPHERAL
#define BLE_UUID_CHARACT_GYROY             "69B10004-E8F2-537E-4F6C-a204768A1215"  //please change to a unique value that matches BLE_IMU_PERIPHERAL
#define BLE_UUID_CHARACT_GYROZ             "79B10004-E8F2-537E-4F6C-a204768A1215"  //please change to a unique value that matches BLE_IMU_PERIPHERAL

///------BUZZER
int speakerPin = 4;

int length = 14; // the number of notes
char notes[] = "ccggaagffeeddc"; // a space represents a rest
int beats[] = { 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2 };
int tempo = 100;
int i = constrain(i,0,length);

///-------IMU
  int buttonState = 0;
  float xAcc, yAcc, zAcc;
  float xGyro, yGyro, zGyro;
  float roll,pitch=0;
  float elapsedTime, currentTime, previousTime =0;

// ------------------------------------------ VOID SETUP ------------------------------------------
void setup() {
  pinMode(speakerPin, OUTPUT);
  Serial.begin(9600);
  while (!Serial);

  // configure the button pin as input
  pinMode(LED_BUILTIN, OUTPUT);

  // initialize the BLE hardware
  BLE.begin();

  Serial.println("BLE Central - Gyroscope control");

  // start scanning for peripherals
  BLE.scanForUuid(BLE_UUID_PERIPHERAL);//
}

// ------------------------------------------ VOID LOOP ------------------------------------------
void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    if (peripheral.localName() != "BLE_IMU") {
      return;
    }

    // stop scanning
    BLE.stopScan();

    LED_IMU(peripheral);

    // peripheral disconnected, start scanning again
    BLE.scanForUuid(BLE_UUID_PERIPHERAL);
  }
  
}

// ------------------------------------------ FUNCTIONS ------------------------------------------
void LED_IMU(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  // retrieve the LED characteristic
  BLECharacteristic ledCharacteristic = peripheral.characteristic(BLE_UUID_CHARACT_LED);
  BLECharacteristic accXCharacteristic = peripheral.characteristic(BLE_UUID_CHARACT_ACCX);
  BLECharacteristic accYCharacteristic = peripheral.characteristic(BLE_UUID_CHARACT_ACCY);
  BLECharacteristic accZCharacteristic = peripheral.characteristic(BLE_UUID_CHARACT_ACCZ);

  BLECharacteristic gyroXCharacteristic = peripheral.characteristic(BLE_UUID_CHARACT_GYROX);
  BLECharacteristic gyroYCharacteristic = peripheral.characteristic(BLE_UUID_CHARACT_GYROY);
  BLECharacteristic gyroZCharacteristic = peripheral.characteristic(BLE_UUID_CHARACT_GYROZ);

  // check if an specific BLE characteristic exists
  if (!ledCharacteristic) {
    Serial.println("Peripheral does not have LED characteristic!");
    peripheral.disconnect();
    return;
  } else if (!ledCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable LED characteristic!");
    peripheral.disconnect();
    return;
  }
    
  while (peripheral.connected()) {
    // while the peripheral is connected
    // read the gyroscope values
    accXCharacteristic.readValue( &xAcc, 4 );
    accYCharacteristic.readValue( &yAcc, 4 );
    accZCharacteristic.readValue( &zAcc, 4 );

    gyroXCharacteristic.readValue( &xGyro, 4 );
    gyroYCharacteristic.readValue( &yGyro, 4 );
    gyroZCharacteristic.readValue( &zGyro, 4 );

  // Calculate Roll and Pitch (rotation around X-axis, rotation around Y-axis)
  roll = atan(yAcc / sqrt(pow(xAcc, 2) + pow(zAcc, 2))) * 180 / PI;
  pitch = atan(-1 * xAcc / sqrt(pow(yAcc, 2) + pow(zAcc, 2))) * 180 / PI;

/// -------------------- Time -----------------------------///
  previousTime = currentTime;        // Previous time is stored before the actual time read
  currentTime = millis();            // Current time actual time read
  elapsedTime = (currentTime - previousTime) / 1000; // Divide by 1000 to get seconds

/// ------------- Serial Monitor to Processing ------------///
    Serial.print(roll);
    Serial.print('\t');
    Serial.print(pitch);
    Serial.print('\t');
    
    Serial.print(xAcc); 
    Serial.print('\t');
    Serial.print(yAcc); 
    Serial.print('\t');         
    Serial.print(zAcc); 
    Serial.print('\t'); 

    Serial.print(xGyro); 
    Serial.print('\t');
    Serial.print(yGyro);
    Serial.print('\t');         
    Serial.print(zGyro);
    Serial.print('\t');  
           
    Serial.print(elapsedTime);
    Serial.println('\t'); 

    playMusic();

    // make the LED blink
    if (buttonState == 0)
    {buttonState = 1;}
    else if (buttonState == 1)
    {buttonState = 0;}
    
    digitalWrite(LED_BUILTIN, buttonState);
      if (buttonState == 0) {
        // write 0x01 to turn the LED on
        ledCharacteristic.writeValue((byte)0x01);
      } else {
        // write 0x00 to turn the LED off
        ledCharacteristic.writeValue((byte)0x00);
      }
  }
  
  Serial.println("Peripheral disconnected");
}

//------ BUZZER FUNCTIONS ------//
void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);
  }
}

void playNote(char note, int duration) {
  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };

  // play the tone corresponding to the note name
  for (int i = 0; i < 8; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}

void playMusic(){
    // touched condition
    if (xGyro!=10000 && yGyro!=10000 && zGyro!=10000){
        // play tune
        if (notes[i] == ' ') {
            delay(beats[i] * tempo); // rest
          } else {
            playNote(notes[i], beats[i] * tempo);
          }

        // increment
        if (i<length){
        i++;
        } else if (i=length){
          i=0; // reset counter
        }
    }
}
