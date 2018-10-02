/*
   Can comment out all the serial.print commands, they are there just for testing resutls 
   PA level can be increaed by uncommenting the command setPALevel command and chnagong the input from RF_PA_LOW to RF_PA_HIGH
   code can be altered from sending only when but is pressed to continuously send button state
*/

#include <XBOXUSB.h>
// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
USB Usb;
XBOXUSB Xbox(&Usb);

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
const uint64_t my_radio_pipe = 0xE8E8F0F0E1LL; //Remember that this code should be the same for the receiver
#define CE 9
#define CSN 53
RF24 radio(CE, CSN);  //Set CE and CSN pins

// The sizeof this struct should not exceed 32 bytes
struct Data_to_be_sent {
  byte up; //up
  byte right; //right
  byte down; //down
  byte left; //left
  
  byte YB; //Y
  byte BB; //B
  byte AB; //A
  byte XB; //X
  
  byte L1B; //L1
  byte L2B; //L2
  byte R1B; //R1
  byte R2B; //R2
  
  byte L3B; //L3
  byte LX; //LX
  byte LY; //LY
  
  byte R3B; //R3
  byte RX; //RX
  byte RY; //RY
  
  byte select; //SELECT
  byte start; //START
  byte main; //HOME
};

//Create a variable with the structure above and name it sent_data
Data_to_be_sent sent_data;

boolean changes = false;
int threshold = 100;
unsigned long lastTime = millis();
int StickPos = 0;
unsigned long timing = 0;

void setup()
{ 
  Serial.begin(115200);

  #if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
  #endif
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); //halt
  }
  Serial.println(F("\r\nXBOX USB Library Started"));

  radio.begin();
  //radio.setPALevel(RF24_PA_LOW);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(my_radio_pipe);

  reset_data();
  
}

/**************************************************/


