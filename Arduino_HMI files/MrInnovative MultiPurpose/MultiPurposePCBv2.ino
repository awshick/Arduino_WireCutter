 //#include <Nextion.h>    //used for MCU through HMI Editor only -- must remove or comment out when using actual screen
//#define nextion Serial  //used for MCU through HMI Editor only -- must remove or comment out when using actual screen

//-------------User Variables------------------------------------------------------------------------------------------------------------------------------------------------------------
//  These may change depending upon the setup: Stepper steps for rotation, Stepper mode, etc
//  My steppers are 200 full steps for a full rotation (360 degrees), so numbers below based on that.
//  Should be the only variables that would need to be changed in code.
int actualLength = 6.23;      //will be used in cutLoop for steps per mm      **I've calculated 623 steps for 100mm, so 623/100=6.23 which should be 1mm. Change this value as required.
//<-----Making the below values big for testing. 200 steps should be a full 360° spin----->
int stripSteps = 200;          //total steps needed for Cutter to make STRIP cut --will need to adjust depending upon wire AWG and your setup.
int cutOpenSteps = 400;        //Steps to open cutters from a close (cut) position
int cutCloseSteps = 400;       //Steps to close cutters from a open position

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//Pin assigments
const int stepEnable = 12;   //Stepper(s) Enable Pin
const int stepADir = A0;     //Stepper1 Direction Pin
const int stepAStep = A1;    //Stepper1 Step Pin
const int stepBDir = A2;     //Stepper2 Direction Pin
const int stepBStep = A3;    //Stepper2 Step Pin

//---Variables----------------------------------------------------------------------------------
String dfd = "";                //Serial data container, data from display

//CutLoop variables, sent from cut page
float preStripValue = 0;      //will be used to store precut(strip) for cutLoop
float lengthValue = 0;        //will be used to store cut length for cutLoop
float postStripValue = 0;     //will be used to store postcut (strip) for cutLoop
int qtyValue = 0;             //will be used to store qty for cutLoop
String uom = "";              //will be used as unit of measure (I=inch, M=mm)
String myStop = "";           //will be used in cutLoop to monitor for STOP being pressed
int vRun = 1;                 //will be used in cutLoop to interrupt if STOP pressed

//feedLoop variables, sent from feed page  - feed wire in/out    (vDirection, vLength)
String vDirection = "";
unsigned long vLength;    

//testLoop variables, sent from test page  - cycle steppers      (vMotor)
int vMotor;     //might be able to change to String 
 
//stepLoop variables, sent from steptst page - run stepper x pulses , x step mode
//will use vDirection (variable above in feedLoop) for step direction 
int vStep = 0;                //Steps to take
String sMotor = "";
int vMode = 0;    //wont be used

//-----------------------------------------------------------------------------------------------

void setup() {
 Serial.begin(9600);                  
 pinMode(stepEnable, OUTPUT);

 pinMode(stepAStep, OUTPUT);
 pinMode(stepADir, OUTPUT);
 pinMode(stepBStep, OUTPUT);
 pinMode(stepBDir, OUTPUT);
 delay(500);

 openCutter();                       //Open cutter. No way to know what position they are in so will open to atleast be consistent.
 while(Serial.available()){Serial.read();} 
 
}


