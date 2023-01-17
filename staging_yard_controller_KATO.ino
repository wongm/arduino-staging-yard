int TRACK1_POWER = 1;
int TRACK2_POWER = 2;
int TRACK3_POWER = 3;

int POINTS_MAIN = 4;
int POINTS_1 = 5;
int POINTS_2 = 6;

int TRACK1_SENSOR = A1;
int TRACK2_SENSOR = A2;
int TRACK3_SENSOR = A3;

enum State_enum {NO_TRAINS, TRAIN_START, TRAIN_EXITING, CONFIRM_EXITED, TRAIN_EXITED, TRAIN_RUNNING, CONFIRM_ARRIVED, TRAIN_ARRIVED};

int state;
int track;
int timer;

void setup() {
  Serial.begin(115200);
  Serial.println("booting!");
  // enable track power, boot up is HIGH with N/O relay contacts connected

  pinMode(TRACK1_POWER,OUTPUT);
  pinMode(TRACK2_POWER,OUTPUT);
  pinMode(TRACK3_POWER,OUTPUT);

  state = NO_TRAINS;
  track = 0;
  timer = 0;

  // initialise points boot up is HIGH with N/O relay contacts connected
  pinMode(POINTS_MAIN,OUTPUT);
  pinMode(POINTS_1,OUTPUT);
  pinMode(POINTS_2,OUTPUT);

  pinMode(TRACK1_SENSOR,INPUT);
  pinMode(TRACK2_SENSOR,INPUT);
  pinMode(TRACK3_SENSOR,INPUT);

  /*
  // set points to known state
  delay(1000);
  Serial.println("initialise points");
  digitalWrite(POINTS_MAIN,HIGH);
  digitalWrite(POINTS_1,HIGH);
  digitalWrite(POINTS_2,HIGH);
  delay(2000);

  Serial.println("set points to known state");
  digitalWrite(POINTS_MAIN,LOW);
  digitalWrite(POINTS_1,LOW);
  digitalWrite(POINTS_2,LOW);
  delay(1000);
  */
}

void trainStart(int startTrack)
{
  state = TRAIN_START;
  track = startTrack;
  
  Serial.println(String("STATUS: Start train ") + track);
  
  switch (track)
  {
    case 1:
      digitalWrite(TRACK1_POWER,LOW);
      digitalWrite(POINTS_MAIN,HIGH);
      digitalWrite(POINTS_1,HIGH);
      break;
      
    case 2:
      digitalWrite(TRACK2_POWER,LOW);
      digitalWrite(POINTS_MAIN,HIGH);
      digitalWrite(POINTS_1,LOW);
      digitalWrite(POINTS_2,HIGH);
      break;
      
    case 3:
      digitalWrite(TRACK3_POWER,LOW);
      digitalWrite(POINTS_MAIN,HIGH);
      digitalWrite(POINTS_1,LOW);
      digitalWrite(POINTS_2,LOW);
      break;
  }
  
  trainExiting();
}

void trainExiting()
{
  state = TRAIN_EXITING;
  delay(2000);
}

void confirmExited()
{
  
  state = CONFIRM_EXITED;
    
  timer += 1;

  if (timer > 3)
  {
    timer = 0;
    state = TRAIN_EXITED;
    
    Serial.println(String("STATUS: train ") + track + String(" exited yard"));
  }
  else
  {
    Serial.println(String("STATUS: confirm train ")+ track + String(" exit: ") + timer + String(" sec..."));
  }
  delay(1000);
}

void confirmArrived()
{
    Serial.println(String("STATUS: confirm train ")+ track + String(" arrival: ") + timer + String(" sec..."));
  
  state = CONFIRM_ARRIVED;
  
  // turn yard power off ASAP as detection happens so points don't overrun
  digitalWrite(TRACK1_POWER,HIGH);
  digitalWrite(TRACK2_POWER,HIGH);
  digitalWrite(TRACK3_POWER,HIGH);

  // also change points back to mainline
      digitalWrite(POINTS_MAIN,LOW);
  
  timer += 1;

  if (timer > 3)
  {
    timer = 0;
    state = TRAIN_ARRIVED;
    
  }
  delay(1000);
}

void trainRunning()
{
  state = TRAIN_RUNNING;
  
  Serial.println(String("STATUS: train ") + track + String(" running"));
  
  // turn yard power off
  digitalWrite(TRACK1_POWER,HIGH);
  digitalWrite(TRACK2_POWER,HIGH);
  digitalWrite(TRACK3_POWER,HIGH);
  delay(1000);
}

void loop() 
{
  bool track1occupied = !digitalRead(TRACK1_SENSOR);
  bool track2occupied = !digitalRead(TRACK2_SENSOR);
  bool track3occupied = !digitalRead(TRACK3_SENSOR);

  // 1 = no trains
  // 0 = covered = train
  Serial.print("Occupancy: 1=");
  Serial.print(track1occupied ? "X" : "0");
  Serial.print(", 2=");
  Serial.print(track2occupied ? "X" : "0");
  Serial.print(", 3=");
  Serial.print(track3occupied ? "X" : "0");

  Serial.println(String(". State = ") + state + String(". Running track = ") + track);

  switch(state)
  {
    case NO_TRAINS:
    {
      Serial.println("STATUS: no trains running");
      if ((track1occupied && track2occupied && track3occupied) || (track1occupied && track2occupied && !track3occupied) || (track1occupied && !track2occupied && !track3occupied))
      {
        trainStart(1);
      }
      else if ((track2occupied && track3occupied) || (!track1occupied && track2occupied))
      {
        trainStart(2);
      }
      else if (track3occupied)
      {
        trainStart(3);
      }
      else
      {
        Serial.println("Waiting for trains...");
        delay(2000);
      }
      break;
    }    
    case TRAIN_EXITING:
    {
      if (!track1occupied && track == 1)
      {
        confirmExited();
      }
      else if (!track2occupied && track == 2)
      {
        confirmExited();
      }
      else if (!track3occupied && track == 3)
      {
        confirmExited();
      }
      else
      {
        Serial.println(String("STATUS: train ") + track + String(" still exiting yard"));
      }
      break;
    }
    case CONFIRM_EXITED:
    {
      confirmExited();
      break;
    }
    case TRAIN_EXITED:
    {
      trainRunning();
      break;
    } 
    case TRAIN_RUNNING:
    {
      if (track1occupied && track == 1)
      {
        confirmArrived();
      }
      else if (track2occupied && track == 2)
      {
        confirmArrived();
      }
      else if (track3occupied && track == 3)
      {
        confirmArrived();
      }
      else
      {
        Serial.println(String("STATUS: train ") + track + String(" still running..."));
      }
      break;
    }
    case CONFIRM_ARRIVED:
    {
      confirmArrived();
      break;
    }
    case TRAIN_ARRIVED:
    {
      Serial.println(String("STATUS: train ") + track + String(" arrived"));
      
      switch (track)
      {
        case 1:
        {
          if (track2occupied)
          {
            trainStart(2);
          }
          else if (track3occupied)
          {
            trainStart(3);
          }
          break;
        }
        case 2:
        {
          if (track3occupied)
          {
            trainStart(3);
          }
          else if (track1occupied)
          {
            trainStart(1);
          }
          break;
        }
        case 3:
        {
          if (track1occupied)
          {
            trainStart(1);
          }
          else if (track2occupied)
          {
            trainStart(2);
          }
          break;
        }
      }
    }
  }
  delay(10);
}
