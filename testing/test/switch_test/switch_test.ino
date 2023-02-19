#include <Nextion.h>    //used for MCU through HMI Editor only -- must remove or comment out when using actual screen
#define nextion Serial  //used for MCU through HMI Editor only -- must remove or comment out when using actual screen


//-------------Static Variables-----------------------------------------------------------------
  //pin assigments
const int motorDC = 10;
int sensorA = 8;
int  sensorB = 9;
int valA = 1;
int valB = 1;

int valAA = 1;                //I added these for the nextion updating of the limit switch buttons while waiting for OPEN/CLOSE command.
int valBB = 1;                //They are used for manually testing the limit switch operations. Used in the void loop() function.

String dfd = "";               //Serial data container, data from display

//-----------------------------------------------------------------------------------------------

void setup() {
 Serial.begin(9600);
 Serial1.begin(9600);    //used to monitor - arduino recieve
 pinMode(motorDC, OUTPUT);
  digitalWrite(motorDC, LOW);     //set motorDC to LOW so motor is not running
 pinMode(sensorA, INPUT_PULLUP);  //default value is HIGH(1)
 pinMode(sensorB, INPUT_PULLUP);  //default value is HIGH(1)
 delay(500);

 //Nextion part - only needed if updating nextion screen
 //set sensorA dual botton state. If valA = 0 then bt0.val on nextion will be set to 0. This state on nextion will be displayed as OFF image, if 1 will be ON image
 Serial1.print("bt0.val=");
 Serial1.print(digitalRead(valA));
 Serial1.write(0xff);               //this line has to be sent 3 times for nextion screen to accept it.
 Serial1.write(0xff);
 Serial1.write(0xff);

 //set sensorB dual botton state. If valB = 0 then bt1.val on nextion will be set to 0. This state on nextion will be displayed as OFF image, if 1 will be ON image
 Serial1.print("bt1.val=");
 Serial1.print(digitalRead(valB));
 Serial1.write(0xff);
 Serial1.write(0xff);
 Serial1.write(0xff);

 Serial.print("Running");
}


void loop() {       
 if(Serial1.available()) {
    dfd += char(Serial1.read());   //read data on serial
   }
 //check for test command(s)
  if((dfd.substring(0,4)=="OPEN") & (dfd.length()==4)) {              //Recieved command 'OPEN' from nextion
   Serial.println("Going to open cutter");
   openCutter();                                                         //Run openCutter function
   while(Serial1.available()){Serial1.read();}                             //clear serial buffer of anything sent after loop completed.
   delay(100);
   dfd = ""; 
  }
  else if((dfd.substring(0,5)=="CLOSE") & (dfd.length()==5)) {        //Recieved command 'CLOSE' from nextion
    Serial.println("Going to close cutter");
   closeCutter();                                                        //Run closeCutter function
   while(Serial1.available()){Serial1.read();}                             //clear serial buffer of anything sent after loop completed.
   delay(100);
   dfd = ""; 
  }

//Adding this part to update nextion screen if testing button(s). This way if you manually trigger your limit switch the screen will update the button image as ON or OFF. 
valA = digitalRead(sensorA);
if (valA != valAA) {               //valA has changed from what it was before so update the nextion screen for sensorA (bt0) image
 Serial1.print("bt0.val=");
 Serial1.print(valA);
 Serial1.write(0xff);               //this line has to be sent 3 times for nextion screen to accept it.
 Serial1.write(0xff);
 Serial1.write(0xff);
 valAA = valA;                     //set valAA to be current value of valA (0 or 1) to compare next loop cycle.
 }

valB = digitalRead(sensorB);
if (valB != valBB) {               //valB has changed from what it was before so update the nextion screen for sensorB (bt1) image
 Serial1.print("bt1.val=");
 Serial1.print(valB);
 Serial1.write(0xff);               //this line has to be sent 3 times for nextion screen to accept it.
 Serial1.write(0xff);
 Serial1.write(0xff);
 valBB = valB;                     //set valBB to be current value of valB (0 or 1) to compare next loop cycle.
 }
delay(300);  //delay for loop cycle 

}



//------------------------------------------------------------------------------------

void openCutter() {
  Serial.println("Open pressed");
 for(int x = 0; x < 5000; x++) {      //putting in stupid high number for steps, as we will utilize switch for stop  
 //   Serial.println(x);
    valA = digitalRead(sensorA);      //get sensorA state LOW/HIGH

  if(valA == 1) {                   //switch has not been hit so run
   digitalWrite(motorDC, HIGH);     //motorDC pin set HIGH to start motor - or keep it running
  }else {                           //switch has been hit stop running
   digitalWrite(motorDC, LOW);     //motorDC pin set LOW to stop motor
   if (valA != valAA) {               //valA has changed from what it was before so update the nextion screen for sensorA (bt0) image
    Serial1.print("bt0.val=");
    Serial1.print(valA);
    Serial1.write(0xff);               //this line has to be sent 3 times for nextion screen to accept it.
    Serial1.write(0xff);
    Serial1.write(0xff);
    valAA = valA;                     //set valAA to be current value of valA (0 or 1) to compare next loop cycle.
   }
   Serial.println("Open Done");
   break;
  }
 delay(200);                       
 }
  digitalWrite(motorDC, LOW);
  while(Serial1.available()){Serial1.read();}  
}

void closeCutter() {
  Serial.println("Close pressed");
 while(valB != 0) {                   //Do this until sensorB reads LOW (0)
  valB = digitalRead(sensorB);        //get sensroB state LOW/HIGH
  digitalWrite(motorDC, HIGH);        //motorDC pin set HIGH to start motor - or keep it running

  if (valB != valBB) {                //valA has changed from what it was before so update the nextion screen for sensorA (bt0) image
   digitalWrite(motorDC, LOW);        //motorDC pin set LOW to stop motor 
   Serial1.print("bt1.val=");
   Serial1.print(valB);
   Serial1.write(0xff);               //this line has to be sent 3 times for nextion screen to accept it.
   Serial1.write(0xff);
   Serial1.write(0xff);
   valBB = valB;                     //set valBB to be current value of valB (0 or 1) to compare next loop cycle.
  }
 
 }
 delay(200);
 Serial.println("Close done");
 while(Serial1.available()){Serial1.read();}
}


  
