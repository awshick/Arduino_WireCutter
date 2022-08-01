//#include <Nextion.h>    //used for MCU through HMI Editor only -- must remove or comment out when using actual screen
//#define nextion Serial  //used for MCU through HMI Editor only -- must remove or comment out when using actual screen

//-------------User Variables------------------------------------------------------------------------------------------------------------------------------------------------------------
//  These may change depending upon the setup: Stepper steps for rotation, Stepper mode, etc
//  My steppers are 200 full steps for a full rotation (360 degrees), so numbers below based on that.
//  Should be the only variables that would need to be changed in code.
float actualLength = 6.23;      //will be used in cutLoop for steps per mm      **I've calculated 623 steps for 100mm, so 623/100=6.23 which should be 1mm. Change this value as required.
int stripSteps = 375;          //total steps needed for Cutter to make STRIP cut --will adjust depending upon wire AWG but I plan to use 22 AWG(red).
int feedStepMode = 1;         //step mode for feed motor. 0=Full, 1=1/2, 2=1/4, 3=1/8, 4=1/16
                              //You may want to change this for speed/efficiency depending upon the setup.
int cutStepMode = 1;          //step mode for opening/closing cutter. 0=Full, 1=1/2, 2=1/4, 3=1/8, 4=1/16
                              //You may want to change this for speed/efficiency depending upon the setup.  
                               
//**9/6/21 changed 'stripSteps' and 'stepMode' to 1 from 3, to test 5:1 geared stepper for cutter.                           
                
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//-------------Static Variables-----------------------------------------------------------------
  //pin assigments
const int enablePinA = 2; //Stepper1 Feed Motor
const int ms1A = 3;       //Stepper1 Feed Motor
const int ms2A = 4;       //Stepper1 Feed Motor
const int ms3A = 5;       //Stepper1 Feed Motor
const int stepPinA = 6;   //Stepper1 Feed Motor
const int dirPinA = 7;    //Stepper1 Feed Motor

const int enablePinB = 8; //Stepper2 Cut Motor
const int ms1B = 9;       //Stepper2 Cut Motor
const int ms2B = 10;      //Stepper2 Cut Motor
const int ms3B = 11;      //Stepper2 Cut Motor
const int stepPinB = 12;  //Stepper2 Cut Motor
const int dirPinB = 13;   //Stepper2 Cut Motor

String dfd = "";          //Serial data container, data from display

  //CutLoop variables, sent from cut page
float preStripValue = 0;      //will be used to store precut(strip) for cutLoop
float lengthValue = 0;        //will be used to store cut length for cutLoop
float postStripValue = 0;     //will be used to store postcut (strip) for cutLoop
int qtyValue = 0;             //will be used to store qty for cutLoop
String uom = "";              //will be used as unit of measure (I=inch, M=mm)
//int actualLength set @ top of variables as this may vary depending upon stepper/driver.
String myStop = "";           //will be used in cutLoop to monitor for STOP being pressed
int vRun = 1;                 //will be used in cutLoop to interrupt if STOP pressed

 //feedLoop variables, sent from feed page  - feed wire in/out    (vDirection, vLength)
String vDirection = "";
unsigned long vLength;    //might be able to change to Int

 //testLoop variables, sent from test page  - cycle steppers      (vMotor)
int vMotor;     //might be able to change to String 
 
 //stepLoop variables, sent from steptst page - run stepper x pulses , x step mode
//will use vDirection for step direction 
int vMode = 0;                //Step Mode to run motor in 
int vStep = 0;                //Steps to take
String sMotor = "";
int intPinA = 0;              //will be used for cutter stop switch for cutting
int intPinB = 0;              //will be used for cutter stop switch for opening
int openLimit = 14;           //default open limit is set to 14 for A0 - will change if A1 is hit instead using defineLimit()
int closeLimit = 15;          //default close limit is set to 15 for A1 - will change if A0 is hit instead using defineLimit()


 
//-----------------------------------------------------------------------------------------------

