#include <LiquidCrystal.h>

// how many seconds to confirm train has exited?
int EXIT_TIMER_SECONDS = 8;
// how many seconds to confirm train has arrived?
int ARRIVAL_TIMER_SECONDS = 3;

// initialize the LCD with the interface pins
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

// pins for track power relay
// avoid pins 0 and 1  because they clatter on startup
int TRACK1_POWER = 2;
int TRACK2_POWER = 3;
int TRACK3_POWER = 4;

// pins for relays with capacitors driving the single-coil solenoid points
int POINTS_MAIN = 5;
int POINTS_1 = 6;
int POINTS_2 = 7;

// pins for track sensor modules
int TRACK1_SENSOR = A1;
int TRACK2_SENSOR = A2;
int TRACK3_SENSOR = A3;

int TRACK_OFF = HIGH;
int TRACK_ON = LOW;
int POINTS_NORMAL = LOW;
int POINTS_REVERSE = HIGH;

// special character since LCD doesn't have backslash
byte backSlashChar[8] = {
  0b00000,
  0b10000,
  0b01000,
  0b00100,
  0b00010,
  0b00001,
  0b00000,
  0b00000
};

// keep track of LCD message, only update if it changes
String currentMessage;

// for state machine
enum State_enum {NO_TRAINS, TRAIN_START, TRAIN_EXITING, CONFIRM_EXITED, TRAIN_EXITED, TRAIN_RUNNING, CONFIRM_ARRIVED, TRAIN_ARRIVED};

// save current system state in these
int state, track;

// various counters
int trackTimer, displayTimer, spinnyCounter, dotsCounter;

// current status of tracks
bool track1occupied, track2occupied, track3occupied;

