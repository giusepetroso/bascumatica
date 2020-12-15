
#include <EEPROM.h>
#include <FilterInput.h>
#include <AsyncPulse.h>
#include "bascumatica_config.h"

//PULSES
AsyncPulse statusLedPulseSlow(1000, 1000);
AsyncPulse statusLedPulseFast(300, 300);

//FILTER AND DELAYS
FilterInput fwdFilter(
  BTN_FWD_FIL_ON,
  BTN_FWD_FIL_OFF,
  false
);

FilterInput bkwFilter(
  BTN_BKW_FIL_ON,
  BTN_BKW_FIL_OFF,
  false
);

FilterInput fwdDelay(
  BTN_FWD_DLY_ON,
  BTN_FWD_DLY_OFF,
  false
);

FilterInput bkwDelay(
  BTN_BKW_DLY_ON,
  BTN_BKW_DLY_OFF,
  false
);

FilterInput switchDelay(
  SWITCH_TIME,
  100,
  false
);

FilterInput confirmDelay(
  CONFIRM_TIME,
  100,
  false
);

//INPUTS
const int I_BTN_FWD = 4;     // PULSANTE MARCIA / AVANTI CON TEACH
const int I_BTN_BKW = 5;     // PULSANTE INDIETRO CON TEACH


//OUTPUTS
const int O_CMD_FWD = 11;     // COMANDO AVANTI
const int O_CMD_BKW = 12;     // COMANDO INDIETRO
const int O_LMP_STATE = 13;     // LED STATO

//MODES
enum modes {
  AUTO,
  TEACH
};
modes mode;
modes lastMode;

//TEACH MODE
byte teachStep;
bool teachStepAntirep;

//AUTO MODE
bool autoStart = 0;
bool autoStartAntirep;

//ENCODER 
bool sig1 = false;
bool sig2 = false;

int state = 0;
int encoder = 0;
int fwdLimit = 0;
int bkwLimit = 0;

struct SaveState {
  int state; //closed=0 | open=1
  int encoder;
  int fwdLimit;
  int bkwLimit;
};

//MISC VARS
unsigned long MILLIS;

void setup() {
  //SETUP INTERRUPT PINS
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), sig1Handler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(3), sig2Handler, CHANGE);
  
  //SETUP INPUT PINS
  pinMode(I_BTN_FWD, INPUT);
  pinMode(I_BTN_BKW, INPUT);

  //SETUP OUTPUT PINS
  pinMode(O_CMD_FWD, OUTPUT);
  pinMode(O_CMD_BKW, OUTPUT);
  pinMode(O_LMP_STATE, OUTPUT);
    
  //INIT MODE
  mode = AUTO;
  //mode = TEACH;
  
  //GET STORED VALUES
  SaveState savedState;
  EEPROM.get(0,savedState);
  
  //INIT ENCODER AND LIMITS
  state = savedState.state;
  encoder = savedState.encoder;
  fwdLimit = savedState.fwdLimit;
  bkwLimit = savedState.bkwLimit;
  
  //INIT SERIAL (DEBUG)
  Serial.begin(9600);
}

void sig1Handler(){
  sig1 = digitalRead(2);

  if(sig1==1){
    if(sig2==1){
      encoder++;
    } else {
      encoder--;
    }
  }
}

void sig2Handler(){
  sig2 = digitalRead(3);
}

//########################################################
// INPUT MANAGEMENT
//########################################################
void input_management(int& fwdStart, int& bkwStart, int& switchCmd, int& confirmCmd){
  int fwdFiltered = fwdFilter.watch(digitalRead(I_BTN_FWD));
  int bkwFiltered = bkwFilter.watch(digitalRead(I_BTN_BKW));
  
  //start forward command
  int fwdStartEnable = bkwFiltered==0 ? fwdFiltered : 0;
  fwdStart = fwdDelay.watch(fwdStartEnable);
  
  //start backward command
  int bkwStartEnable = fwdFiltered==0 ? bkwFiltered : 0;
  bkwStart = bkwDelay.watch(bkwStartEnable);
  
  //switch command
  int switchCmdEnable = fwdFiltered==1 && bkwFiltered==1 ? 1 : 0;
  switchCmd = switchDelay.watch(switchCmdEnable);

  int confirmCmdEnable = (switchCmdEnable && mode==TEACH) ? 1 : 0;
  confirmCmd = confirmDelay.watch(confirmCmdEnable);
}