void setup() {
 Serial.begin(9600);    //used to monitor - arduino recieve
 
 digitalWrite(enablePinA,HIGH); //disable driveA
 digitalWrite(enablePinB,HIGH); //disable driveB
 pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);   //setting these so stepper disabled on startup to prevent chatter movement
 pinMode(3, OUTPUT);
 pinMode(4, OUTPUT);
 pinMode(5, OUTPUT);
 pinMode(6, OUTPUT);
 pinMode(7, OUTPUT);
 pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);   //setting these so stepper disabled on startup to prevent chatter movement
 pinMode(9, OUTPUT);
 pinMode(10, OUTPUT);
 pinMode(11, OUTPUT);
 pinMode(12, OUTPUT);
 pinMode(13, OUTPUT);
 pinMode(14, INPUT);  //using Analog 0 as digital I/O for cutter open stop switch
 pinMode(15, INPUT);  //using Analog 1 as digital I/O for cutter close stop switch
 
//Update actualLength based on step mode (if 1/4 step then it wont be the same as if full step)
if(cutStepMode==1){
  actualLength=actualLength * 2;
  stripSteps=stripSteps * 2;
}else if(cutStepMode==2) {
  actualLength=actualLength * 4;
  stripSteps=stripSteps * 4;
}else if(cutStepMode==3) {
  actualLength=actualLength * 8;
  stripSteps=stripSteps * 8;
}else if(cutStepMode==4) {
  actualLength=actualLength * 16;
  stripSteps=stripSteps * 16;
}

 delay(500);
 //openCutter();  //open cutter to 0 position <--moved to run after limit switch init.
}


void loop() {       
 if(Serial.available()) {
    dfd += char(Serial.read());   //read data on serial
   }

 //check for cut command(s)
    if((dfd.substring(0,3)=="cut") & (dfd.length()==10)) {    //START button data recieved
     if(dfd.substring(3,4)=="1") preStripValue = float(dfd.substring(5).toInt()) / 10;
     if(dfd.substring(3,4)=="2") lengthValue = float(dfd.substring(5).toInt()) / 10;
     if(dfd.substring(3,4)=="3") postStripValue = float(dfd.substring(5).toInt()) / 10;
     if(dfd.substring(3,4)=="4") {
      uom = dfd.substring(4,5);
      qtyValue = dfd.substring(5).toInt();
      delay(50);
      dfd="";
      cutLoop();
      while(Serial.available()){Serial.read();}   //clear serial buffer of anything sent after loop completed.
      delay(100);
     }
    dfd=""; 
    }

 //check for Limit Switch check
 else if((dfd.substring(0,7)=="LSCHECK") & (dfd.length()==7)) {     //Recieved command to assist load
  defineLimit();
  while(Serial.available()){Serial.read();}   //clear serial buffer of anything sent after loop completed.
  delay(100);
  dfd = ""; 
 }
 
 //Check for feed command(s)
   else if(((dfd.substring(0,3)=="DIN") or (dfd.substring(0,3)=="OUT")) & (dfd.length()==6)) {    //Recieved command to Feed
    vDirection = dfd.substring(0,3);                      //set vDirectino to xxx of xxx010 to be in used in Feed loop
    vLength = dfd.substring(3).toInt();                   //set vLength to xxx of DINxxx, also convert to integer 
    delay(50);
    feedLoop();
    while(Serial.available()){Serial.read();}   //clear serial buffer of anything sent after loop completed.
    delay(100);
    dfd="";
   }
 
 //check for load assist command
 else if((dfd.substring(0,5)=="WPREP") & (dfd.length()==5)) {     //Recieved command to assist load
  loadAssist();
  while(Serial.available()){Serial.read();}   //clear serial buffer of anything sent after loop completed.
  delay(100);
  dfd = ""; 
 }
 
 //check for test command(s)
  else if((dfd.substring(0,5)=="MTEST") & (dfd.length()==6)) {    //Recieved command to Test (MTEST1 MTEST2 MTEST3)
   vMotor = dfd.substring(5).toInt();                             //set vMotor to 1,2,or 3 to determine which motor to run in testLoop
   delay(50);
   testLoop();
   while(Serial.available()){Serial.read();}   //clear serial buffer of anything sent after loop completed.
   delay(100);
   dfd = ""; 
  }

 //Check for step move (steptst) command(s)
  else if(((dfd.substring(6)=="DIN") or (dfd.substring(6)=="OUT")) & (dfd.length()==9)) {
   vMode = dfd.substring(0,1).toInt();
   vStep = dfd.substring(2,6).toInt();
   sMotor = dfd.substring(1,2);
   vDirection = dfd.substring(6);
   delay(50);
   steptst();
   while(Serial.available()){Serial.read();}   //clear serial buffer of anything sent after loop completed.
   delay(100);
   dfd = "";
  }

}


