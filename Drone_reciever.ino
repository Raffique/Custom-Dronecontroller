#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

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
  byte LX = 127; //LX
  byte LY = 127; //LY
  
  byte R3B; //R3
  byte RX =127; //RX
  byte RY = 0; //RY
  
  byte select; //SELECT
  byte start; //START
  byte main; //HOME
};

Received_data received_data;


boolean income = false;
unsigned long lastRecTime = 0;



//////////////////////CONFIGURATION///////////////////////////////
#define CHANNEL_NUMBER 12  //set the number of chanels
#define CHANNEL_DEFAULT_VALUE 1500  //set the default servo value
#define FRAME_LENGTH 22500  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PULSE_LENGTH 300  //set the pulse length
#define onState 1  //set polarity of the pulses: 1 is positive, 0 is negative
#define sigPin 7  //set PPM signal output pin on the arduino

/*this array holds the servo values for the ppm signal
 change theese values in your code (usually servo values move between 1000 and 2000)*/
int ppm[CHANNEL_NUMBER];

////////////////////////////SUB FUNCTIONS/////////////////////////////////////////////////
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

/*******************************************************************/

//We create the function that will read the data each certain time
void receive_data()
{
  while ( radio.available() ) {
  radio.read(&received_data, sizeof(Received_data));
  lastRecTime = millis(); //Here we receive the data
  income = true;
}
}
/**********************************************************************/
int flightmodes(){
  int modeVal;

  if(received_data.up == 1){modeVal = 1200;}
  else if(received_data.up == 2){modeVal = 1300;}
  else if(received_data.up == 3){modeVal = 1400;}
  else if(received_data.up == 4){modeVal = 1600;}
  else if(received_data.up == 5){modeVal = 1700;} 
  else {modeVal = 1800;}
  return modeVal;
  
}

///////////////////////////////////MAIN SETUP AND LOOP/////////////////////////////
void setup(){  
  Serial.begin(115200);
  //initiallize default ppm values
  for(int i=0; i<CHANNEL_NUMBER; i++){
      ppm[i]= CHANNEL_DEFAULT_VALUE;
  }

  pinMode(sigPin, OUTPUT);
  digitalWrite(sigPin, !onState);  //set the PPM signal pin to the default state (off)
  
  cli();
  TCCR1A = 0; // set entire TCCR1 register to 0
  TCCR1B = 0;
  
  OCR1A = 100;  // compare match register, change this
  TCCR1B |= (1 << WGM12);  // turn on CTC mode
  TCCR1B |= (1 << CS11);  // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();

  ///////////////////nrf2401 SETTINGS//////////////////////
   //We reset the received values
  reset_data();
  received_data.up = 1;
  
  //Once again, begin and radio configuration
  radio.begin();
  //radio.setPALevel(RF24_PA_LOW);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);  
  radio.openReadingPipe(1,pipeIn);
  
  //We start the radio comunication
  radio.startListening();

}

void loop(){
  
  //Receive the radio data
  receive_data();

  if(millis()- lastRecTime > 1000){
    
    }
  
  if(income == true){
    ppm[0] = map(received_data.LX,0,255,1000,2000); Serial.print("roll: ");Serial.println(ppm[0]);
    ppm[1] = map(received_data.LY,0,255,1000,2000); Serial.print("pitch: ");Serial.println(ppm[1]);
    ppm[2] = map(received_data.RY,0,255,1000,2000); Serial.print("throttle: ");Serial.println(ppm[2]);
    ppm[3] = map(received_data.RX,0,255,1000,2000); Serial.print("yaw: ");Serial.println(ppm[3]);
    ppm[4] = flightmodes();Serial.println(ppm[4]);
    ppm[5] = map(received_data.right,0,1,1000,2000); Serial.print("aux6: ");Serial.println(ppm[5]);
    ppm[6] = map(received_data.left,0,1,1000,2000); Serial.print("aux7: ");Serial.println(ppm[6]);
    ppm[7] = map(received_data.down,0,1,1000,2000); Serial.print("aux8: ");Serial.println(ppm[7]);
      }
      
    reset_data();
    lastRecTime = millis();
    income = false;
  
}

/////////////////////////////////TIMER EVENTS///////////////////////////////////////
ISR(TIMER1_COMPA_vect){  //leave this alone
  static boolean state = true;
  
  TCNT1 = 0;
  
  if (state) {  //start pulse
    digitalWrite(sigPin, onState);
    OCR1A = PULSE_LENGTH * 2;
    state = false;
  } else{  //end pulse and calculate when to start the next pulse
    static byte cur_chan_numb;
    static unsigned int calc_rest;
  
    digitalWrite(sigPin, !onState);
    state = true;

    if(cur_chan_numb >= CHANNEL_NUMBER){
      cur_chan_numb = 0;
      calc_rest = calc_rest + PULSE_LENGTH;// 
      OCR1A = (FRAME_LENGTH - calc_rest) * 2;
      calc_rest = 0;
    }
    else{
      OCR1A = (ppm[cur_chan_numb] - PULSE_LENGTH) * 2;
      calc_rest = calc_rest + ppm[cur_chan_numb];
      cur_chan_numb++;
    }     
  }
}