void setup() 
{
  Serial.begin(115200);
  
  printMessage("Booting...");
  delay(500);

  // turn off track power on boot up - relay is NOT energised
  digitalWrite(TRACK1_POWER, TRACK_OFF);
  digitalWrite(TRACK2_POWER, TRACK_OFF);
  digitalWrite(TRACK3_POWER, TRACK_OFF);
  
  // avoid clatter by setting status BEFORE enabling pin
  pinMode(TRACK1_POWER, OUTPUT);
  pinMode(TRACK2_POWER, OUTPUT);
  pinMode(TRACK3_POWER, OUTPUT);

  state = NO_TRAINS;
  track = 0;
  trackTimer = 0;
  displayTimer = 0;
  spinnyCounter = 0;
  dotsCounter = 0;

  // initialise points boot up is HIGH with N/O relay contacts connected
  pinMode(POINTS_MAIN, OUTPUT);
  pinMode(POINTS_1, OUTPUT);
  pinMode(POINTS_2, OUTPUT);

  pinMode(TRACK1_SENSOR, INPUT);
  pinMode(TRACK2_SENSOR, INPUT);
  pinMode(TRACK3_SENSOR, INPUT);

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

void initLCD()
{
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.createChar(0, backSlashChar);
}

void printMessageAction(String message)
{  
  //add one or more dots at end of text, increasing over time
  for (int dots=0; dots <= (dotsCounter / 50); dots++)
  {
    message += String(".");
  }
  //and add trailing space, in case dots not shown
  for (int dots=0; dots <= 4 - (dotsCounter / 50); dots++)
  {
    message += String(" ");
  }
  printMessage(message, true);

  if (displayTimer % 200)
  {
    dotsCounter++;
  }
  
  if (dotsCounter > (3 * 50))
  {
    dotsCounter = 1;
  }
}

void printMessage(String message)
{
  printMessage(message, false);
}

void printMessage(String message, bool skipRefresh)
{    
  // LCD is slow - only refresh if needed
  if (currentMessage != message)
  {  
    if (!skipRefresh)
    {
      // restart LCD in case it's showing gibberish
      // (if noise on data lines, might cause issue with nibbles getting out of sync)
      initLCD();
    }
  
    lcd.setCursor(0, 0);
    lcd.print(message);

    Serial.println(message); 
    currentMessage = message;
  }
}

void trainStart(int startTrack)
{
  state = TRAIN_START;
  track = startTrack;
  
  printMessage(String("Start train #") + track + String("  "));
  
  switch (track)
  {
    case 1:
      digitalWrite(TRACK1_POWER, TRACK_ON);
      digitalWrite(POINTS_MAIN, POINTS_REVERSE);
      digitalWrite(POINTS_1, POINTS_REVERSE);
      break;
      
    case 2:
      digitalWrite(TRACK2_POWER, TRACK_ON);
      digitalWrite(POINTS_MAIN, POINTS_REVERSE);
      digitalWrite(POINTS_1, POINTS_NORMAL);
      digitalWrite(POINTS_2, POINTS_REVERSE);
      break;
      
    case 3:
      digitalWrite(TRACK3_POWER, TRACK_ON);
      digitalWrite(POINTS_MAIN, POINTS_REVERSE);
      digitalWrite(POINTS_1, POINTS_NORMAL);
      digitalWrite(POINTS_2, POINTS_NORMAL);
      break;
  }
}

void trainExiting()
{
  state = TRAIN_EXITING;
  trackTimer = 0;
}

void confirmExited()
{
  state = CONFIRM_EXITED;
    
  trackTimer += 1;

  //has train reappeared?
  if (track1occupied && track == 1)
  {
    trainExiting();
  }
  else if (track2occupied && track == 2)
  {
    trainExiting();
  }
  else if (track3occupied && track == 3)
  {
    trainExiting();
  }

  if (trackTimer > EXIT_TIMER_SECONDS)
  {
    trackTimer = 0;
    state = TRAIN_EXITED;
    
    printMessage(String("#") + track + String(" exited       "));
  }
  else
  {
    printMessage(String("#")+ track + String(" exit? ") + trackTimer + String(" sec  "));
  }
  delay(1000);
}

void confirmArrived()
{
  printMessage(String("#")+ track + String(" arrive? ") + trackTimer + String(" sec"));
  
  state = CONFIRM_ARRIVED;
  
  // turn yard power off ASAP as detection happens so points don't overrun
  digitalWrite(TRACK1_POWER, TRACK_OFF);
  digitalWrite(TRACK2_POWER, TRACK_OFF);
  digitalWrite(TRACK3_POWER, TRACK_OFF);

  // also change points back to mainline
  digitalWrite(POINTS_MAIN, POINTS_NORMAL);
  
  trackTimer += 1;

  if (trackTimer > ARRIVAL_TIMER_SECONDS)
  {
    trackTimer = 0;
    state = TRAIN_ARRIVED;    
  }
  delay(1000);
}

void trainRunning()
{
  state = TRAIN_RUNNING;
  
  printMessage(String("#") + track + String(" running      "));
  
  // turn yard power off
  //digitalWrite(TRACK1_POWER, TRACK_OFF);
  //digitalWrite(TRACK2_POWER, TRACK_OFF);
  //digitalWrite(TRACK3_POWER, TRACK_OFF);
  delay(1000);
}

void loop() 
{
  // 1 = no trains
  // 0 = covered = train
  track1occupied = !digitalRead(TRACK1_SENSOR);
  track2occupied = !digitalRead(TRACK2_SENSOR);
  track3occupied = !digitalRead(TRACK3_SENSOR);

  switch(state)
  {
    case NO_TRAINS:
    {
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
        printMessageAction("Stage trains");
      }
      break;
    }    
    case TRAIN_START:
    {
      trainExiting();      
      delay(1000);
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
        printMessage(String("#") + track + String(" still exiting"));
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
        printMessageAction(String("#") + track + String(" running"));
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
      printMessage(String("Train #") + track + String(" arrived"));
      
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

  displayTimer++;
  if (displayTimer % 50)
  {    
    String statusMessage = String("R1:") + (track1occupied ? "X" : "0") + String(" R2:") + (track2occupied ? "X" : "0") + String(" R3:") + (track3occupied ? "X" : "0");
    printStatus(statusMessage);
  }
  
  delay(10);
}

void printStatus(String statusMessage)
{
  Serial.println(String("Occupancy: ") + statusMessage);
  Serial.println(String("State = ") + state + String(". Running track = ") + track);
  
  lcd.setCursor(0, 1);
  lcd.print(statusMessage + " ");
  
  spinnyCounter = (displayTimer / 50);
  switch (spinnyCounter)
  {
    case 0:
      lcd.print("|");
      break;
    case 1:
      lcd.print("/");
      break;
    case 2:
      lcd.print("-");
      break;
    case 3:
      lcd.write(byte(0));
      break;
    case 4:
      spinnyCounter = 0;
      displayTimer = 0;
      break;
  }
}