//----------------------LOOP FOR CUTTING WIRE-----------------------------------------  STOPPED HERE, NEED TO REVERSE STEPPERS AS A is FEED, B is CUT (or verify them), ALSO NEED TO SET STEP MODE etc
void cutLoop() {
  delay(500);                     //delay to let screen catch up
  Serial.print("vis go,0");    //change to status page
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  delay(300);
  Serial.print("t2.txt=\"Running..\"");    //change status
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  delay(300);

  while(Serial.available()){Serial.read();}   //clear serial buffer of any lagging junk from screen/variable change. Sometimes the 0xff write is caught in buffer and interrupts myStop check.

  digitalWrite(enablePinA,LOW); //enable stepperA
  digitalWrite(enablePinB,LOW); //enable stepperB  
  delay(300);

  if(uom=="I") {    //input is in inches. Convert to mm for standard processing.
    preStripValue = preStripValue * 25.4;
    lengthValue = lengthValue * 25.4;
    postStripValue = postStripValue * 25.4;
  }
  
  for(int a = 1; a < (qtyValue + 1); a++) {         //loop 1 for each qty entered
   if(Serial.available() & (vRun==1)) {      //checking serial data again, in case STOP is sent
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
    
    //Setup and Run feed motor
   if(feedStepMode==4){
   digitalWrite(ms1A,HIGH);      //going to do this at 1/16 step
   digitalWrite(ms2A,HIGH);
   digitalWrite(ms3A,HIGH);
   }else if(feedStepMode==3) {
   digitalWrite(ms1A,HIGH);      //going to do this at 1/8 step
   digitalWrite(ms2A,HIGH);
   digitalWrite(ms3A,LOW); 
   }else if(feedStepMode==2) {
   digitalWrite(ms1A,LOW);      //going to do this at 1/4 step
   digitalWrite(ms2A,HIGH);
   digitalWrite(ms3A,LOW); 
   }else if(feedStepMode==1) {
   digitalWrite(ms1A,HIGH);      //going to do this at 1/2 step
   digitalWrite(ms2A,LOW);
   digitalWrite(ms3A,LOW);
   }else {
   digitalWrite(ms1A,LOW);      //going to do this at full step
   digitalWrite(ms2A,LOW);
   digitalWrite(ms3A,LOW);
   }
     
   if(preStripValue > 0){   //only run if preStripValue was entered
    //Code to run FEED motor for preCut Strip Cut
    float preCutL = preStripValue * actualLength;       //multiply input by actualLength(int variable set at start)
    int newPreCutL = round(preCutL);
    digitalWrite(dirPinA,LOW);         //set feed direction to CCW to feed wire
    digitalWrite(enablePinA,LOW);      //enable feed driver
    for(int x = 0; x < newPreCutL; x++) {   
     digitalWrite(stepPinA,HIGH);
     delayMicroseconds(500);         //this controls the speed of the turn. 500 is pretty smooth but 900 is slower
     digitalWrite(stepPinA,LOW);
     delayMicroseconds(500);
   }
   delay(500);
      //Code to run CUT motor for preCut strip cut
    stripCutter();   //loop that will close strip cutter using stripSteps, then opens cutter
   }
   
    //Code to run FEED motor for actual Length (dirPinA still set HIGH from 1st preCut)
    //int cutL = lengthValue * actualLength;       //multiply input by actualLength(int variable set at start)
    float cutL = lengthValue * actualLength;
    int newCutL = round(cutL);  //round to whole number for loop below
       
    for(int x = 0; x < newCutL; x++) {  //multiply input by 6.23 because it takes 623 steps for 100mm, so 6.23 for 1mm (+/- mm)
     digitalWrite(stepPinA,HIGH);
     delayMicroseconds(500);         //this controls the speed of the turn. 500 is pretty smooth but 900 is slower
     digitalWrite(stepPinA,LOW);
     delayMicroseconds(500);
   }
   delay(500);

  if(postStripValue > 0){   //only run if postStripValue was entered   
   stripCutter();   //loop that will close strip cutter using stripSteps, then opens cutter
    //Code to run FEED motor for length postCut (dirPinA still set HIGH from 1st preCut)
   float postCutL = postStripValue * actualLength;       //multiply input by actualLength(int variable set at start) 
   int newPostCutL = round(postCutL);
   for(int x = 0; x < newPostCutL; x++) {   //multiply input by 6.23 because it takes 623 steps for 100mm, so 6.23 for 1mm (+/- mm)
     digitalWrite(stepPinA,HIGH);
     delayMicroseconds(500);         //this controls the speed of the turn. 500 is pretty smooth but 900 is slower
     digitalWrite(stepPinA,LOW);
     delayMicroseconds(500);
   }
   delay(500);
  }
  
    //Code to run CUT motor for final Cut (dirPinB still set LOW from precut)
   closeCutter();
   openCutter();
   delay(1000);   //end of cut delay
   }
    myStop="";
  }
  
    digitalWrite(enablePinA,HIGH); //disable driveA
    digitalWrite(enablePinB,HIGH); //disable driveB
    digitalWrite(dirPinA,LOW);    //not needed, but why keep dir pin HIGH if not in-use
    digitalWrite(dirPinB,LOW);    //not needed, but why keep dir pin HIGH if not in-use
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
   if(feedStepMode==4){
   digitalWrite(ms1A,HIGH);      //going to do this at 1/16 step
   digitalWrite(ms2A,HIGH);
   digitalWrite(ms3A,HIGH);
   }else if(feedStepMode==3) {
   digitalWrite(ms1A,HIGH);      //going to do this at 1/8 step
   digitalWrite(ms2A,HIGH);
   digitalWrite(ms3A,LOW); 
   }else if(feedStepMode==2) {
   digitalWrite(ms1A,LOW);      //going to do this at 1/4 step
   digitalWrite(ms2A,HIGH);
   digitalWrite(ms3A,LOW); 
   }else if(feedStepMode==1) {
   digitalWrite(ms1A,HIGH);      //going to do this at 1/2 step
   digitalWrite(ms2A,LOW);
   digitalWrite(ms3A,LOW);
   }else {
   digitalWrite(ms1A,LOW);      //going to do this at full step
   digitalWrite(ms2A,LOW);
   digitalWrite(ms3A,LOW);
   }
  digitalWrite(enablePinA,LOW); //enable driveA
  //run feed motor
  if(vDirection=="DIN"){
   digitalWrite(dirPinA,LOW);  //spin CCW
  }
  else{
    digitalWrite(dirPinA,HIGH);  //spin CW
  }
  if(vLength==001){
    for(int x = 0; x < (1 * actualLength); x++) {   
     digitalWrite(stepPinA,HIGH);
     delayMicroseconds(500);         //this controls the speed of the turn. 500 is pretty smooth but 900 is slower
     digitalWrite(stepPinA,LOW);
     delayMicroseconds(500);
   }
  }

  if(vLength==10){
    for(int x = 0; x < (10 * actualLength); x++) {   
     digitalWrite(stepPinA,HIGH);
     delayMicroseconds(500);         //this controls the speed of the turn. 500 is pretty smooth but 900 is slower
     digitalWrite(stepPinA,LOW);
     delayMicroseconds(500);
   }
 }
  
 if(vLength==100){
    for(int x = 0; x < (100 * actualLength); x++) {   
     digitalWrite(stepPinA,HIGH);
     delayMicroseconds(500);         //this controls the speed of the turn. 500 is pretty smooth but 900 is slower
     digitalWrite(stepPinA,LOW);
     delayMicroseconds(500);
   }
 }
 
   digitalWrite(dirPinA,LOW);  
   delay(1000);
 //Serial.print("feed loop " + (vDirection) + " : " + String(vLength));
 
 digitalWrite(enablePinA,HIGH); //disable driveA
 dfd="";
}
//------------------------------------------------------------------------------------

