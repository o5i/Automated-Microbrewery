/*
   Temp = sensor.read*100
   Read Write Modbus registers test
*/

#include <PID_v1.h>               //http://playground.arduino.cc/Code/PIDLibrary
#include <SimpleModbusSlave.h>    //http://forum.arduino.cc/index.php?topic=176142.new#new https://drive.google.com/folderview?id=0B0B286tJkafVSENVcU1RQVBfSzg&usp=drive_web'
#include <elapsedMillis.h>        //http://playground.arduino.cc/Code/ElapsedMillis
#include <OneWire.h>              //http://playground.arduino.cc/Learning/OneWire
#include <DallasTemperature.h>

// STUFF
#define OUT_PUMP 11
#define OUT_STIRRER 12
#define OUT_GASV_1_A 13         // Regulation Valve
#define OUT_GASV_1_D 14         // On Off Valve
#define OUT_GASV_2_A 15         // Regulation Valve
#define OUT_GASV_2_D 16         // On Off Valve

// ONEWIRE
#define BUS_PIN 10
#define TEMPERATURE_PRECISION 10
OneWire ONEWIRE_BUS(BUS_PIN);
DallasTemperature DS18B20(&ONEWIRE_BUS);
DeviceAddress DS18B20_1, DS18B20_2, DS18B20_3;

// MODBUS
#define MOD_REGS 2000
#define MOD_COM Serial1
#define MOD_BAUD 57600
unsigned int MOD_REG[MOD_REGS];

// PID
double PID_SET_1, PID_IN_1, PID_OUT_1;
double PID_SET_2, PID_IN_2, PID_OUT_2;
PID PID1(&PID_IN_1, &PID_OUT_1, &PID_SET_1, 2, 5, 1, DIRECT);
PID PID2(&PID_IN_2, &PID_OUT_2, &PID_SET_2, 2, 5, 1, DIRECT);

elapsedMillis TIME_ELAPSED;

// USER VARIABLE
// READ WRITE
// MOD_REG[0]                       // AUTO_MAN
// MOD_REG[1]                       // MAN_ON
// MOD_REG[2]                       // AUTO_ON
// MOD_REG[3]                       // Cycle
// MASTER TO SLAVE
const unsigned int S_TEMP_DIFF = 200;              // Temperature Hysteresis between on off
unsigned int S_AUTO_SETTING[8][20] = {0};    // 0 = Autostart Cycle, 1 = Pump, 2 = Stirrer, 3 = Gasv_1 on off, 4 = Gasv_2 on off, 5 = Target Temp_1, 6 = Target Temp 2 , 7 = Time Cycle)
unsigned int S_MAN_SETTING[9] = {0};         // 0 = Pump, 1 = Stirrer, 2 = Gasv_1 on off, 3 = Gasv_2 on off, 4 = Gasv_1 Manual Power 0-100, 5 = Gasv_2 Manual Power 0-100, 6 = Gasv_1 temp/pwr, 7 = Gasv_2 temp/pwr, 8 = Target Temp_1, 9 = Target Temp 2)
// SLAVE TO MASTER
unsigned int TEMP_1;
unsigned int TEMP_2;
unsigned int TEMP_3;

