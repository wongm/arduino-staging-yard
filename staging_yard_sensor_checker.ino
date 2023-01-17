
int TRACK1_SENSOR = A1;
int TRACK2_SENSOR = A2;
int TRACK3_SENSOR = A3;

void setup() {
  // put your setup code here, to run once:

Serial.begin(115200);
int TRACK1_SENSOR = A1;
int TRACK2_SENSOR = A2;
int TRACK3_SENSOR = A3;
  pinMode(TRACK1_SENSOR,INPUT_PULLUP);
  pinMode(TRACK2_SENSOR,INPUT_PULLUP);
  pinMode(TRACK3_SENSOR,INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  
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
    delay(1000);

}