//----------------------LOOP FOR LOAD ASSIST------------------------------------------
void loadAssist() {
  //This loop is to run the feed stepper using
  //vDirection = DIN or DOUT for clockwise or counterclockwise direction
  //vLength = 001/010/100 for mm to travel
   if(feedStepMode==4){
   digitalWrite(ms1A,HIGH);      //going to do this at 1/16 step
   digitalWrite(ms2A,HIGH);
   digitalWrite(ms3A,HIGH);
   }else if(feedStepMode==3) {
   digitalWrite(ms1A,HIGH);      //going to do this at 1/8 step
   digitalWrite(ms2A,HIGH);
   digitalWrite(ms3A,LOW); 
   }else if(feedStepMode==2) {
   digitalWrite(ms1A,LOW);      //going to do this at 1/4 step
   digitalWrite(ms2A,HIGH);
   digitalWrite(ms3A,LOW); 
   }else if(feedStepMode==1) {
   digitalWrite(ms1A,HIGH);      //going to do this at 1/2 step
   digitalWrite(ms2A,LOW);
   digitalWrite(ms3A,LOW);
   }else {
   digitalWrite(ms1A,LOW);      //going to do this at full step
   digitalWrite(ms2A,LOW);
   digitalWrite(ms3A,LOW);
   }
  openCutter();
  delay(500);
  digitalWrite(enablePinA,LOW); //enable driveA
  //run feed motor
   digitalWrite(dirPinA,LOW);  //spin CCW
    for(int x = 0; x < (80 * actualLength); x++) {    //feed ~80mm of wire
     digitalWrite(stepPinA,HIGH);
     delayMicroseconds(500);         //this controls the speed of the turn. 500 is pretty smooth but 900 is slower
     digitalWrite(stepPinA,LOW);
     delayMicroseconds(500);
   }
  digitalWrite(dirPinA,LOW);  
   delay(1000);
 digitalWrite(enablePinA,HIGH); //disable driveA
 //cut any excess wire that feed out
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
 //Serial.print("test loop " + String(vMotor));
 if(vMotor==1){      //test FEED motor
   if(feedStepMode==4){
   digitalWrite(ms1A,HIGH);      //going to do this at 1/16 step
   digitalWrite(ms2A,HIGH);
   digitalWrite(ms3A,HIGH);
   }else if(feedStepMode==3) {
   digitalWrite(ms1A,HIGH);      //going to do this at 1/8 step
   digitalWrite(ms2A,HIGH);
   digitalWrite(ms3A,LOW); 
   }else if(feedStepMode==2) {
   digitalWrite(ms1A,LOW);      //going to do this at 1/4 step
   digitalWrite(ms2A,HIGH);
   digitalWrite(ms3A,LOW); 
   }else if(feedStepMode==1) {
   digitalWrite(ms1A,HIGH);      //going to do this at 1/2 step
   digitalWrite(ms2A,LOW);
   digitalWrite(ms3A,LOW);
   }else {
   digitalWrite(ms1A,LOW);      //going to do this at full step
   digitalWrite(ms2A,LOW);
   digitalWrite(ms3A,LOW);
   }
   digitalWrite(enablePinA,LOW); //enable driveA
   //Code to run FEED motor Counter Clockwise
   digitalWrite(dirPinA,LOW);  
    for(int x = 0; x < (100 * actualLength); x++) {   
     digitalWrite(stepPinA,HIGH);
     delayMicroseconds(500);         
     digitalWrite(stepPinA,LOW);
     delayMicroseconds(500);
   }
   digitalWrite(dirPinA,HIGH);
   delay(1000);
   //Code to run FEED motor Clockwise
    for(int x = 0; x < (100 * actualLength); x++) {   
     digitalWrite(stepPinA,HIGH);
     delayMicroseconds(500);         
     digitalWrite(stepPinA,LOW);
     delayMicroseconds(500);
   }   
   delay(1000);
   digitalWrite(enablePinA,HIGH); //disable driveA
   digitalWrite(dirPinA,LOW);
 }

 if(vMotor==2){      //test CUT motor, close until switch is hit then open until switch is hit
  closeCutter();
  openCutter();
 }
  dfd="";
}
//------------------------------------------------------------------------------------