void setup() {
  DS18B20.begin();
  while (DS18B20.getDeviceCount() < 3) {
    // Stop if no Sensors available
  }
  DS18B20.setResolution(DS18B20_1, TEMPERATURE_PRECISION);
  DS18B20.setResolution(DS18B20_2, TEMPERATURE_PRECISION);
  DS18B20.setResolution(DS18B20_3, TEMPERATURE_PRECISION);
  modbus_configure(&MOD_COM, MOD_BAUD, SERIAL_8N2, 1, 2, MOD_REGS, MOD_REG);
  modbus_update_comms(MOD_BAUD, SERIAL_8N2, 1);
  PID1.SetOutputLimits(0, 1024);
  PID2.SetOutputLimits(0, 1024);
  pinMode(OUT_PUMP, OUTPUT);
  pinMode(OUT_STIRRER, OUTPUT);
  pinMode(OUT_GASV_1_D, OUTPUT);
  pinMode(OUT_GASV_2_D, OUTPUT);
  analogWriteResolution(10);
  pinMode(OUT_GASV_1_A, OUTPUT);
  analogWriteFrequency(OUT_GASV_1_A, 22000);
  pinMode(OUT_GASV_2_A, OUTPUT);
  analogWriteFrequency(OUT_GASV_2_A, 22000);
  PUMP_ONOFF(0);
  STIRRER_ONOFF(0);
  GASV_1_D_ONOFF(0);
  GASV_2_D_ONOFF(0);
  analogWrite(OUT_GASV_1_A, 0);
  analogWrite(OUT_GASV_2_A, 0);
  memset(MOD_REG, 0, sizeof(MOD_REG));
}

