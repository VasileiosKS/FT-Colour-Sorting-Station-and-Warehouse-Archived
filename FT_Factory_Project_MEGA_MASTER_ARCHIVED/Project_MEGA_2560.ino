               //------Libraries------//
#include <AFMotor.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SparkFun_APDS9960.h>
              //------DEBUG MODE Enable/Disable------//
#define DEBUG 1
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugR(x) Serial.read(x)
#else
#define debug(x)  
#define debugln(x)
#define debugR(x)
#endif
    //------UART COMMUNICATION Enable/Disable------//
#define UART_EN 1
#if UART_EN == 1
#define RXPIN 16
#define TXPIN 17

#endif
//------------------------------------------------------------//                
               //------Constants & Variables------//
    int Mode = 0;
    int c;
    int l;
    int dir;
    int infr = 0;
    int limr = 0;
    int airs = 0;
boolean p=false;
boolean r=false;    
boolean busy = false;
boolean infp = false;
boolean limp = false;
boolean color_rec=false;
boolean Colorrun = false; //???
boolean IRT3=false; //???
    int CheckInfraredResult;
boolean CheckInfrared = false;
//------------------------------------------------------------//   
/*  ARDUINO MEGA2560 GPIO  */
               //------Colour Indicator LED's------//
#define RedLed   22
#define GreenLed 23
#define BlueLed  24 
//------------------------------------------------------------//    
        //------Module 1------// 
//------Infrared "Detection"------//                      
#define Mod1_IR1 26
#define Mod1_IR2 27
#define Mod1_IR3 28
        //------Module 2------// 
//------Infrared "Detection"------//             
#define Mod2_IR1 29
#define Mod2_IR2 30
#define Mod2_IR3 31
#define Mod2_IR4 32
        //------Module 3------//
//Limit Switches          
#define Mod3_Rotating_Base_0 34
#define Mod3_Rotating_Base_1 35         
#define Mod3_Gripper_0 36           
#define Mod3_Gripper_1 37 
#define Mod3_Plat_0 38
#define Mod3_Plat_1 39
//------Module 4------//  
      //NO NEED
//------------------------------------------------------------//       
             //-------Vacuum System-------//
#define Air_Compressor          47           
#define Air_Valve_R             48
#define Air_Valve_G             49
#define Air_Valve_B             50
#define Air_Valve_RGB_Rtrct     51
#define Air_Valve_Gripper_Push  52
#define Air_Valve_Gripper_Rtrct 53
//------------------------------------------------------------//   
             //Switch between Color Sorting Mode & Storage Mode          
#define Mode_Switch 78  //By Default Color Sorting is Selected              
//------------------------------------------------------------//   
              //-------APDS9960-------//
#define APDS9960_INT    2  //INT[2]
SparkFun_APDS9960 apds = SparkFun_APDS9960();
//------------------------------------------------------------//   
             //-------MOTOR SHIELD-------//
AF_DCMotor m1(1);
AF_DCMotor m2(2);
AF_DCMotor m3(3);
AF_DCMotor m4(4);                     
//------------------------------------------------------------//   
             //-------Adafruit_SSD1306-------//           