void loop() {       
 if(Serial.available()) {
    dfd += char(Serial.read());   //read data on serial if recieved, store all in dfd variable
   }

 //check for 'cut' command. This will be sent from nextion status page 'START' button being pressed.
 //Nextion data sent will be 4 lines:
 //cut1M00010           <--- will contain the 1st strip length
 //cut2M00550           <--- will contain the wire length
 //cut3M00010           <--- will contain 2nd strip length
 //cut4M00008           <--- will contain qty
    if((dfd.substring(0,3)=="cut") & (dfd.length()==10)) {                                //if first 3 characters recieved are 'cut' and the total characters are 10 then do this:
     if(dfd.substring(3,4)=="1") preStripValue = float(dfd.substring(5).toInt()) / 10;    //if the 4th char is '1' then this is the 1st strip length line (cut1) set preStripValue to number(s) after 5th char and divide by 10.
     if(dfd.substring(3,4)=="2") lengthValue = float(dfd.substring(5).toInt()) / 10;      //if the 4th char is '2' then this is the wire length line (cut2) set lengthValue to number(s) after 5th char and divide by 10.
     if(dfd.substring(3,4)=="3") postStripValue = float(dfd.substring(5).toInt()) / 10;   //if the 4th char is '3' then this is the 2nd strip length line (cut3) set postStripValue to number(s) after 5th char and divide by 10.
     if(dfd.substring(3,4)=="4") {                                                        //if the 4th char is '4' then this is the qty line (cut4) set uom to the 4th character, and qtyValue to number(s) after 5th char.
      uom = dfd.substring(4,5);
      qtyValue = dfd.substring(5).toInt();
      delay(50);
      dfd="";
      cutLoop();                                                                          //with all variables needed collected, run cutLoop function.
      while(Serial.available()){Serial.read();}                                           //clear serial buffer of anything sent after loop completed.
      delay(100);
     }
    dfd=""; 
    }

 //Check for 'DIN' or 'DOUT' command. This will be sent from nextion feed page '1mm , 10mm, 100mm' button being pressed.
 //Nextion data sent will be 1 line:
 //DIN010               <-- will contain direction and length. DIN or DOUT for direction. 001 = 1mm , 010 = 10mm , 100 = 100mm.
   else if(((dfd.substring(0,3)=="DIN") or (dfd.substring(0,3)=="OUT")) & (dfd.length()==6)) {    //if first 3 characters recieved are 'DIN' or 'DOUT' and total characters are 6 then do this:
    vDirection = dfd.substring(0,3);                                                              //set vDirection variable to first 3 characters.
    vLength = dfd.substring(3).toInt();                                                           //set vLength variable to number(s) after the 3rd char. 
    delay(50);
    feedLoop();                                                                                   //with all variables needed collected, run feedLoop function.
    while(Serial.available()){Serial.read();}                                                     //clear serial buffer of anything sent after loop completed.
    delay(100);
    dfd="";
   }
 
 //Check for 'WPREP' command. This will be sent from nextion 'wirePrep' page.
 //Nextion data sent will be 1 line:
 //WPREP              <--- will be used as a simple trigger to run function.
 else if((dfd.substring(0,5)=="WPREP") & (dfd.length()==5)) {                                     //if first 5 characters recieved are 'WPREP' and total characters are 5 then do this:
  loadAssist();                                                                                   //no variables needed, run loadAssist function.
  while(Serial.available()){Serial.read();}                                                       //clear serial buffer of anything sent after loop completed.
  delay(100);
  dfd = ""; 
 }

 //check for 'MTEST' command. This will be sent from nextion 'test' page.
 //Nextion data sent will be 1 line:
 //MTEST1           <--- will contain MTEST1 or MTEST2 for stepper1 or stepper2.
  else if((dfd.substring(0,5)=="MTEST") & (dfd.length()==6)) {                                    //if first 5 characters recieved or 'MTEST' and total characters are 6 then do this:
   vMotor = dfd.substring(5).toInt();                                                             //set vMotor variable to number(s) after the 5th character. In this case it will be 1 or 2.
   delay(50);
   testLoop();                                                                                    //with all variables needed collected, run testLoop function.
   while(Serial.available()){Serial.read();}                                                      //clear serial buffer of anything sent after loop completed.
   delay(100);
   dfd = ""; 
  }

 //check for 'DIN' or 'OUT' at END of command. This will be sent from nextion 'steptst' page. (similar to feedLoop, but DIN would be at the end of the string, not beginning)
 //Nextion data sent will be 1 line:
 //4F0255OUT      <--- will contain step mode, steps, direction. Step mode not used here due to using MultiPurpose PCB (this is set with jumpers), but leaving so nextion code does not need modified.
  else if(((dfd.substring(6)=="DIN") or (dfd.substring(6)=="OUT")) & (dfd.length()==9)) {         //if characters after 6th char = 'DIN' or 'OUT' and total characters are 9 then do this:
   vMode = dfd.substring(0,1).toInt();                                                            //seting variable, but again will not be used in this code. 
   vStep = dfd.substring(2,6).toInt();                                                            //set vStep variable to number(s) after 2nd char to character 6. (4F0255OUT would set vStep to 0255).
   sMotor = dfd.substring(1,2);                                                                   //set sMotor variable to 1st character (F or C).
   vDirection = dfd.substring(6);                                                                 //set vDirection variable to characters after 6th char (DIN or OUT).
   delay(50); 
   steptst();                                                                                     //with all variables needed collected, run stetst function.
   while(Serial.available()){Serial.read();}                                                      //clear serial buffer of anything sent after loop completed.
   delay(100);
   dfd = "";
  }

  else if(dfd.length()>10) {
    while(Serial.available()){Serial.read();}   
  }

  
}



