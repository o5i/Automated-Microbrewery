/*
   Temp = sensor.read*100
   Read Write Modbus registers test
*/

#include <OneWire.h>              //http://playground.arduino.cc/Learning/OneWire
#include <PID_v1.h>               //http://playground.arduino.cc/Code/PIDLibrary
#include <SimpleModbusSlave.h>    //http://forum.arduino.cc/index.php?topic=176142.new#new https://drive.google.com/folderview?id=0B0B286tJkafVSENVcU1RQVBfSzg&usp=drive_web'
#include <elapsedMillis.h>        //http://playground.arduino.cc/Code/ElapsedMillis

// STUFF
#define OUT_PUMP 11
#define OUT_STIRRER 12
#define OUT_GASV_1_A 13         // Regulation Valve
#define OUT_GASV_1_D 14         // On Off Valve
#define OUT_GASV_2_A 15         // Regulation Valve
#define OUT_GASV_2_D 16         // On Off Valve
// ONEWIRE
#define SENSORS 3               // Number of Sensors
// MODBUS
#define MOD_REGS 2000
#define MOD_COM Serial1
#define MOD_BAUD 57600
unsigned int  MOD_REG[MOD_REGS];

// USER VARIABLE
int S_AUTO_MAN = 0;
int S_MAN_ON = 0;
int S_AUTO_ON = 0;                                              // 1 = ON, 2 = 0FF
int S_AUTO_CYCLE = 0;
int S_TEMP_DIFF = 1;                                            // Temperature Hysteresis between on off
int S_AUTO_SETTING[8][20] = {0};                                      // 0 = Autostart Cycle, 1 = Pump, 2 = Stirrer, 3 = Gasv_1 on off, 4 = Gasv_2 on off, 5 = Target Temp_1, 6 = Target Temp 2 , 7 = Time Cycle)
int S_MAN_SETTING[9] = {0};                                          // 0 = Pump, 1 = Stirrer, 2 = Gasv_1 on off, 3 = Gasv_2 on off, 4 = Gasv_1 pwr, 5 = Gasv_2 pwr, 6 = Gasv_1 temp/pwr, 7 = Gasv_2 temp/pwr, 8 = Target Temp_1, 9 = Target Temp 2)
int S_AUTO_RESET = 0;
int S_MAN_RESET = 0;


// USER READONLY
int TEMP_1;
int TEMP_2;


// PID
double PID_SET_1, PID_IN_1, PID_OUT_1;
double PID_SET_2, PID_IN_2, PID_OUT_2;
PID PID1(&PID_IN_1, &PID_OUT_1, &PID_SET_1, 2, 5, 1, DIRECT);
PID PID2(&PID_IN_2, &PID_OUT_2, &PID_SET_2, 2, 5, 1, DIRECT);



elapsedMillis TIME_ELAPSED;
OneWire ONEWIRE_BUS(10);


void setup() {
  modbus_configure(&MOD_COM, MOD_BAUD, SERIAL_8N2, 1, 2, MOD_REGS, MOD_REG);
  modbus_update_comms(MOD_BAUD, SERIAL_8N2, 1);
  pinMode(OUT_PUMP, OUTPUT);
  pinMode(OUT_STIRRER, OUTPUT);
  pinMode(OUT_GASV_1_D, OUTPUT);
  pinMode(OUT_GASV_2_D, OUTPUT);
  pinMode(OUT_GASV_1_A, OUTPUT);
  pinMode(OUT_GASV_2_A, OUTPUT);
  digitalWrite(OUT_PUMP, LOW);
  digitalWrite(OUT_STIRRER, LOW);
  digitalWrite(OUT_GASV_1_D, LOW);
  digitalWrite(OUT_GASV_2_D, LOW);
  analogWrite(OUT_GASV_1_A, 0);
  analogWrite(OUT_GASV_2_A, 0);
  memset(S_MAN_SETTING, 0, sizeof(S_MAN_SETTING));
}

