#include <Servo.h>

int TRACK1_POWER = 0;
int TRACK2_POWER = 1;
int TRACK3_POWER = 2;

int TRACK1_POINTS = 4;
int TRACK2_POINTS = 5;
int TRACK3_POINTS = 6;

int TRACK1_SENSOR = A0;
int TRACK2_SENSOR = A1;
int TRACK3_SENSOR = A2;

Servo myservo1;
Servo myservo2;  // create servo object to control a servo

void setup() {

  // turn track power off
  pinMode(TRACK1_POWER,OUTPUT);
  pinMode(TRACK2_POWER,OUTPUT);
  pinMode(TRACK3_POWER,OUTPUT);
  digitalWrite(TRACK1_POWER,HIGH);
  digitalWrite(TRACK2_POWER,HIGH);
  digitalWrite(TRACK3_POWER,HIGH);

  // initilise points
  pinMode(TRACK1_POINTS,OUTPUT);
  pinMode(TRACK2_POINTS,OUTPUT);
  pinMode(TRACK3_POINTS,OUTPUT);
  digitalWrite(TRACK1_POINTS,HIGH);
  digitalWrite(TRACK2_POINTS,HIGH);
  digitalWrite(TRACK3_POINTS,HIGH);

  delay(500);
  
  digitalWrite(TRACK1_POINTS,LOW);
  digitalWrite(TRACK2_POINTS,LOW);
  digitalWrite(TRACK3_POINTS,LOW);
  
  
  myservo1.attach(0);  // attaches the servo on pin 9 to the servo object
  myservo2.attach(1);  // attaches the servo on pin 9 to the servo object

  // reset servo location  
  myservo1.write(180); 
  myservo2.write(180); 
  delay(500);
  
  myservo1.write(0); 
  myservo2.write(0); 
  delay(500);
    
  bool track1occupied = !digitalRead(A0);
  bool track2occupied = !digitalRead(A1);

  if (track1occupied)
  {
    trackGo1();    
  }
  else if (track2occupied)
  {
    trackGo2();    
  }  
}

void trackGo1()
{
  digitalWrite(TRACK1_POWER,LOW);
  myservo1.write(180); 
  delay(10000);  
  resetTracks();
}

void trackGo2()
{
  digitalWrite(TRACK2_POWER,LOW);
  myservo2.write(180); 
  delay(10000);  
  resetTracks();
}

void resetTracks()
{
  // turn power off
  digitalWrite(TRACK1_POWER,HIGH);
  digitalWrite(TRACK2_POWER,HIGH);
  
  myservo1.write(0);     
  myservo2.write(0); 
}

void loop() {

  bool track1occupied = !digitalRead(A0);
  bool track2occupied = !digitalRead(A1);
  
  // 1 = no trains
  // 0 = covered = train
  // Serial.print("track 1 = ");
  // Serial.println(track1occupied ? "occupied" : "clear");
  // Serial.print("track 2 = ");
  // Serial.println(track2occupied ? "occupied" : "clear");

  if (track1occupied && !track2occupied)
  {
    trackGo1();
  } 
  else if (track2occupied && !track1occupied)
  {
    trackGo2();
  }
  else
  {
   // Serial.println("Do nothing!");
    delay(1000);
  }
  

}
