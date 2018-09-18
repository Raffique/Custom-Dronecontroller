/* can altered code to continuously output button state, instead of showing only when data recieved
   PA level can be adjusted witht the RF_PA_LEVEL command
 
    Module // Arduino MEGA
    
    GND    ->   GND
    Vcc    ->   3.3V
    CE     ->   D9
    CSN    ->   D10
    CLK    ->   D52
    MOSI   ->   D51
    MISO   ->   D52

*/


#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>  //To create PWM signals we need this lybrary

const uint64_t pipeIn = 0xE8E8F0F0E1LL;     //Remember that this code is the same as in the transmitter
RF24 radio(9, 10);  //CSN and CE pins

// The sizeof this struct should not exceed 32 bytes
struct Received_data {
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
  int LX; //LX
  int LY; //LY
  
  byte R3B; //R3
  int RX; //RX
  int RY; //RY
  
  byte select; //SELECT
  byte start; //START
  byte main; //HOME
};

Received_data received_data;


boolean income = false;
/**************************************************/

void setup()
{
  Serial.begin(115200);
  
  //Attach the servo signal on pins from D2 to D8
  
  //We reset the received values
  reset_data();

  //Once again, begin and radio configuration
  radio.begin();
  //radio.setPALevel(RF24_PA_LOW);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);  
  radio.openReadingPipe(1,pipeIn);
  
  //We start the radio comunication
  radio.startListening();

}

/**************************************************/

unsigned long lastRecvTime = 0;

//We create the function that will read the data each certain time
void receive_data()
{
  while ( radio.available() ) {
  radio.read(&received_data, sizeof(Received_data));
  lastRecvTime = millis(); //Here we receive the data
  income = true;
}
}

/**************************************************/

void loop()
{
  //Receive the radio data
  receive_data();

//////////This small if will reset the data if signal is lost for 1 sec.
/////////uncomment the reset_data() when using continuous sending and recieving
////////tip: can also right code to some form of action if signal lost eg return home function
/////////////////////////////////////////////////////////////////////////
 
  unsigned long now = millis();
  if ( now - lastRecvTime > 1000 ) {
    // signal lost?
    //reset_data();
    //Go up and change the initial values if you want depending on
    //your aplications. Put 0 for throttle in case of drones so it won't
    //fly away
  } 
  
  
  if(income == true){
    Serial.print("UP: "); Serial.println(received_data.up);
    Serial.print("DOWN: "); Serial.println(received_data.down);
    Serial.print("LEFT: "); Serial.println(received_data.left);
    Serial.print("RIGHT: "); Serial.println(received_data.right);
    Serial.print("A: "); Serial.println(received_data.AB);
    Serial.print("B: "); Serial.println(received_data.BB);
    Serial.print("Y: "); Serial.println(received_data.YB);
    Serial.print("X: "); Serial.println(received_data.XB);
    Serial.print("L1: "); Serial.println(received_data.L1B);
    Serial.print("L2: "); Serial.println(received_data.L2B);
    Serial.print("L3: "); Serial.println(received_data.L3B);
    Serial.print("LX: "); Serial.println(received_data.LX);
    Serial.print("LY: "); Serial.println(received_data.LY);
    Serial.print("R1: "); Serial.println(received_data.R1B);
    Serial.print("R2: "); Serial.println(received_data.R2B);
    Serial.print("R3: "); Serial.println(received_data.R3B);
    Serial.print("RX: "); Serial.println(received_data.RX);
    Serial.print("RY: "); Serial.println(received_data.RY);
    Serial.print("SELECT: "); Serial.println(received_data.select);
    Serial.print("START: "); Serial.println(received_data.start);
    Serial.print("HOME: "); Serial.println(received_data.main);
    income = false;
    reset_data();
  }
  
  
}//Loop end

void reset_data() 
{
  // 'safe' values to use when NO radio input is detected
  received_data.up = 0;      
  received_data.down = 0;
  received_data.right = 0;
  received_data.left = 0;
  
  received_data.AB = 0;
  received_data.BB = 0;
  received_data.YB = 0;
  received_data.XB = 0;
  
  received_data.L1B = 0;
  received_data.L2B = 0;
  received_data.R1B = 0;
  received_data.R2B = 0;
  
  received_data.R3B = 0;
  received_data.RX = 0;
  received_data.RY = 0;
  
  received_data.L3B = 0;
  received_data.LX = 0;
  received_data.LY = 0;
  
  received_data.select = 0;
  received_data.start = 0;
  received_data.main = 0;
}

