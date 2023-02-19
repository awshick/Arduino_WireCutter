#include <Nextion.h>    //used for MCU through HMI Editor only -- must remove or comment out when using actual screen
#define nextion Serial  //used for MCU through HMI Editor only -- must remove or comment out when using actual screen


//-------------Static Variables-----------------------------------------------------------------
  //pin assigments
const int motorDC = 10;
const int sensorA = 8;
const int  sensorB = 9;
int valA = 0;
int valB = 0;

int valAA = 0;                //I added these for the nextion updating of the limit switch buttons while waiting for OPEN/CLOSE command.
int valBB = 0;                //They are used for manually testing the limit switch operations. Used in the void loop() function.

String dfd = "";               //Serial data container, data from display

void(* resetFunc) (void) = 0;  // declare reset fuction at address 0 , this is used to reset the arduino. I use this after every function to clear memory.
 
//-----------------------------------------------------------------------------------------------

void setup() {
 Serial.begin(9600);    //used to monitor - arduino recieve
 pinMode(motorDC, OUTPUT);
  digitalWrite(motorDC, LOW);     //set motorDC to LOW so motor is not running
 pinMode(sensorA, INPUT_PULLUP);  //default value is HIGH(1)
 pinMode(sensorB, INPUT_PULLUP);  //default value is HIGH(1)
 delay(500);

 //Nextion part - only needed if updating nextion screen
 //set sensorA dual botton state. If valA = 0 then bt0.val on nextion will be set to 0. This state on nextion will be displayed as OFF image, if 1 will be ON image
 Serial.print("bt0.val=");
 Serial.print(digitalRead(valA));
 Serial.write(0xff);               //this line has to be sent 3 times for nextion screen to accept it.
 Serial.write(0xff);
 Serial.write(0xff);

 //set sensorB dual botton state. If valB = 0 then bt1.val on nextion will be set to 0. This state on nextion will be displayed as OFF image, if 1 will be ON image
 Serial.print("bt1.val=");
 Serial.print(digitalRead(valB));
 Serial.write(0xff);
 Serial.write(0xff);
 Serial.write(0xff);
 
}


void loop() {       
 if(Serial.available()) {
    dfd += char(Serial.read());   //read data on serial
   }
 //check for test command(s)
  if((dfd.substring(0,5)=="OPENTST") & (dfd.length()==7)) {              //Recieved command 'OPENTST' from nextion
   openCutter();                                                         //Run openCutter function
   while(Serial.available()){Serial.read();}                             //clear serial buffer of anything sent after loop completed.
   delay(100);
   dfd = ""; 
  }
  else if((dfd.substring(0,5)=="CLOSETST") & (dfd.length()==8)) {        //Recieved command 'CLOSETST' from nextion
   closeCutter();                                                        //Run closeCutter function
   while(Serial.available()){Serial.read();}                             //clear serial buffer of anything sent after loop completed.
   delay(100);
   dfd = ""; 
  }

//Adding this part to update nextion screen if testing button(s). This way if you manually trigger your limit switch the screen will update the button image as ON or OFF. 
valA = digitalRead(sensorA);
if (valA != valAA) {               //valA has changed from what it was before so update the nextion screen for sensorA (bt0) image
 Serial.print("bt0.val=");
 Serial.print(valA);
 Serial.write(0xff);               //this line has to be sent 3 times for nextion screen to accept it.
 Serial.write(0xff);
 Serial.write(0xff);
 valAA = valA;                     //set valAA to be current value of valA (0 or 1) to compare next loop cycle.
 }

valA = digitalRead(sensorB);
if (valB != valBB) {               //valB has changed from what it was before so update the nextion screen for sensorB (bt1) image
 Serial.print("bt0.val=");
 Serial.print(valB);
 Serial.write(0xff);               //this line has to be sent 3 times for nextion screen to accept it.
 Serial.write(0xff);
 Serial.write(0xff);
 valBB = valB;                     //set valBB to be current value of valB (0 or 1) to compare next loop cycle.
 }
delay(300);  //delay for loop cycle 

}



//------------------------------------------------------------------------------------

void openCutter() {
 for(int x = 0; x < 5000; x++) {      //putting in stupid high number for steps, as we will utilize switch for stop  
    valA = digitalRead(sensorA);      //get sensorA state LOW/HIGH
    if(valA == 1) {                   //switch has not been hit so run
     digitalWrite(motorDC, HIGH);     //motorDC pin set HIGH to start motor - or keep it running
    }else {                           //switch has been hit stop running
      digitalWrite(motorDC, LOW);     //motorDC pin set LOW to stop motor
//      break;                          //exit loop
      resetFunc();                    //used as an alternate to break, this is a quick arduino reset.
      }
    }
 delay(800);
}

void closeCutter() {
 while(valB != 0) {                   //Do this until sensorB reads LOW (0)
  valB = digitalRead(sensorB);        //get sensroB state LOW/HIGH
  digitalWrite(motorDC, HIGH);        //motorDC pin set HIGH to start motor - or keep it running
 }
                                      //we are out of the while statement, so sensorB should be reading LOW
 digitalWrite(motorDC, LOW);          //motorDC pin set LOW to stop motor
 delay(800);
 resetFunc();                         //used as an alternate to break, this is a quick arduino reset.
}


  