//FUNCTIONS CALLED FROM VOID LOOP BELOW:

//----------------------LOOP FOR CUTTING WIRE----------------------------------------- 
void cutLoop() {
  delay(500);                      //delay to let screen catch up
  Serial.print("vis go,0");        //change to status page
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  delay(300);
  Serial.print("t2.txt=\"Running..\"");       //change status
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  delay(300);

  while(Serial.available()){Serial.read();}   //clear serial buffer of any lagging junk from screen/variable change. Sometimes the 0xff write is caught in buffer and interrupts myStop check.

  openCutter();                              //Open cutter before starting. Suspect it will max out here and skip steps - but need a consistent start point in order to cut.
  delay(300);

  if(uom=="I") {                              //If uom variable is I then convert to mm for standard processing.
    preStripValue = preStripValue * 25.4;     //1 inch = 25.4mm , so multiply inches entered by 25.4
    lengthValue = lengthValue * 25.4;
    postStripValue = postStripValue * 25.4;
  }
  
  for(int a = 1; a < (qtyValue + 1); a++) {         //Will run the below loop for each qty entered.
   if(Serial.available() & (vRun==1)) {             //checking serial data again, in case STOP is sent. Not an instant stop but will stop after last loop completed if told to.
      myStop = char(Serial.read()); 
   }
    if((myStop=="S")){        //if STOP is rcvd then exit the loop
    Serial.print("t2.txt=\"STOP !\"");
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
    delay(1000);
    dfd="";
    myStop="";
    vRun=0;
    break;
     }
    else { 
    Serial.print("t1.txt=\"" + String(a) + " of " + String(qtyValue) + "\"");
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
    
   digitalWrite(stepEnable,LOW);                      //enable drivers, will do this in every loop as open/close function will disable on completion (called below).
//Setup and Run feed motor
 //----1st Strip Cut (if required)----
   if(preStripValue > 0){                             //only run this part if preStripValue was entered.
    //Code to run FEED motor for preCut Strip Cut
    int preCutL = preStripValue * actualLength;       //multiply input by actualLength(int variable set at start)
    digitalWrite(stepADir,HIGH);                      //set feed direction to CCW to feed wire
    for(int x = 0; x < preCutL; x++) {                //This is the stepper loop that will 'pulse' or 'step' the motor 
     digitalWrite(stepAStep,HIGH);
     delayMicroseconds(450);                          //this controls the speed of the turn. 500 is pretty smooth but 900 is slower. Can adjust all of these delays if desired.
     digitalWrite(stepAStep,LOW);
     delayMicroseconds(450);
   }
   delay(500);
   //Run cut motor for strip movement.
    digitalWrite(stepBDir,LOW);                   //Turn cutter motor clockwise, to close.
    for(int x = 0; x < stripSteps; x++) {      
     digitalWrite(stepBStep,HIGH);
     delayMicroseconds(450);         
     digitalWrite(stepBStep,LOW);
     delayMicroseconds(450);
    }
    digitalWrite(stepBDir,HIGH);                   //Turn cutter motor counter clockwise, to open back.
    for(int x = 0; x < stripSteps; x++) {      
     digitalWrite(stepBStep,HIGH);
     delayMicroseconds(450);         
     digitalWrite(stepBStep,LOW);
     delayMicroseconds(450);
    }
  }
 //----Done with 1st Strip Cut ---

    
 //Run FEED motor for actual Length 
    int cutL = lengthValue * actualLength;            //multiply input by actualLength(int variable set at start)
    for(int x = 0; x < cutL; x++) {              
     digitalWrite(stepAStep,HIGH);
     delayMicroseconds(450);                     
     digitalWrite(stepAStep,LOW);
     delayMicroseconds(450);
   }
   delay(500);

 //----2nd Strip Cut (if required)----
  if(postStripValue > 0){   //only run if postStripValue was entered   
    //Code to run FEED motor for preCut Strip Cut
    //Run cut motor for strip movement.
    digitalWrite(stepBDir,LOW);                   //Turn cutter motor clockwise, to close.
    for(int x = 0; x < stripSteps; x++) {      
     digitalWrite(stepBStep,HIGH);
     delayMicroseconds(450);         
     digitalWrite(stepBStep,LOW);
     delayMicroseconds(450);
    }
    digitalWrite(stepBDir,HIGH);                   //Turn cutter motor counter clockwise, to open back.
    for(int x = 0; x < stripSteps; x++) {      
     digitalWrite(stepBStep,HIGH);
     delayMicroseconds(450);         
     digitalWrite(stepBStep,LOW);
     delayMicroseconds(450);
    }
   //Feed out post strip length
    int postCutL = postStripValue * actualLength;     //multiply input by actualLength(int variable set at start)
    digitalWrite(stepADir,HIGH);                      //set feed direction to CCW to feed wire
    for(int x = 0; x < postCutL; x++) {               //This is the stepper loop that will 'pulse' or 'step' the motor 
     digitalWrite(stepAStep,HIGH);
     delayMicroseconds(450);                          //this controls the speed of the turn. 500 is pretty smooth but 900 is slower. Can adjust all of these delays if desired.
     digitalWrite(stepAStep,LOW);
     delayMicroseconds(450);
   }
   delay(500);

  }
 //----Done with 2nd Strip Cut ---

  
 //Run CUT motor for final Cut 
   closeCutter();
   delay(500);
   openCutter();
   delay(500);   //end of cut delay
   }
    myStop="";
  }
  
    digitalWrite(stepEnable,HIGH);   //disable drivers
    delay(300);
    Serial.print("cut.n3.val=0");       //reset quantity on cut page
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
    delay(1000);
    Serial.print("t1.txt=\"DONE!\"");
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
    delay(1000);
    Serial.print("page start");
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);      
    delay(500);
    Serial.print("page start");
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff); 
    dfd="";
    vRun=1;
 }