#define OLED_RESET 4
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
Adafruit_SSD1306 display(OLED_RESET);
//------------------------------------------------------------//            
void setup() {
  Serial.begin(9600);
  debugln(F("--------------------------------"));
  debugln(F("SparkFun APDS-9960 - ColorSensor"));
  pinMode(Air_Compressor, OUTPUT);
  pinMode(Air_Valve_R, OUTPUT);
  pinMode(Air_Valve_G, OUTPUT);
  pinMode(Air_Valve_B, OUTPUT);
  pinMode(Air_Valve_RGB_Rtrct, OUTPUT);
  pinMode(Air_Valve_Gripper_Push, OUTPUT);
  pinMode(Air_Valve_Gripper_Rtrct, OUTPUT);
  pinMode(Mod1_IR1, INPUT);
  pinMode(Mod1_IR2, INPUT);
  pinMode(Mod2_IR1, INPUT);
  pinMode(Mod2_IR2, INPUT);
  pinMode(Mod2_IR3, INPUT);
  pinMode(Mod2_IR4, INPUT);
  pinMode(Mod3_Gripper_0, INPUT_PULLUP);
  pinMode(Mod3_Gripper_1, INPUT_PULLUP); 
  pinMode(Mod3_Rotating_Base_0, INPUT_PULLUP);
  pinMode(Mod3_Rotating_Base_1, INPUT_PULLUP);
  pinMode(Mod3_Plat_0, INPUT_PULLUP);
  pinMode(Mod3_Plat_1, INPUT_PULLUP);
  pinMode(Mode_Switch, INPUT_PULLUP);
  //------Initialize APDS-9960------//
    if ( apds.init() ) {
    debug(F("APDS-9960 initialization complete"));
  } else {
    debugln(F("Something went wrong during APDS-9960 init!"));
  }
  if ( apds.enableLightSensor(false) ) {
    debugln(F("Light sensor is now running"));
  } else {
    debugln(F("Something went wrong during light sensor init!"));
  }
  _delay_ms(50);  
  //------Initialize Adafruit_SSD1306------// 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
  display.display();  
  //------Initialize Motors------// 
  m1.setSpeed(255);
  m2.setSpeed(255);
  m3.setSpeed(255);
  m4.setSpeed(255);
  m1.run(RELEASE);
  m2.run(RELEASE);
  m3.run(RELEASE);
  m4.run(RELEASE); 
  digitalWrite(Air_Valve_R,LOW);
  digitalWrite(Air_Valve_G,LOW);
  digitalWrite(Air_Valve_B,LOW);
  digitalWrite(Air_Valve_RGB_Rtrct,LOW);
  digitalWrite(Air_Valve_Gripper_Push,LOW);
  digitalWrite(Air_Valve_Gripper_Rtrct,LOW);
  debugln("SETUP COMPLETE");  
  _delay_ms(100);  
}

//------------------------------------------------------------//   
void loop(){
    SerRead();
    program();
}    

void program(){
  
    if(Mode==0){        // SORTING STATION MODE
      sorter();
    }
    else if(Mode==1){  // WAREHOUSE MODE
     warehouse();
    }
}

