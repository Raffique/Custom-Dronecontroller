
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>  //To create PWM signals we need this lybrary

const uint64_t pipeIn = 0xE8E8F0F0E1LL;     //Remember that this code is the same as in the transmitter
RF24 radio(9, 10);  //CSN and CE pins

// The sizeof this struct should not exceed 32 bytes
struct Received_data {
byte up = 1; //up
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
  int LX = 127; //LX
  int LY = 127; //LY
  
  byte R3B; //R3
  int RX =127; //RX
  int RY = 0; //RY
  
  byte select; //SELECT
  byte start; //START
  byte main; //HOME
};

Received_data received_data;


boolean income = false;

#define roll 2
#define pitch 3
#define throttle 4
#define yaw 7
#define flight 8
#define aux6 11
#define aux7 12
#define aux8 13
/**************************************************/

void setup()
{
  
  pinMode(roll, OUTPUT); // Channel 1 Roll/Aileron
  pinMode(pitch, OUTPUT); // channel 2 Pitch/Elevator
  pinMode(throttle, OUTPUT); // channel 3 Throttle
  pinMode(yaw, OUTPUT); // channel 4 Yaw/Rudder
  
  pinMode(flight, OUTPUT); // channel 5 Flight modes
  pinMode(aux6, OUTPUT); // channel 6 
  pinMode(aux7, OUTPUT); // channel 7
  pinMode(aux8, OUTPUT); // channel 8
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

  if(millis()- lastRecTime > 1000){
    
    }
  
  if(income == true){
     analogWrite(roll, received_data.LX);
     analogWrite(pitch, received_data.LY);
     analogWrite(throttle, received_data.RY);
     analogWrite(yaw, received_data.RX);
     analogWrite(flight, received_data.up);
     analogWrite(aux6, received_data.right);
     analogWrite(aux7, received_data.down);
     analogWrite(aux8, received_data.left);
      }
      
    reset_data();
    lastRecTime = millis();
    income = false;
  }
  
  
  
}//Loop end

void reset_data() 
{
  // 'safe' values to use when NO radio input is detected
  //received_data.up = 0;      
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
  received_data.RX = 127;
  //received_data.RY = 0;
  
  received_data.L3B = 0;
  received_data.LX = 127;
  received_data.LY = 127;
  
  received_data.select = 0;
  received_data.start = 0;
  received_data.main = 0;
}