//------------------------------------------------------------------------------------



//----------------------LOOP FOR FEEDING WIRE-----------------------------------------
void feedLoop() {
  //This loop is to run the feed stepper using
  //vDirection = DIN or DOUT for clockwise or counterclockwise direction
  //vLength = 001/010/100 for mm to travel
  digitalWrite(stepEnable,LOW);    //enable drivers
  //run feed motor
  if(vDirection=="DIN"){
   digitalWrite(stepADir,HIGH);         //spin CCW
  }
  else{
    digitalWrite(stepADir,LOW);        //spin CW
  }
  if(vLength==001){
    for(int x = 0; x < (1 * actualLength); x++) {   
     digitalWrite(stepAStep,HIGH);
     delayMicroseconds(450);         
     digitalWrite(stepAStep,LOW);
     delayMicroseconds(450);
   }
  }
  if(vLength==10){
    for(int x = 0; x < (10 * actualLength); x++) {   
     digitalWrite(stepAStep,HIGH);
     delayMicroseconds(450);         
     digitalWrite(stepAStep,LOW);
     delayMicroseconds(450);
   }
 }
 if(vLength==100){
    for(int x = 0; x < (100 * actualLength); x++) {   
     digitalWrite(stepAStep,HIGH);
     delayMicroseconds(450);         
     digitalWrite(stepAStep,LOW);
     delayMicroseconds(450);
   }
 }
 delay(1000);
 digitalWrite(stepEnable,HIGH); //disable drivers
 dfd="";
}
//------------------------------------------------------------------------------------


//----------------------LOOP FOR LOAD ASSIST------------------------------------------
void loadAssist() {
  //This loop is to run the feed stepper using
  //vDirection = DIN or DOUT for clockwise or counterclockwise direction
  //vLength = 001/010/100 for mm to travel
  openCutter();
  delay(500);
  digitalWrite(stepEnable,LOW);                    //enable drivers
  //run feed motor
   digitalWrite(stepADir,HIGH);                     //spin CCW
    for(int x = 0; x < (80 * actualLength); x++) {    //feed ~80mm of wire
     digitalWrite(stepAStep,HIGH);
     delayMicroseconds(450);         
     digitalWrite(stepAStep,LOW);
     delayMicroseconds(450);
   }
 delay(1000);
 digitalWrite(stepEnable,HIGH); //disable drivers
 //cut excess wire that feed out
 closeCutter();
 delay(500);
 openCutter();
 delay(500);
 dfd="";
}
//------------------------------------------------------------------------------------