//------------------------------------------------------------//   SORTING STATION
void sorter(){
    if(digitalRead(Mod2_IR1) == HIGH){  
      if(IRT3 == false)
        Mod2_ConvBelt(1);    
    } 
    else if((digitalRead(Mod2_IR1) == LOW)){
        _delay_ms(100);
        if(IRT3==false){
          Mod2_ConvBelt(0);
          process(); 
        }
        IRT3=false; 
    }    
}
//------------------------------------------------------------//   
void warehouse(){
  while(digitalRead(Mod2_IR4) == LOW && busy==false){
        Mod2_ConvBelt(1);    
  }      
      _delay_ms(250);
      Mod2_ConvBelt(0);
      busy=true;
      pickConvAndDropWH();
}
//------------------------------------------------------------//   
void pickConvAndDropWH(){
      while(digitalRead(Mod3_Gripper_1) == HIGH){
      Serial.println("Moving Lift To Top");
      m3.run(BACKWARD);
      }
      m3.run(RELEASE);  
      _delay_ms(250);
      while(digitalRead(Mod3_Plat_0) == HIGH){
        Serial.println("MOVING Platform TO START");
        m2.run(FORWARD);
      }
      m2.run(RELEASE);
      _delay_ms(200);
      while(digitalRead(Mod3_Rotating_Base_0) == HIGH){
        Serial.println("Rotating Base TO START");
        m4.run(FORWARD);
      }
      m4.run(RELEASE);  
      _delay_ms(200);
      /*PUSH PISTON */
      while(p==false){
        digitalWrite(Air_Compressor,HIGH);
        digitalWrite(Air_Valve_Gripper_Push,HIGH);
        digitalWrite(Air_Valve_Gripper_Rtrct,LOW);
        _delay_ms(1500);
        p=true;
      }
      _delay_ms(200);
      digitalWrite(Air_Compressor,LOW); 
      while(digitalRead(Mod3_Gripper_0) == HIGH){
        Serial.println("Moving Lift To Bot");
        m3.run(FORWARD);
      }
      m3.run(RELEASE);  
     /*RETRACT PISTON*/ 
      while(r==false){
        digitalWrite(Air_Compressor,HIGH);
        digitalWrite(Air_Valve_Gripper_Push,LOW);
        digitalWrite(Air_Valve_Gripper_Rtrct,HIGH);  
        _delay_ms(1500);
        r=true;
      }
       _delay_ms(200);
      digitalWrite(Air_Compressor,LOW);  
      while(digitalRead(Mod3_Gripper_1) == HIGH){
        Serial.println("Moving Lift To Top");
        m3.run(BACKWARD);
      }
      m3.run(RELEASE);    
      _delay_ms(250);
      while(digitalRead(Mod3_Rotating_Base_1) == HIGH){
        Serial.println("Rotating Platform To END"); 
        m4.run(BACKWARD);
      }
      m4.run(RELEASE);  
      _delay_ms(250);
      while(digitalRead(Mod3_Gripper_0) == HIGH){
        Serial.println("Moving LIFT To Bot");
        m3.run(FORWARD);
      } 
      m3.run(RELEASE); 
      p=false; 
      _delay_ms(200);
      /*PUSH PISTON */
      while(p==false){
        digitalWrite(Air_Compressor,HIGH);
        digitalWrite(Air_Valve_Gripper_Push,HIGH);
        digitalWrite(Air_Valve_Gripper_Rtrct,LOW);
        _delay_ms(1500);
        p=true;
      }
      _delay_ms(200);
      digitalWrite(Air_Compressor,LOW);
      digitalWrite(Air_Valve_Gripper_Push,LOW);
      while(digitalRead(Mod3_Gripper_1) == HIGH){
        Serial.println("Moving Lift To Top");
        m3.run(BACKWARD);
      }
      m3.run(RELEASE);   
      debugln("DONE & DUNE");
      busy=false;
}
//------------------------------------------------------------//   PUSHING DISKS TO THEIR RESPECTIVE TRAY ACCORDING TO THEIR COLOUR FROM color() function
void process(){   
  color_rec=true;
  p=false;
  int proc=color(c); 
  debug("EXITING COLOR RECOGNITION");
  if(proc!=" " or proc!=NULL){
    
    if(proc==0){ //RED
        while(digitalRead(Mod2_IR2) == HIGH){
          Mod2_ConvBelt(1);
        }
        _delay_ms(550);  
        Mod2_ConvBelt(0);
        _delay_ms(200);
             /*PUSH PISTON */
            debugln("Pushing Red Object to Color Tray"); 
            digitalWrite(Air_Compressor,HIGH);
            digitalWrite(Air_Valve_R,HIGH);
            digitalWrite(Air_Valve_RGB_Rtrct,LOW); 
            _delay_ms(650);
            digitalWrite(Air_Valve_R,LOW);
            _delay_ms(650);
           /*RETRACT PISTON*/   
            debugln("Retracting Piston from conveyor space");   
            digitalWrite(Air_Valve_RGB_Rtrct,HIGH);
            _delay_ms(650);
            digitalWrite(Air_Valve_RGB_Rtrct,LOW);
            digitalWrite(Air_Compressor,LOW);     
    }
    else if(proc==1){ //GREEN
        while(digitalRead(Mod2_IR3) == HIGH){
          Mod2_ConvBelt(1);
        }
        _delay_ms(550);  
        Mod2_ConvBelt(0);
        _delay_ms(200);
        //pistoni
           debugln("Pushing Green Object to Color Tray"); 
            digitalWrite(Air_Compressor,HIGH);
            digitalWrite(Air_Valve_G,HIGH);
            digitalWrite(Air_Valve_RGB_Rtrct,LOW); 
            _delay_ms(650);
            digitalWrite(Air_Valve_G,LOW);
            _delay_ms(650);
           /*RETRACT PISTON*/   
            debugln("Retracting Piston from conveyor space");   
            digitalWrite(Air_Valve_RGB_Rtrct,HIGH);
            _delay_ms(650);
            digitalWrite(Air_Valve_RGB_Rtrct,LOW);
            digitalWrite(Air_Compressor,LOW);     
    }
    else if(proc==2){ //BLUE
        while(digitalRead(Mod2_IR4) == LOW){
          Mod2_ConvBelt(1);
        }
        _delay_ms(550);  
        Mod2_ConvBelt(0);
        _delay_ms(200);
        //pistoni
           debugln("Pushing Blue Object to Color Tray"); 
            digitalWrite(Air_Compressor,HIGH);
            digitalWrite(Air_Valve_B,HIGH);
            digitalWrite(Air_Valve_RGB_Rtrct,LOW); 
            _delay_ms(650);
            digitalWrite(Air_Valve_B,LOW);
            _delay_ms(650);
           /*RETRACT PISTON*/   
            debugln("Retracting Piston from conveyor space");   
            digitalWrite(Air_Valve_RGB_Rtrct,HIGH);
            _delay_ms(650);
            digitalWrite(Air_Valve_RGB_Rtrct,LOW);
            digitalWrite(Air_Compressor,LOW);     
  }
  else{
    debugln("Something went Wrong with color detection!");
  }
  IRT3=true;
  return;
}
}
//------------------------------------------------------------//   
//COLOR DETECTION
int color(int c){
  uint16_t ambient_light=0,r=0,g=0,b=0,i,s=0,red=0,green=0,blue=0;
   if((color_rec==true) ) {
    debugln("ENTERED COLOR FUNCTION");
    for(i=0;i<=2;i++){ 
      if (  !apds.readAmbientLight(ambient_light) ||
            !apds.readRedLight(r) ||
            !apds.readGreenLight(g) ||
            !apds.readBlueLight(b) ) {
        debugln("Error reading light values");
      } else {
        debug("Ambient: ");
        debug(ambient_light);
        debug(" Red: ");
        debug(r);
        debug(" Green: ");
        debug(g);
        debug(" Blue: ");
        debugln(b);
      }
      red=red+r;
      green=green+g;
      blue=blue+b;
      _delay_ms(1000);  
    }
    debug("RedT: "); debug(red);
    debug(" GreenT: "); debug(green);
    debug (" BlueT: " ); debug(blue); 
    debugln();
    debug("Color is: ");
    red = red/3;
    green = green/3;
    blue = blue/3;  
    if((red >= blue) or (green >= blue)){
      if(red > green ){      
        debug("RED");
        Led(4);
        c=0;
      }  
      else{
        debugln("GREEN");
        Led(2);
        c=1;
      }
    }
    else{  
       debugln("BLUE");  
       Led(1);
       c=2;
    }
    color_rec=false;
    return c;
   }
}
//------------------------------------------------------------//   