void loop()
{

  Usb.Task();
  if (Xbox.Xbox360Connected) {
    if (Xbox.getButtonPress(L2) || Xbox.getButtonPress(R2)) {
      Serial.print("L2: ");
      Serial.print(Xbox.getButtonPress(L2));
      Serial.print("\tR2: ");
      Serial.println(Xbox.getButtonPress(R2));
      sent_data.L2B = Xbox.getButtonPress(L2);
      sent_data.R2B = Xbox.getButtonPress(R2);
      changes = true;
      //Xbox.setRumbleOn(Xbox.getButtonPress(L2), Xbox.getButtonPress(R2));
    } //else
      //Xbox.setRumbleOn(0, 0);

    if (Xbox.getAnalogHat(LeftHatX) > threshold || Xbox.getAnalogHat(LeftHatX) < -threshold || Xbox.getAnalogHat(LeftHatY) > threshold || Xbox.getAnalogHat(LeftHatY) < -threshold || Xbox.getAnalogHat(RightHatX) > threshold || Xbox.getAnalogHat(RightHatX) < -threshold || Xbox.getAnalogHat(RightHatY) > threshold || Xbox.getAnalogHat(RightHatY) < -threshold) {
      if (Xbox.getAnalogHat(LeftHatX) > threshold || Xbox.getAnalogHat(LeftHatX) < -threshold) {
        
        sent_data.LX = map(Xbox.getAnalogHat(LeftHatX), -32768, 32767, 0, 255);
        changes = true;
        Serial.print("LX: ");Serial.println(sent_data.LX);
      }
      if (Xbox.getAnalogHat(LeftHatY) > threshold || Xbox.getAnalogHat(LeftHatY) < -threshold) {
        
        sent_data.LY = map(Xbox.getAnalogHat(LeftHatY), -32768, 32767, 0, 255);
        changes = true;
        Serial.print("LY: ");Serial.println(sent_data.LY);
      }
      if (Xbox.getAnalogHat(RightHatX) > threshold || Xbox.getAnalogHat(RightHatX) < -threshold) {
        
        sent_data.RX = map(Xbox.getAnalogHat(RightHatX), -32768, 32767, 0, 255);
        changes = true;
         Serial.print("RX: ");Serial.println(sent_data.RX);
      }
      if (Xbox.getAnalogHat(RightHatY) > threshold || Xbox.getAnalogHat(RightHatY) < -threshold) {
        
        //sent_data.RY = map(Xbox.getAnalogHat(RightHatY), -32768, 32767, 0, 255);
        int data = Xbox.getAnalogHat(RightHatY);
        if(data >= 0){ 
            timing = map(data, 0, 32767, 61, 30);
        }
        else{
            timing = map(data, -1, -32768, 61, 30);
        }
        if(millis() - lastTime > timing){ 
      
         if(data >= 0){
            StickPos += 1;
         }
         else{
            StickPos -= 1;
         }
          
        if (StickPos > 255){
          StickPos = 255;
        }
        if (StickPos < 0){
          StickPos = 0;
        }
    
        lastTime = millis();
        sent_data.RY = StickPos;
        changes = true;
        Serial.print("RY: ");Serial.println(sent_data.RY);
        }
      }
    }

    if (Xbox.getButtonClick(UP)) {
      Xbox.setLedOn(LED1);
      Serial.println(F("Up"));
      sent_data.up++;
      if (sent_data.up > 6){
          sent_data.up = 1;
        }
        Serial.println(sent_data.up);
      changes = true;
    }
    if (Xbox.getButtonClick(DOWN)) {
      Xbox.setLedOn(LED4);
      Serial.println(F("Down"));
      sent_data.down = 1;
      changes = true;
    }
    if (Xbox.getButtonClick(LEFT)) {
      Xbox.setLedOn(LED3);
      Serial.println(F("Left"));
      sent_data.left = 1;
      changes = true;
    }
    if (Xbox.getButtonClick(RIGHT)){
      Xbox.setLedOn(LED2);
      Serial.println(F("Right"));
      sent_data.right = 1;
      changes = true;
    }

    if (Xbox.getButtonClick(START)){
      Xbox.setLedMode(ALTERNATING);
      Serial.println(F("Start"));
      sent_data.start = 1; 
      changes = true;
    }
    if (Xbox.getButtonClick(BACK)){
      Xbox.setLedBlink(ALL);
      Serial.println(F("Back"));
      sent_data.select = 1;
      changes = true;
    }
    if (Xbox.getButtonClick(L3)){
      Serial.println(F("L3"));
      sent_data.L3B = 1;
      changes = true;
    }
    if (Xbox.getButtonClick(R3)){
      Serial.println(F("R3"));
      sent_data.R3B = 1;
      changes = true;
    }
    if (Xbox.getButtonClick(L1)){
      Serial.println(F("L1"));
      sent_data.L1B = 1;
      changes = true;
    }
    if (Xbox.getButtonClick(R1)){
      Serial.println(F("R1"));
      sent_data.R1B = 1;
      changes = true;
    }
    if (Xbox.getButtonClick(XBOX)){
      Xbox.setLedMode(ROTATING);
      Serial.println(F("Xbox"));
      sent_data.main = 1;
      changes = true;
    }

    if (Xbox.getButtonClick(A)){
      Serial.println(F("A"));
      sent_data.AB = 1;
      changes = true;
    }
    if (Xbox.getButtonClick(B)){
      Serial.println(F("B"));
      sent_data.BB = 1;
      changes = true;
    }
    if (Xbox.getButtonClick(X)){
      Serial.println(F("X"));
      sent_data.XB = 1;
      changes = true;
    }
    if (Xbox.getButtonClick(Y)){
      Serial.println(F("Y"));
      sent_data.YB = 1;
      changes = true;
    }
  }
  //delay(1);
  
  if(changes == true){
  ///////////////////////////////////////////////////
  ///////   SENDING DATA WITH THIS COMMAND      ////
  radio.write(&sent_data, sizeof(Data_to_be_sent));
  changes = false; 
  reset_data();
  }
}



void reset_data(){
 //Reset each channel value
  //sent_data.up = 0; // used to control flight mode, keep value
  //sent_data.down = 0; // used to control flight mode, keep value
  sent_data.right = 0;
  sent_data.left = 0;
  
  sent_data.AB = 0;
  sent_data.BB = 0;
  sent_data.XB = 0;
  sent_data.YB = 0;
  
  sent_data.L1B = 0;
  sent_data.L2B = 0;
  sent_data.R1B = 0;
  sent_data.R2B = 0;
  
  sent_data.L3B = 0;
  sent_data.LX = 127; //neutral value for loaded spring joystick
  sent_data.LY = 127; //neutral value for loaded spring joystick
  
  sent_data.R3B = 0;
  sent_data.RX = 127; //neutral value for loaded spring joystick
  //sent_data.RY = 0; //this is used for throttle which should stay constant on the drone
  
  sent_data.select = 0;
  sent_data.start = 0;
  sent_data.main = 0;
}
  