//----------------------LOOP FOR TESTING MOTORS---------------------------------------
void testLoop() {
 delay(300);     //delay to let screen catch up 

 if(vMotor==1){      //test FEED motor
  digitalWrite(stepEnable,LOW);                      //enable drivers
  //Code to run FEED motor Counter Clockwise
   digitalWrite(stepADir,HIGH);  
    for(int x = 0; x < (100 * actualLength); x++) {   
     digitalWrite(stepAStep,HIGH);
     delayMicroseconds(450);         
     digitalWrite(stepAStep,LOW);
     delayMicroseconds(450);
   }
   digitalWrite(stepADir,LOW);
   delay(1000);
   //Code to run FEED motor Clockwise
    for(int x = 0; x < (100 * actualLength); x++) {   
     digitalWrite(stepAStep,HIGH);
     delayMicroseconds(450);         
     digitalWrite(stepAStep,LOW);
     delayMicroseconds(450);
   }   
   delay(800);
   digitalWrite(stepEnable,HIGH); //disable drivers
 }

 if(vMotor==2){      //test CUT motor, run open and the close function
  openCutter();
  closeCutter();
 }
  dfd="";
}
//------------------------------------------------------------------------------------



//----------------------LOOP FOR STEP TEST OF MOTORS----------------------------------
void steptst() {
  //Serial.println("Mode:" + String(vMode) + " , Step:" + String(vStep) + " , Motor:" + String(sMotor) + " , Dir:" + String(vDirection));
  if(sMotor=="F") {                     //Test Feed Motor
   digitalWrite(stepEnable,LOW);     //enable drivers
   if(vDirection=="DIN") {              //Set direction
    digitalWrite(stepADir,LOW);
   }else {
    digitalWrite(stepADir,HIGH);
   }
   for(int x = 0; x < vStep; x++) {   
     digitalWrite(stepAStep,HIGH);
     delayMicroseconds(450);         
     digitalWrite(stepAStep,LOW);
     delayMicroseconds(450);
   }
   digitalWrite(stepEnable,HIGH); //disable drivers
   delay(800);
 }
 
 else if(sMotor=="C") {               //Test Cut Motor
   digitalWrite(stepEnable,LOW);   //enable drivers
   if(vDirection=="DIN") {            //Set direction
    digitalWrite(stepBDir,LOW);
   }else {
    digitalWrite(stepBDir,HIGH);
   }
    for(int x = 0; x < vStep; x++) {
        digitalWrite(stepBStep,HIGH);
        delayMicroseconds(450);         
        digitalWrite(stepBStep,LOW);
        delayMicroseconds(450);
    } 
 digitalWrite(stepEnable,HIGH); //disable drivers
 delay(500);
 }
}
//------------------------------------------------------------------------------------


void openCutter() {
 digitalWrite(stepEnable,LOW);              //enable drivers
 digitalWrite(stepBDir,HIGH);               //will be turning CCW
   for(int x = 0; x < cutOpenSteps; x++) {     
     digitalWrite(stepBStep,HIGH);
     delayMicroseconds(450);         
     digitalWrite(stepBStep,LOW);
     delayMicroseconds(450);
    }
 digitalWrite(stepEnable,HIGH);             //disable drivers
 delay(800);
}

//------------------------------------------------------------------------------------

void closeCutter() {
 digitalWrite(stepEnable,LOW);             //enable drivers
 digitalWrite(stepBDir,LOW);               //will be turning CW
   for(int x = 0; x < cutCloseSteps; x++) {      
     digitalWrite(stepBStep,HIGH);
     delayMicroseconds(450);         
     digitalWrite(stepBStep,LOW);
     delayMicroseconds(450);
   }
 digitalWrite(stepEnable,HIGH);            //disable drivers
 delay(800);
}


  