//----------------------LOOP FOR STEP TEST OF MOTORS----------------------------------
void steptst() {
  //Serial.println("Mode:" + String(vMode) + " , Step:" + String(vStep) + " , Motor:" + String(sMotor) + " , Dir:" + String(vDirection));
  if(sMotor=="F") {               //Setup Feed Motor
   if(vDirection=="DIN") {        //Set direction
    digitalWrite(dirPinA,HIGH);
   }
   else{
    digitalWrite(dirPinA,LOW);
   }
   if(vMode==4) {                //mode is 1/16 step
    digitalWrite(ms1A,HIGH);
    digitalWrite(ms2A,HIGH);
    digitalWrite(ms3A,HIGH);
   }
   else if(vMode==3) {           //mode is 1/8 step
    digitalWrite(ms1A,HIGH);
    digitalWrite(ms2A,HIGH);
    digitalWrite(ms3A,LOW);
   }
   else if(vMode==2) {           //mode is 1/4 step
    digitalWrite(ms1A,LOW);
    digitalWrite(ms2A,HIGH);
    digitalWrite(ms3A,LOW);
   }
   else if(vMode==1) {           //mode is 1/2 step
    digitalWrite(ms1A,HIGH);
    digitalWrite(ms2A,LOW);
    digitalWrite(ms3A,LOW);
   }
   else if(vMode==0) {           //mode is full step
    digitalWrite(ms1A,LOW);
    digitalWrite(ms2A,LOW);
    digitalWrite(ms3A,LOW);
   }
   //Code to run motor 
   digitalWrite(enablePinA,LOW); //enable driveA
   for(int x = 0; x < vStep; x++) {   
     digitalWrite(stepPinA,HIGH);
     delayMicroseconds(500);         
     digitalWrite(stepPinA,LOW);
     delayMicroseconds(500);
   }
   digitalWrite(enablePinA,HIGH); //disable driveA
   delay(1000);
   
 }
 
 else if(sMotor=="C") {          //Setup Cut Motor
   if(vMode==4) {                //mode is 1/16 step
    digitalWrite(ms1B,HIGH);
    digitalWrite(ms2B,HIGH);
    digitalWrite(ms3B,HIGH);
   }
   else if(vMode==3) {           //mode is 1/8 step
    digitalWrite(ms1B,HIGH);
    digitalWrite(ms2B,HIGH);
    digitalWrite(ms3B,LOW);
   }
   else if(vMode==2) {           //mode is 1/4 step
    digitalWrite(ms1B,LOW);
    digitalWrite(ms2B,HIGH);
    digitalWrite(ms3B,LOW);
   }
   else if(vMode==1) {           //mode is 1/2 step
    digitalWrite(ms1B,HIGH);
    digitalWrite(ms2B,LOW);
    digitalWrite(ms3B,LOW);
   }
   else if(vMode==0) {           //mode is full step
    digitalWrite(ms1B,LOW);
    digitalWrite(ms2B,LOW);
    digitalWrite(ms3B,LOW);
   }
   //Code to run motor 
   digitalWrite(enablePinB,LOW); //enable driveB
   if(vDirection=="DIN") {    //motor is being stepped in CW direction (cut)
    digitalWrite(dirPinB,HIGH);
    for(int x = 0; x < vStep; x++) {
      intPinA = digitalRead(closeLimit);  //get A1 switch state 1=OPEN 0=CLOSED
      if(intPinA==1){   //switch is OPEN so step motor
        digitalWrite(stepPinB,HIGH);
        delayMicroseconds(500);         
        digitalWrite(stepPinB,LOW);
        delayMicroseconds(500);
      }else {   //switch is CLOSED so we have hit max limit
        digitalWrite(dirPinB,LOW);    //back off so switch is not CLOSED
        digitalWrite(ms1B,HIGH);      //going to do this at 1/16 step
        digitalWrite(ms2B,HIGH);
        digitalWrite(ms3B,HIGH);
        while(intPinA != 1) {   //move until switch is OPEN
          intPinA = digitalRead(15);
          digitalWrite(stepPinB,HIGH);
          delayMicroseconds(800);         
          digitalWrite(stepPinB,LOW);
          delayMicroseconds(800);
        }
        break;
      } //close else loop
    } //close step loop
   }else {    //motor is being stepped in CCW direction (open)
    digitalWrite(dirPinB,LOW);
    for(int x = 0; x < vStep; x++) {
      intPinB = digitalRead(openLimit);
      if(intPinB==1){   //switch is OPEN so step motor
        digitalWrite(stepPinB,HIGH);
        delayMicroseconds(500);         
        digitalWrite(stepPinB,LOW);
        delayMicroseconds(500);
      }else {   //switch is CLOSED so we have hit max limit
        digitalWrite(dirPinB,HIGH);    //back off so switch is not CLOSED
        digitalWrite(ms1B,HIGH);      //going to do this at 1/16 step
        digitalWrite(ms2B,HIGH);
        digitalWrite(ms3B,HIGH);
        while(intPinB != 1) {   //move until switch is OPEN
          intPinB = digitalRead(14);
          digitalWrite(stepPinB,HIGH);
          delayMicroseconds(800);         
          digitalWrite(stepPinB,LOW);
          delayMicroseconds(800);
        }
        break;
      } //close else loop
    } //close step loop
   }
   
   digitalWrite(enablePinB,HIGH); //disable driveB
   delay(500);
 }
}
//------------------------------------------------------------------------------------