//------------------------------------------------------------//   
void ScreenDisplay(){
  
/*switch (mode) {
      case 1: //Warehouse Mode
      
        break;
      default: //Colour Sorting Mode
      
        break;     
    }

*/
}

//------------------------------------------------------------//   
 
void InitSequenceCheck(){
 

  
}
//------------------------------------------------------------//   
int Mod2_ConvBelt(int dir){
  if (dir==1){    
    Serial.println("Forward Conveyor Belt"); 
    m1.run(FORWARD);
  }
  else if (dir==2){
    Serial.println("Reversing Conveyor Belt");
    m1.run(BACKWARD);
  }
  else if (dir==0){
    Serial.println("Shutting Down Conveyor Belt");
    m1.run(RELEASE);
  }
}
//------------------------------------------------------------// 
int Mod3_Plat(int pos){
  if (pos==0){
   m2.run(RELEASE);    
  }
  else if (pos==1){
    while(digitalRead(Mod3_Plat_0) == HIGH){
      Serial.println("MOVING Plat TO START");
      m2.run(FORWARD);
    }
    m2.run(RELEASE);  
  }
  else if (pos==2){
    while(digitalRead(Mod3_Plat_1) == HIGH){
      Serial.println("MOVING Plat TO END");
      m2.run(BACKWARD);
    }
    m2.run(RELEASE);     
  }
  else{}
}
//------------------------------------------------------------//  
int Mod3_Gripper(int pos){
  if (pos==0){
   m3.run(RELEASE);    
  }
  else if (pos==1){
    while(digitalRead(Mod3_Gripper_0) == HIGH){
      Serial.println("MOVING LIFT TO START");
      m3.run(FORWARD);
    }
    m3.run(RELEASE);  
  }
  else if (pos==2){
    while(digitalRead(Mod3_Gripper_1) == HIGH){
      Serial.println("MOVING LIFT TO 1");
      m3.run(BACKWARD);
    }
    m3.run(RELEASE);     
  }
  else{}
}
//------------------------------------------------------------// Y
int Mod3_Base(int pos){
  if (pos==0){
    m4.run(RELEASE);    
  }
  else if (pos==1){
    while(digitalRead(Mod3_Rotating_Base_0) == HIGH){
      Serial.println("MOVING PLATFORM TO START");
      m4.run(FORWARD);
    }
    m4.run(RELEASE);  
  }
  else if (pos==2){
    while(digitalRead(Mod3_Rotating_Base_1) == HIGH){
    Serial.println("MOVING PLATFORM TO END");
    m4.run(BACKWARD);
      }
    m4.run(RELEASE);     
  }
  else{}
}   
//------------------------------------------------------------// 
int pneumatics(int p) { //
    /* PISTON INLET:  COMMON VALVE 1 | NO-> AIR COMPR | NC-> NOT CONNECTED 
    PISTON OUTLET: COMMON VALVE 2 | NO-> AIR COMPR | NC-> NOT CONNECTED
    push:   VALVE1-LOW  | VALVE2-HIGH
    retract:VALVE1-HIGH | VALVE2-LOW */

}
//------------------------------------------------------------// 
void Led(int c){
  if(c==7){ //111
       digitalWrite(RedLed,HIGH);
       delay(1000);
       digitalWrite(RedLed,LOW);
       delay(100);
       digitalWrite(GreenLed,HIGH);
       delay(1000);
       digitalWrite(GreenLed,LOW);
       delay(100);
       digitalWrite(BlueLed,HIGH);
       delay(1000);
       digitalWrite(BlueLed,LOW);
       delay(100); 
  }
  else if(c==4){ //100
     for(int i=0;i<=3;i++){
       digitalWrite(RedLed,HIGH);
       delay(500);
       digitalWrite(RedLed,LOW);
       delay(500);  
   }
  }
  else if(c==2){ //010
   for(int i=0;i<=3;i++){
       digitalWrite(GreenLed,HIGH);
       delay(500);
       digitalWrite(GreenLed,LOW);
       delay(500);  
   }
  }
  else if(c==1){ //001
    for(int i=0;i<=3;i++){
       digitalWrite(BlueLed,HIGH);
       delay(500);
       digitalWrite(BlueLed,LOW);
       delay(500);
    }    
  }
  else{
      digitalWrite(RedLed,LOW);
      digitalWrite(GreenLed,LOW);
      digitalWrite(BlueLed,LOW);
  }
}    
//------------------------------------------------------------// 
int InitCheckInfrared(int CheckInfraredResult){
  if(CheckInfrared == false){
    if((Mod2_IR1  == LOW) || (Mod2_IR2 == LOW) || (Mod2_IR3 == LOW)|| (Mod2_IR4 == HIGH)) {
      Serial.println("Check Infrared Wiring!! or Remove any Objects");
      CheckInfraredResult=0; //0 = failed
      CheckInfrared =true;
    }
    else
      CheckInfraredResult = 1; //1 = passed
  }
}
//------------------------------------------------------------// Serial Commands
void SerRead(){
  char msg;
  msg = Serial.read();
  if(msg=='A'){      //UNALLOCATED
  }
  else if(msg=='S'){ //UNALLOCATED
  }
  else if(msg=='D'){ //UNALLOCATED
  } 
  else if(msg=='Q'){ 
    debugln("Manually Setting Conveyor Belt 1 Direction: Forward");
    Mod2_ConvBelt(1);
  }
  else if(msg=='W'){ 
    debugln("Manually Setting Conveyor Belt 1 Direction: Reverse");
    Mod2_ConvBelt(2);
  }
  else if(msg=='E'){  
    debugln("Manually Stopping Conveyor Belt 1");
    Mod2_ConvBelt(0);
  }
  else if(msg=='M'){
    debugln("Changing Mode");
    if(Mode<1)
    {
      Mode++; 
    }
    else{
      Mode=0;
    } 
  }
  else if(msg=='i'){
    debugln("Infrared Sensor Status");
    if(infr <1)
    {
      infr++; 
      infp=true;
    }
    else{
      infr =0;
      infp=false;
    } 
  }
  else if(msg=='l'){
    debugln("Changing Mode");
    if(limr <1)
    {
      limr++; 
      limp=true;
    }
    else{
      limr =0;
      limp=false;
    } 
  }
  else if(msg=='0'){
   debugln("S1ping Base Rotation");
   Mod3_Base(0);
  }   
  else if(msg=='4'){
     debugln("Rotating Base to Start Position"); //<<--
     Mod3_Base(1);  
  }   
  else if(msg=='6'){
    debugln("Rotating Base to End Position"); //-->>
    Mod3_Base(2); 
  } 
  else if(msg=='5'){
     debugln("S1ping Lift Movement"); 
     Mod3_Gripper(0);
  }
  else if(msg=='2'){
     debugln("Lowering Down Arm to 0tom Position");
     Mod3_Gripper(1);
  }
  else if(msg=='8'){
     debugln("Lifting Up Arm to 1 Position");
     Mod3_Gripper(2);
  }
  else if(msg=='<'){
     debugln("Moving Platform to Start Position"); //<<--
     Mod3_Plat(0);  
  }
  else if(msg=='>'){
     debugln("Moving Platform to End Position"); //<<--
     Mod3_Plat(1);  
  }
  else if(msg=='C')
     process();
  else if(msg=='+'){
    debugln("Increasing Conveyor belt's Speed");
    m2.setSpeed(250);
  } 
  else if(msg=='-'){
    debugln("Decreasing Conveyor belt's Speed");
    m2.setSpeed(128);
  }
  else if(msg=='z'){
    while(digitalRead(Mod3_Gripper_1) == HIGH){
      Serial.println("Moving Lift To Top");
      m3.run(BACKWARD);
      }
      m3.run(RELEASE);  
      _delay_ms(250);
      while(digitalRead(Mod3_Plat_0) == HIGH){
        Serial.println("MOVING Platform TO START");
        m2.run(FORWARD);
      }
      m2.run(RELEASE);
      _delay_ms(200);
      while(digitalRead(Mod3_Rotating_Base_0) == HIGH){
        Serial.println("Rotating Base TO START");
        m4.run(FORWARD);
      }
      m4.run(RELEASE);  
      _delay_ms(200);
      /*PUSH PISTON */
      while(p==false){
        digitalWrite(Air_Compressor,HIGH);
        digitalWrite(Air_Valve_Gripper_Push,HIGH);
        digitalWrite(Air_Valve_Gripper_Rtrct,LOW);
        _delay_ms(1500);
        p=true;
      }
      _delay_ms(200);
      digitalWrite(Air_Compressor,LOW); 
      while(digitalRead(Mod3_Gripper_0) == HIGH){
        Serial.println("Moving Lift To 0");
        m3.run(FORWARD);
      }
      m3.run(RELEASE);  
     /*RETRACT PISTON*/ 
      while(r==false){
        digitalWrite(Air_Compressor,HIGH);
        digitalWrite(Air_Valve_Gripper_Push,LOW);
        digitalWrite(Air_Valve_Gripper_Rtrct,HIGH);  
        _delay_ms(1500);
        r=true;
      }
       _delay_ms(200);
      digitalWrite(Air_Compressor,LOW);  
      while(digitalRead(Mod3_Gripper_1) == HIGH){
        Serial.println("Moving Lift To Top");
        m3.run(BACKWARD);
      }
      m3.run(RELEASE);    
      _delay_ms(250);
      while(digitalRead(Mod3_Rotating_Base_1) == HIGH){
        Serial.println("Rotating Platform To END"); 
        m4.run(BACKWARD);
      }
      m4.run(RELEASE);  
      _delay_ms(250);
      while(digitalRead(Mod3_Gripper_0) == HIGH){
        Serial.println("Moving LIFT To 0");
        m3.run(FORWARD);
      } 
      m3.run(RELEASE);  
      _delay_ms(200);
      p=false;
      /*PUSH PISTON */
      while(p==false){
        digitalWrite(Air_Compressor,HIGH);
        digitalWrite(Air_Valve_Gripper_Push,HIGH);
        digitalWrite(Air_Valve_Gripper_Rtrct,LOW);
        _delay_ms(1500);
        p=true;
      }
      _delay_ms(200);
      digitalWrite(Air_Compressor,LOW);
      digitalWrite(Air_Valve_Gripper_Push,LOW);
      while(digitalRead(Mod3_Gripper_1) == HIGH){
        Serial.println("Moving Lift To Top");
        m3.run(BACKWARD);
      }
      m3.run(RELEASE);   
      debugln("DONE & DUNE");
      busy=false;
  }  
} 
//------------------------------------------------------------//   