//########################################################
// SWITCH MODE
//########################################################
void handle_mode(int switchCmd){
  if(mode==AUTO){
    digitalWrite(O_LMP_STATE,LOW);
    if(switchCmd==1 && lastMode==TEACH) mode=TEACH;
    if(switchCmd==0) lastMode=TEACH;
    teachStep=0;
  } else {
    if(teachStep == 0) digitalWrite(O_LMP_STATE,statusLedPulseSlow.get());
    if(teachStep == 1) digitalWrite(O_LMP_STATE,statusLedPulseFast.get());
    if(switchCmd==1 && lastMode==AUTO) mode=AUTO;
    if(switchCmd==0) lastMode=AUTO;
  }
}

//########################################################
// AUTO LOOP
//########################################################
void auto_loop(int fwdStart, int bkwStart){  
  //Serial.println(fwdStart);
  
  if(autoStart && state == 0){
    if(encoder <= fwdLimit){
      digitalWrite(O_CMD_FWD,HIGH);
      digitalWrite(O_CMD_BKW,LOW);
    } else {
      autoStart = 0;
      state = 1;
    }
  }
  
  if(autoStart && state == 1){
    if(encoder >= bkwLimit){
      digitalWrite(O_CMD_FWD,LOW);
      digitalWrite(O_CMD_BKW,HIGH);
    } else {
      autoStart = 0;
      state = 0;
    }
  } 

  if(!autoStart) {
    digitalWrite(O_CMD_FWD,LOW);
    digitalWrite(O_CMD_BKW,LOW);
  }

  //step progression
  if(fwdStart){
    if(!autoStartAntirep){  
      //switch the state of start cmd
      autoStart = !autoStart;

      //save state
      SaveState savedState {
        state, //closed state
        encoder,
        fwdLimit,
        bkwLimit  
      };
      EEPROM.put(0,savedState);

      //antirepetitive
      autoStartAntirep = true;
    }
  } else {
    autoStartAntirep = false;
  }
}

//########################################################
// TEACH LOOP
//########################################################
void teach_loop(int fwdStart, int bkwStart, int confirmCmd){
  if(false){
    Serial.print("S: ");
    Serial.println(teachStep);
  }
 
  //move motor
  if(fwdStart){
    digitalWrite(O_CMD_FWD,HIGH);
    digitalWrite(O_CMD_BKW,LOW);
  } else if(bkwStart){
    digitalWrite(O_CMD_FWD,LOW);
    digitalWrite(O_CMD_BKW,HIGH);
  } else {
    digitalWrite(O_CMD_FWD,LOW);
    digitalWrite(O_CMD_BKW,LOW);
  }

  //step progression
  if(confirmCmd){
    if(!teachStepAntirep){
      teachStep++;

      //set the first limit
      if(teachStep==1) {
        //set first limit
        fwdLimit = encoder;
      }

      //set the second limit, save and change mode
      if(teachStep==2) {
        //set second limit
        bkwLimit = encoder;

        //save
        SaveState savedState {
          0, //closed state
          encoder,
          fwdLimit,
          bkwLimit  
        };
        EEPROM.put(0,savedState);

        //change mode
        mode=AUTO;
      }

      //antirepetitive
      teachStepAntirep = true;
    }
  } else {
    teachStepAntirep = false;
  }
}

//########################################################
// MAIN LOOP
//########################################################
void loop() { 
  //TAKE MILLIS
  MILLIS = millis();

  if(true){
    Serial.print("ENC: ");
    Serial.print(encoder);
    Serial.print(" | FWD LIMIT: ");
    Serial.print(fwdLimit);
    Serial.print(" | BKW LIMIT: ");
    Serial.println(bkwLimit);
  }
  
  //INPUT MANAGEMENT
  int fwdStart, bkwStart, switchCmd, confirmCmd;
  input_management(fwdStart, bkwStart, switchCmd, confirmCmd);

  //HANDLE MODE
  handle_mode(switchCmd);
 
  //RUN ACTUAL MODE
  if(mode == AUTO){
    auto_loop(fwdStart, bkwStart);
  } else {
    teach_loop(fwdStart, bkwStart, confirmCmd);
  }
}