void openCutter() {
   if(cutStepMode==4){
   digitalWrite(ms1B,HIGH);      //going to do this at 1/16 step
   digitalWrite(ms2B,HIGH);
   digitalWrite(ms3B,HIGH);
   }else if(cutStepMode==3) {
   digitalWrite(ms1B,HIGH);      //going to do this at 1/8 step
   digitalWrite(ms2B,HIGH);
   digitalWrite(ms3B,LOW); 
   }else if(cutStepMode==2) {
   digitalWrite(ms1B,LOW);      //going to do this at 1/4 step
   digitalWrite(ms2B,HIGH);
   digitalWrite(ms3B,LOW); 
   }else if(cutStepMode==1) {
   digitalWrite(ms1B,HIGH);      //going to do this at 1/2 step
   digitalWrite(ms2B,LOW);
   digitalWrite(ms3B,LOW);
   }else {
   digitalWrite(ms1B,LOW);      //going to do this at full step
   digitalWrite(ms2B,LOW);
   digitalWrite(ms3B,LOW);
   }
   
   digitalWrite(enablePinB,LOW); //enable driveB
   digitalWrite(dirPinB,LOW);     //will be turning CCW
   for(int x = 0; x < 5000; x++) {      //putting in stupid high number for steps, as we will utilize switch for stop  
    intPinB = digitalRead(openLimit); //get A0 state LOW/HIGH
    if(intPinB == 1) {       //switch has not been hit so run
     digitalWrite(stepPinB,HIGH);
     delayMicroseconds(500);         
     digitalWrite(stepPinB,LOW);
     delayMicroseconds(500);
    }else {     //switch has been hit, move back to reset switch
      digitalWrite(dirPinB,HIGH);   //reverse to CW direction to move off of switch
      delay(500);
      while(intPinB != 1) {    //move until switch goes back to open
       intPinB = digitalRead(openLimit);  //re-read A0 state for change 
       digitalWrite(stepPinB,HIGH);
       delayMicroseconds(800);         
       digitalWrite(stepPinB,LOW);
       delayMicroseconds(800);
      }
      break;    //exit loop to stop motor from cont to try and move
    }
   }

   digitalWrite(enablePinB,HIGH); //disable driveB
   delay(800);
}