void loop() {




  // Modbus
  modbus_update();

  // Master to Slave
  S_AUTO_MAN = MOD_REG[0];
  S_MAN_ON = MOD_REG[1];
  S_AUTO_ON = MOD_REG[2];
  S_AUTO_CYCLE = MOD_REG[3];
  S_TEMP_DIFF = MOD_REG[4];
  S_AUTO_RESET = MOD_REG[5];
  S_MAN_RESET = MOD_REG[6];
  for (int i = 0; i < 9; i++) {
    int offset_;
    offset_ = 50;

    S_MAN_SETTING[i] = MOD_REG[offset_ + i];
  }
  for (int i = 0; i < 8; i++) {
    int offset_;
    offset_ = 100;
    for (int j = 0; j < 20; i++) {
      S_AUTO_SETTING[i][j] = MOD_REG[offset_ + i * j];
    }
  }
  






  if (S_AUTO_MAN) {
    S_MAN_ON = 0;
  } else {
    S_AUTO_ON = 0;
  }

  if (S_AUTO_RESET) {
    memset(S_AUTO_SETTING, 0, sizeof(S_AUTO_SETTING));
    S_AUTO_CYCLE = 0;
    S_AUTO_ON = 0;
    S_AUTO_RESET = 0;
  }
  if (S_MAN_RESET) {
    memset(S_MAN_SETTING, 0, sizeof(S_MAN_SETTING));
    S_MAN_ON = 0;
    S_AUTO_RESET = 0;
  }

  /////////////////////////////////////
  ////////////// AUTO /////////////////

  if (S_AUTO_ON) {
    if (TIME_ELAPSED >= S_AUTO_SETTING[7][S_AUTO_CYCLE] * 60000) {
      S_AUTO_CYCLE++;
      if (S_AUTO_SETTING[7][S_AUTO_CYCLE]) {
        S_AUTO_ON = 0;
      }
    }
    if (S_AUTO_SETTING[1][S_AUTO_CYCLE]) {
      digitalWrite(OUT_PUMP, HIGH);
    }
    else {
      digitalWrite(OUT_PUMP, LOW);
    }
    if (S_AUTO_SETTING[2][S_AUTO_CYCLE]) {
      digitalWrite(OUT_STIRRER, HIGH);
    }
    else {
      digitalWrite(OUT_STIRRER, LOW);
    }
    if (S_AUTO_SETTING[3][S_AUTO_CYCLE]) {
      if (TEMP_1 - S_TEMP_DIFF <= S_AUTO_SETTING[5][S_AUTO_CYCLE]) {
        digitalWrite(OUT_GASV_1_D, HIGH);
        PID_IN_1 = TEMP_1;
        PID_SET_1 = S_AUTO_SETTING[5][S_AUTO_CYCLE];
        PID1.Compute();
        analogWrite(OUT_GASV_1_A, PID_OUT_1);
      }
      else {
        digitalWrite(OUT_GASV_1_D, LOW);
        analogWrite(OUT_GASV_1_A, 0);
      }
    }
    else {
      digitalWrite(OUT_GASV_1_D, LOW);
      analogWrite(OUT_GASV_1_A, 0);
    }
    if (S_AUTO_SETTING[4][S_AUTO_CYCLE])  {
      if (TEMP_2 - S_TEMP_DIFF <= S_AUTO_SETTING[6][S_AUTO_CYCLE]) {
        digitalWrite(OUT_GASV_2_D, HIGH);
        PID_IN_2 = TEMP_2;
        PID_SET_2 = S_AUTO_SETTING[6][S_AUTO_CYCLE];
        PID2.Compute();
        analogWrite(OUT_GASV_2_A, PID_OUT_2);
      }
      else {
        digitalWrite(OUT_GASV_2_D, LOW);
        analogWrite(OUT_GASV_2_A, 0);
      }
    }
    else {
      digitalWrite(OUT_GASV_2_D, LOW);
      analogWrite(OUT_GASV_2_A, 0);
    }
  }
  else {
    TIME_ELAPSED = 0;
  }



  /////////////////////////////////////
  ////////////// MAN //////////////////

  if (S_MAN_ON) {
    if (S_MAN_SETTING[0]) {
      digitalWrite(OUT_PUMP, HIGH);
    }
    else {
      digitalWrite(OUT_PUMP, LOW);
    }
    if (S_MAN_SETTING[1]) {
      digitalWrite(OUT_STIRRER, HIGH);
    }
    else {
      digitalWrite(OUT_STIRRER, LOW);
    }
    if (S_MAN_SETTING[2]) {
      if (S_MAN_SETTING[6]) {
        if (TEMP_1 - S_TEMP_DIFF <= S_MAN_SETTING[8]) {
          digitalWrite(OUT_GASV_1_D, HIGH);
          PID_IN_1 = TEMP_1;
          PID_SET_1 = S_MAN_SETTING[8];
          PID1.Compute();
          analogWrite(OUT_GASV_1_A, PID_OUT_1);
        }
        else {
          digitalWrite(OUT_GASV_1_D, LOW);
          analogWrite(OUT_GASV_1_A, 0);
        }
      }
      else {
        digitalWrite(OUT_GASV_1_D, HIGH);
        analogWrite(OUT_GASV_1_A, S_MAN_SETTING[4]);
      }
    }
    else {
      digitalWrite(OUT_GASV_1_D, LOW);
      analogWrite(OUT_GASV_1_A, 0);
    }
    if (S_MAN_SETTING[3]) {
      if (S_MAN_SETTING[7]) {
        if (TEMP_2 - S_TEMP_DIFF <= S_MAN_SETTING[9]) {
          digitalWrite(OUT_GASV_2_D, HIGH);
          PID_IN_2 = TEMP_2;
          PID_SET_2 = S_MAN_SETTING[9];
          PID2.Compute();
          analogWrite(OUT_GASV_2_A, PID_OUT_2);
        }
        else {
          digitalWrite(OUT_GASV_2_D, LOW);
          analogWrite(OUT_GASV_2_A, 0);
        }
      }
      else {
        digitalWrite(OUT_GASV_2_D, HIGH);
        analogWrite(OUT_GASV_2_A, S_MAN_SETTING[5]);
      }
    }
    else {
      digitalWrite(OUT_GASV_2_D, LOW);
      analogWrite(OUT_GASV_2_A, 0);
    }
  }


  ///////////////////////////////////
  ///////// ALL OFF ////////////////

  else {
    digitalWrite(OUT_PUMP, LOW);
    digitalWrite(OUT_STIRRER, LOW);
    digitalWrite(OUT_GASV_1_D, LOW);
    digitalWrite(OUT_GASV_2_D, LOW);
    analogWrite(OUT_GASV_1_A, 0);
    analogWrite(OUT_GASV_2_A, 0);
  }
}




















