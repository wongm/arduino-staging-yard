#include <Servo.h>

int TRACK1_POWER = 1;
int TRACK2_POWER = 2;
int TRACK3_POWER = 3;

int TRACK1_POINTS = 4;
int TRACK2_POINTS = 5;
int TRACK3_POINTS = 6;

int TRACK1_SENSOR = A1;
int TRACK2_SENSOR = A2;
int TRACK3_SENSOR = A3;

Servo myservo1;
Servo myservo2;  // create servo object to control a servo

void setup() {
Serial.begin(115200);
  Serial.println("booting!");
  // enable track power, boot up is HIGH with N/O relay contacts connected
  
  pinMode(TRACK1_POWER,OUTPUT);
  pinMode(TRACK2_POWER,OUTPUT);
  pinMode(TRACK3_POWER,OUTPUT);

  // initilise points boot up is HIGH with N/O relay contacts connected
  pinMode(TRACK1_POINTS,OUTPUT);
  pinMode(TRACK2_POINTS,OUTPUT);
  pinMode(TRACK3_POINTS,OUTPUT);
  
  pinMode(TRACK1_SENSOR,INPUT);
  pinMode(TRACK2_SENSOR,INPUT);
  pinMode(TRACK3_SENSOR,INPUT);


  // set points to known state
  delay(1000);    
  Serial.println("initialise points");
  digitalWrite(TRACK1_POINTS,HIGH);
  digitalWrite(TRACK2_POINTS,HIGH);
  digitalWrite(TRACK3_POINTS,HIGH);  
  delay(2000);   
  
  Serial.println("set points to known state");
  digitalWrite(TRACK1_POINTS,LOW);
  digitalWrite(TRACK2_POINTS,LOW);
  digitalWrite(TRACK3_POINTS,LOW);
  delay(1000);
}

void trackGo1()
{
  digitalWrite(TRACK1_POWER,LOW);
  digitalWrite(TRACK1_POINTS,LOW);
  delay(1000);  
  resetTracks();
}

void trackGo2()
{
  digitalWrite(TRACK2_POWER,LOW);
  digitalWrite(TRACK2_POINTS,LOW);
  delay(1000); 
  resetTracks();
}

void trackGo3()
{
  digitalWrite(TRACK3_POWER,LOW);
  digitalWrite(TRACK3_POINTS,LOW);
  delay(1000); 
  resetTracks();
}

void resetTracks()
{
   Serial.println("resetTracks");
   
  // turn power off
  digitalWrite(TRACK1_POWER,HIGH);
  digitalWrite(TRACK2_POWER,HIGH);
  digitalWrite(TRACK3_POWER,HIGH);
  
  digitalWrite(TRACK1_POINTS,HIGH);
  digitalWrite(TRACK2_POINTS,HIGH);
  digitalWrite(TRACK3_POINTS,HIGH);
}

void loop() {

  bool track1occupied = !digitalRead(TRACK1_SENSOR);
  bool track2occupied = !digitalRead(TRACK2_SENSOR);
  bool track3occupied = !digitalRead(TRACK3_SENSOR);

  
  // 1 = no trains
  // 0 = covered = train
   Serial.print("track 1 = ");
   Serial.println(track1occupied ? "occupied" : "clear");
   Serial.print("track 2 = ");
   Serial.println(track2occupied ? "occupied" : "clear");
   Serial.print("track 3 = ");
   Serial.println(track3occupied ? "occupied" : "clear");

  
  if (track1occupied)
  {
    trackGo1();    
  }
  else if (track2occupied)
  {
    trackGo2();    
  }
  else if (track3occupied)
  {
    trackGo3();    
  } 
  else
  {
    Serial.println("Do nothing!");
    delay(1000);
  }
  

}