void closeCutter() {
   if(cutStepMode==4){
   digitalWrite(ms1B,HIGH);      //going to do this at 1/16 step
   digitalWrite(ms2B,HIGH);
   digitalWrite(ms3B,HIGH);
   }else if(cutStepMode==3) {
   digitalWrite(ms1B,HIGH);      //going to do this at 1/8 step
   digitalWrite(ms2B,HIGH);
   digitalWrite(ms3B,LOW); 
   }else if(cutStepMode==2) {
   digitalWrite(ms1B,LOW);      //going to do this at 1/4 step
   digitalWrite(ms2B,HIGH);
   digitalWrite(ms3B,LOW); 
   }else if(cutStepMode==1) {
   digitalWrite(ms1B,HIGH);      //going to do this at 1/2 step
   digitalWrite(ms2B,LOW);
   digitalWrite(ms3B,LOW);
   }else {
   digitalWrite(ms1B,LOW);      //going to do this at full step
   digitalWrite(ms2B,LOW);
   digitalWrite(ms3B,LOW);
   }
 
   digitalWrite(enablePinB,LOW); //enable driveB
   digitalWrite(dirPinB,HIGH);    //will be turning CW
   for(int x = 0; x < 5000; x++) {      //putting in stupid high number for steps, as we will utilize switch for stop
    intPinA = digitalRead(closeLimit); //get A1 state LOW/HIGH
    if(intPinA == 1) {       //switch has not been hit so run
     digitalWrite(stepPinB,HIGH);
     delayMicroseconds(500);         
     digitalWrite(stepPinB,LOW);
     delayMicroseconds(500);
    }else {     //switch has been hit, move back to reset switch
      digitalWrite(dirPinB,LOW);   //reverse to CCW direction to move off of switch
      delay(500);
      while(intPinA != 1) {    //move until switch goes back to open
       intPinA = digitalRead(closeLimit);  //re-read A5 state for change 
       digitalWrite(stepPinB,HIGH);
       delayMicroseconds(800);         
       digitalWrite(stepPinB,LOW);
       delayMicroseconds(800);
      }
      break;    //exit loop to stop motor from cont to try and move
    }
   }

   digitalWrite(enablePinB,HIGH); //disable driveB
   delay(800);
}

