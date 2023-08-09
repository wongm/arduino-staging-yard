#include <LiquidCrystal.h>
#include <Arduino_Helpers.h> // Include the Arduino Helpers library.
#include <AH/Hardware/ExtendedInputOutput/ShiftRegisterOut.hpp>

using namespace ExtIO; // Bring the ExtIO pin functions into your sketch
 
const pin_t latchPin = 8; // Pin connected to ST_CP of 74HC595, green wire
const pin_t dataPin = 10;  // Pin connected to DS of 74HC595, blue wire
const pin_t clockPin = 9; // Pin connected to SH_CP of 74HC595, white wire

// Instantiate a shift register on the correct pins, most significant bit first,
// and a total of 8 outputs.
ShiftRegisterOut<16> sreg {dataPin, clockPin, latchPin, MSBFIRST};

// how many seconds to confirm train has exited?
// until this time expires, no new trains can exit
int EXIT_TIMER_SECONDS = 16;
// how many seconds to confirm train has arrived?
// controls how long between departing trains
int ARRIVAL_TIMER_SECONDS = 3;

// initialize the LCD with the interface pins
//LiquidCrystal lcd(13, 12, 11, 10, 9, 8);
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);


/*
 * 
// avoid pins 0 and 1  because they clatter on startup
int TRACK1_POWER = 2;
int TRACK2_POWER = 3;
int TRACK3_POWER = 4;
/*
int POINTS_MAIN = 5;
int POINTS_1 = 6;
int POINTS_2 = 7;
 */


// pins for track power relay
const pin_t TRACK1_POWER = sreg.pin(15);
const pin_t TRACK2_POWER = sreg.pin(14);
const pin_t TRACK3_POWER = sreg.pin(13);

// pins for relays with capacitors driving the single-coil solenoid points
const pin_t POINTS_MAIN = sreg.pin(12);
const pin_t POINTS_1 = sreg.pin(11);
const pin_t POINTS_2 = sreg.pin(10);

// signal outputs
const pin_t TRACK1_GO = sreg.pin(2);
const pin_t TRACK1_STOP = sreg.pin(3);
const pin_t TRACK2_GO = sreg.pin(4);
const pin_t TRACK2_STOP = sreg.pin(5);
const pin_t TRACK3_GO = sreg.pin(6);
const pin_t TRACK3_STOP = sreg.pin(7);

// pins for track sensor modules
const pin_t TRACK1_SENSOR = A3;
const pin_t TRACK2_SENSOR = A4;
const pin_t TRACK3_SENSOR = A5;

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

  sreg.begin();            // Initialize the shift registers
  
  printMessage("Booting...");
  
  // avoid clatter by setting status BEFORE enabling pin
  pinMode(TRACK1_POWER, OUTPUT);
  pinMode(TRACK2_POWER, OUTPUT);
  pinMode(TRACK3_POWER, OUTPUT);

  // turn off track power on boot up - relay is NOT energised
  digitalWrite(TRACK1_POWER, TRACK_OFF);
  digitalWrite(TRACK2_POWER, TRACK_OFF);
  digitalWrite(TRACK3_POWER, TRACK_OFF);

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

  // hold startup message
  delay(2000);

  // throw points backward then forward, 
  // so capacitors for Kato point motors are always ready to go
  printMessage("Points reverse...");
  digitalWrite(POINTS_MAIN, POINTS_REVERSE);
  digitalWrite(POINTS_1, POINTS_REVERSE);
  digitalWrite(POINTS_2, POINTS_REVERSE);
  delay(2000);
  
  printMessage("Points normal...");
  digitalWrite(POINTS_MAIN, POINTS_NORMAL);
  digitalWrite(POINTS_1, POINTS_NORMAL);
  digitalWrite(POINTS_2, POINTS_NORMAL);
  delay(2000);
  
  printMessage("Signals off...");
  digitalWrite(TRACK1_STOP, HIGH);
  digitalWrite(TRACK1_GO, LOW);
  digitalWrite(TRACK2_STOP, HIGH);
  digitalWrite(TRACK2_GO, LOW);
  digitalWrite(TRACK3_STOP, HIGH);
  digitalWrite(TRACK3_GO, LOW);
  delay(2000);
  
  printMessage("Track 1 signal...");
  digitalWrite(TRACK1_STOP, LOW);
  digitalWrite(TRACK1_GO, HIGH);
  delay(2000);
  
  printMessage("Track 2 signal...");
  digitalWrite(TRACK1_STOP, HIGH);
  digitalWrite(TRACK1_GO, LOW);
  digitalWrite(TRACK2_STOP, LOW);
  digitalWrite(TRACK2_GO, HIGH);
  delay(2000);
  
  printMessage("Track 3 signal...");
  digitalWrite(TRACK2_STOP, HIGH);
  digitalWrite(TRACK2_GO, LOW);
  digitalWrite(TRACK3_STOP, LOW);
  digitalWrite(TRACK3_GO, HIGH);
  delay(2000);
  
  digitalWrite(TRACK3_STOP, HIGH);
  digitalWrite(TRACK3_GO, LOW);
}

void initLCD()
{
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.createChar(0, backSlashChar);
}

void printMessageAction(String message)
{  
  /*
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
  */
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
      digitalWrite(TRACK1_GO, HIGH);
      digitalWrite(TRACK1_STOP, LOW);
      digitalWrite(TRACK1_POWER, TRACK_ON);
      digitalWrite(POINTS_MAIN, POINTS_REVERSE);
      digitalWrite(POINTS_1, POINTS_REVERSE);
      break;
      
    case 2:
      digitalWrite(TRACK2_GO, HIGH);
      digitalWrite(TRACK2_STOP, LOW);
      digitalWrite(TRACK2_POWER, TRACK_ON);
      digitalWrite(POINTS_MAIN, POINTS_REVERSE);
      digitalWrite(POINTS_1, POINTS_NORMAL);
      digitalWrite(POINTS_2, POINTS_REVERSE);
      break;
      
    case 3:
      digitalWrite(TRACK3_GO, HIGH);
      digitalWrite(TRACK3_STOP, LOW);
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
    digitalWrite(TRACK1_GO, LOW);
    digitalWrite(TRACK1_STOP, HIGH);
    digitalWrite(TRACK2_GO, LOW);
    digitalWrite(TRACK2_STOP, HIGH);
    digitalWrite(TRACK3_GO, LOW);
    digitalWrite(TRACK3_STOP, HIGH);
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