void loop() {
  DS18B20.requestTemperatures();
  TEMP_1 = DS18B20.getTempC(DS18B20_1) * 100;
  TEMP_2 = DS18B20.getTempC(DS18B20_2) * 100;
  TEMP_3 = DS18B20.getTempC(DS18B20_3) * 100;

  // Modbus
  modbus_update();


  // Master to Slave
  for (int i = 0; i < 9; i++) {
    int offset_;
    offset_ = 100;
    S_MAN_SETTING[i] = MOD_REG[offset_ + i];
  }
  for (int i = 0; i < 8; i++) {
    int offset_;
    offset_ = 200;
    for (int j = 0; j < 20; i++) {
      S_AUTO_SETTING[i][j] = MOD_REG[offset_ + i * j];
    }
  }

  // Slave to Master
  MOD_REG[300] = TEMP_1;
  MOD_REG[301] = TEMP_2;
  MOD_REG[301] = TEMP_3;



  if (MOD_REG[0]) {                                         // Man or auto
    MOD_REG[1] = 0;
  } else {
    MOD_REG[2] = 0;
  }

  ///////////////////////////////////
  ///////// ALL OFF ////////////////

  if ((!MOD_REG[1]) || (!MOD_REG[2])) {
    PUMP_ONOFF(0);
    STIRRER_ONOFF(0);
    GASV_1_D_ONOFF(0);
    GASV_2_D_ONOFF(0);
    analogWrite(OUT_GASV_1_A, 0);
    analogWrite(OUT_GASV_2_A, 0);
  }

  /////////////////////////////////////
  ////////////// AUTO /////////////////

  if (MOD_REG[2]) {
    if (TIME_ELAPSED >= S_AUTO_SETTING[7][MOD_REG[3]] * 60000) {
      MOD_REG[3]++;
      if (S_AUTO_SETTING[7][MOD_REG[3]]) {
        MOD_REG[2] = 0;
      }
    }
    if (S_AUTO_SETTING[1][MOD_REG[3]]) {
      PUMP_ONOFF(1);
    } else {
      PUMP_ONOFF(0);
    }
    if (S_AUTO_SETTING[2][MOD_REG[3]]) {
      STIRRER_ONOFF(1);
    }    else {
      STIRRER_ONOFF(0);
    }
    if (S_AUTO_SETTING[3][MOD_REG[3]]) {
      PID_IN_1 = TEMP_1;
      PID_SET_1 = S_AUTO_SETTING[5][MOD_REG[3]];
      PID1.Compute();
      analogWrite(OUT_GASV_1_A, PID_OUT_1);
      if (TEMP_1 - S_TEMP_DIFF <= S_AUTO_SETTING[5][MOD_REG[3]]) {
        GASV_1_D_ONOFF(1);
      } else if (TEMP_1 + S_TEMP_DIFF >= S_AUTO_SETTING[5][MOD_REG[3]]) {
        GASV_1_D_ONOFF(0);
      }
    } else {
      GASV_1_D_ONOFF(0);
      analogWrite(OUT_GASV_1_A, 0);
    }
    if (S_AUTO_SETTING[4][MOD_REG[3]])  {
      PID_IN_2 = TEMP_2;
      PID_SET_2 = S_AUTO_SETTING[6][MOD_REG[3]];
      PID2.Compute();
      analogWrite(OUT_GASV_2_A, PID_OUT_2);
      if (TEMP_2 - S_TEMP_DIFF <= S_AUTO_SETTING[6][MOD_REG[3]]) {
        GASV_2_D_ONOFF(1);
      } else if (TEMP_2 + S_TEMP_DIFF >= S_AUTO_SETTING[6][MOD_REG[3]]) {
        GASV_2_D_ONOFF(0);
      }
    } else {
      GASV_2_D_ONOFF(0);
      analogWrite(OUT_GASV_2_A, 0);
    }
  } else {
    TIME_ELAPSED = 0;
  }



  /////////////////////////////////////
  ////////////// MAN //////////////////

  if (MOD_REG[1]) {
    if (S_MAN_SETTING[0]) {
      PUMP_ONOFF(1);
    } else {
      PUMP_ONOFF(0);
    }
    if (S_MAN_SETTING[1]) {
      STIRRER_ONOFF(1);
    } else {
      STIRRER_ONOFF(0);
    }
    if (S_MAN_SETTING[2]) {
      if (S_MAN_SETTING[6]) {
        PID_IN_1 = TEMP_1;
        PID_SET_1 = S_MAN_SETTING[8];
        PID1.Compute();
        analogWrite(OUT_GASV_1_A, PID_OUT_1);
        if (TEMP_1 - S_TEMP_DIFF <= S_MAN_SETTING[8]) {
          GASV_1_D_ONOFF(1);
        } else if (TEMP_1 + S_TEMP_DIFF >= S_MAN_SETTING[8]) {
          GASV_1_D_ONOFF(0);
        }
      } else {
        GASV_1_D_ONOFF(1);
        analogWrite(OUT_GASV_1_A, map(S_MAN_SETTING[4], 0, 100, 0, 1024));
      }
    } else {
      GASV_1_D_ONOFF(0);
      analogWrite(OUT_GASV_1_A, 0);
    }
    if (S_MAN_SETTING[3]) {
      if (S_MAN_SETTING[7]) {
        PID_IN_2 = TEMP_2;
        PID_SET_2 = S_MAN_SETTING[9];
        PID2.Compute();
        analogWrite(OUT_GASV_2_A, PID_OUT_2);
        if  (TEMP_2 - S_TEMP_DIFF <= S_MAN_SETTING[9]) {
          GASV_2_D_ONOFF(1);
        } else if (TEMP_2 + S_TEMP_DIFF >= S_MAN_SETTING[9]) {
          GASV_2_D_ONOFF(0);
        }
      } else {
        GASV_2_D_ONOFF(1);
        analogWrite(OUT_GASV_2_A, map(S_MAN_SETTING[5], 0, 100, 0, 1024));
      }
    } else {
      GASV_2_D_ONOFF(0);
      analogWrite(OUT_GASV_2_A, 0);
    }
  }
}


void PUMP_ONOFF (bool onoff) {
  if (onoff) {
    digitalWrite(OUT_PUMP, LOW);
  } else {
    digitalWrite(OUT_PUMP, HIGH);
  }
}

void STIRRER_ONOFF (bool onoff) {
  if (onoff) {
    digitalWrite(OUT_STIRRER, LOW);
  } else {
    digitalWrite(OUT_STIRRER, HIGH);
  }
}

void GASV_1_D_ONOFF (bool onoff) {
  if (onoff) {
    digitalWrite(OUT_GASV_1_D, LOW);
  } else {
    digitalWrite(OUT_GASV_1_D, HIGH);
  }
}


void GASV_2_D_ONOFF (bool onoff) {
  if (onoff) {
    digitalWrite(OUT_GASV_2_D, LOW);
  } else {
    digitalWrite(OUT_GASV_2_D, HIGH);
  }
}