void stripCutter() {
   if(cutStepMode==4){
   digitalWrite(ms1B,HIGH);      //going to do this at 1/16 step
   digitalWrite(ms2B,HIGH);
   digitalWrite(ms3B,HIGH);
   }else if(cutStepMode==3) {
   digitalWrite(ms1B,HIGH);      //going to do this at 1/8 step
   digitalWrite(ms2B,HIGH);
   digitalWrite(ms3B,LOW); 
   }else if(cutStepMode==2) {
   digitalWrite(ms1B,LOW);      //going to do this at 1/4 step
   digitalWrite(ms2B,HIGH);
   digitalWrite(ms3B,LOW); 
   }else if(cutStepMode==1) {
   digitalWrite(ms1B,HIGH);      //going to do this at 1/2 step
   digitalWrite(ms2B,LOW);
   digitalWrite(ms3B,LOW);
   }else {
   digitalWrite(ms1B,LOW);      //going to do this at full step
   digitalWrite(ms2B,LOW);
   digitalWrite(ms3B,LOW);
   }
 
   digitalWrite(enablePinB,LOW); //enable driveB
   digitalWrite(dirPinB,HIGH);    //will be turning CW
   for(int x = 0; x < stripSteps; x++) {      
    //intPinA = digitalRead(15); //get A1 state LOW/HIGH
    intPinA = digitalRead(closeLimit); //get A1 state LOW/HIGH
    if(intPinA == 1) {       //switch has not been hit so run
     digitalWrite(stepPinB,HIGH);
     delayMicroseconds(500);         
     digitalWrite(stepPinB,LOW);
     delayMicroseconds(500);
    }else {     //switch has been hit, move back to reset switch
      digitalWrite(dirPinB,LOW);   //reverse to CCW direction to move off of switch
      delay(500);
      while(intPinA != 1) {    //move until switch goes back to open
       intPinA = digitalRead(closeLimit);  //re-read A1 state for change 
       digitalWrite(stepPinB,HIGH);
       delayMicroseconds(800);         
       digitalWrite(stepPinB,LOW);
       delayMicroseconds(800);
      }
      break;    //exit loop to stop motor from cont to try and move
    }
   }
   delay(300);
   openCutter();
   digitalWrite(enablePinB,HIGH); //disable driveB
   delay(300);
}

void defineLimit() {      //set open/close limit switch assignment(s). Added this because I kept plugging the switches in to the wrong connectors.
  //We are going to spin the motor CCW until a switch is triggered, then assign that switch to the OPEN switch
 digitalWrite(enablePinB,LOW); //enable driveB
 digitalWrite(dirPinB,LOW);     //will be turning CCW
 for(int x = 0;x < 10000; x++) {    //putting in for 10000 steps so plenty of steps to hit a switch, but dont want to run forever incase issue
  intPinB = digitalRead(14);
  intPinA = digitalRead(15);
   if(intPinA == 1 && intPinB == 1) {       //no switch has not been hit so step
     digitalWrite(stepPinB,HIGH);
     delayMicroseconds(500);         
     digitalWrite(stepPinB,LOW);
     delayMicroseconds(500);
    }else {     //a limit switch has been hit
     if(intPinA == 0) { //pin 15 (A1) was triggered, this is not the expected switch (default of pin 14 (A0)) so we will update 
      openLimit = 15;   //by default this is set to 14
      closeLimit = 14;
     }
      digitalWrite(dirPinB,HIGH);   //reverse to CW direction to move off of switch
      delay(500);
      while(intPinB != 1) {    //move until switch goes back to open
       intPinB = digitalRead(14);  //re-read A0 state for change 
       digitalWrite(stepPinB,HIGH);
       delayMicroseconds(800);         
       digitalWrite(stepPinB,LOW);
       delayMicroseconds(800);
      }
      break;    //exit loop to stop motor from cont to try and move
    }
   }
  digitalWrite(enablePinB,HIGH); //disable driveB
  delay(500);
  openCutter();
  delay(500);
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
}
  
