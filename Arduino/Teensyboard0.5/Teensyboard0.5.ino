/*
  Read Write Modbus registers test
  gasventil über pid einaus?
  modbus change "modbus_construct" http://flprog.ru/_fr/6/SimpleModbusMas.pdf
*/

#include <PID_v1.h> //http://playground.arduino.cc/Code/PIDLibrary
#include <SimpleModbusSlave.h> //http://forum.arduino.cc/index.php?topic=176142.new#new https://drive.google.com/folderview?id=0B0B286tJkafVSENVcU1RQVBfSzg&usp=drive_web'
#include <elapsedMillis.h> //http://playground.arduino.cc/Code/ElapsedMillis
#include <OneWire.h> //http://playground.arduino.cc/Learning/OneWire
#include <DallasTemperature.h>

// STUFF
#define OUT_PUMP 7
#define OUT_STIRRER 8
#define OUT_COOLER_CIRCULATION 20
#define OUT_BUZZER 9
#define OUT_GASV_1_A 5 // Regulation Valve
#define OUT_GASV_1_D 3 // On Off Valve
#define OUT_GASV_2_A 4 // Regulation Valve
#define OUT_GASV_2_D 6 // On Off Valve
#define OUT_LED 13

// SERIAL
#define SER_COM Serial
#define SER_BAUD 115200

// ONEWIRE
#define BUS_PIN 10
#define TEMPERATURE_PRECISION 11
OneWire ONEWIRE_BUS(BUS_PIN);
DallasTemperature DS18B20(&ONEWIRE_BUS);
DeviceAddress DS18B20_0, DS18B20_1, DS18B20_2;

// MODBUS
#define MOD_REGS 600
#define MOD_COM Serial1
#define MOD_BAUD 57600
unsigned int MOD_REG[MOD_REGS];

// PID
double PID_SET_1, PID_IN_1, PID_OUT_1;
double PID_SET_2, PID_IN_2, PID_OUT_2;
PID PID1(&PID_IN_1, &PID_OUT_1, &PID_SET_1, 2, 5, 1, DIRECT);
PID PID2(&PID_IN_2, &PID_OUT_2, &PID_SET_2, 2, 5, 1, DIRECT);

// Timers
elapsedMillis TIME_BUZZER;
elapsedMillis TIME_ELAPSED;
elapsedMillis WHOLE_TIME_ELAPSED;

// USER SETTINGS
// READ WRITE
// MOD_REG[0] // AUTO_MAN
// MOD_REG[1] // MAN_ON
// MOD_REG[2] // AUTO_ON
// MOD_REG[3] // Cycle
// MASTER TO SLAVE
const unsigned int S_GASV_ONOFF_DIFF = 200; // Min Max opening of the Valve
unsigned int S_AUTO_SETTING[8][20] = {0}; // 0=Autostart Cycle, 1=Pump, 2=Stirrer, 3=Gasv_1 onoff, 4=Gasv_2 onoff, 5=Target Temp_1, 6=Target Temp 2 , 7=Time Cycle), 8=Cooler circulation pump
unsigned int S_MAN_SETTING[9] = {0}; // 0=Pump, 1=Stirrer, 2=Gasv_1 on off, 3=Gasv_2 on off, 4=Gasv_1 Manual Power 0-100, 5=Gasv_2 Manual Power 0-100, 6=Gasv_1 temp/pwr, 7=Gasv_2 temp/pwr, 8=Target Temp_1, 9=Target Temp 2), 10=Cooler circulation pump
// SLAVE TO MASTER
unsigned int TEMP_1;
unsigned int TEMP_2;
unsigned int TEMP_3;

void setup() {
  delay(5000);
  pinMode(OUT_PUMP, OUTPUT);
  pinMode(OUT_STIRRER, OUTPUT);
  pinMode(OUT_COOLER_CIRCULATION, OUTPUT);
  pinMode(OUT_BUZZER, OUTPUT);
  pinMode(OUT_LED, OUTPUT);
  pinMode(OUT_GASV_1_D, OUTPUT);
  pinMode(OUT_GASV_2_D, OUTPUT);
  analogWriteResolution(10);
  pinMode(OUT_GASV_1_A, OUTPUT);
  analogWriteFrequency(OUT_GASV_1_A, 22000);
  pinMode(OUT_GASV_2_A, OUTPUT);
  analogWriteFrequency(OUT_GASV_2_A, 22000);
  SER_COM.begin(SER_BAUD);

  SER_COM.println("INITIALIZING SENSORS");
  DS18B20.begin();
  DS18B20.getAddress(DS18B20_0, 0);
  DS18B20.getAddress(DS18B20_1, 1);
  DS18B20.getAddress(DS18B20_2, 2);
  DS18B20.setResolution(DS18B20_0, TEMPERATURE_PRECISION);
  DS18B20.setResolution(DS18B20_1, TEMPERATURE_PRECISION);
  DS18B20.setResolution(DS18B20_2, TEMPERATURE_PRECISION);
  SER_COM.print("DEVICES:");
  SER_COM.println(DS18B20.getDeviceCount(), DEC);

  SER_COM.println("INITIALIZING MODBUS");
  modbus_configure(&MOD_COM, MOD_BAUD, SERIAL_8N2, 1, 2, MOD_REGS, MOD_REG);
  modbus_update_comms(MOD_BAUD, SERIAL_8N2, 1);

  SER_COM.println("INITIALIZING PID CONTROLLERS");
  PID1.SetOutputLimits(0, 1023);
  PID2.SetOutputLimits(0, 1023);
  PID1.SetMode(AUTOMATIC);
  PID2.SetMode(AUTOMATIC);

  SER_COM.println("RESET VARIABLES");
  memset(MOD_REG, 0, sizeof(MOD_REG));
  TIME_ELAPSED = 0;
  WHOLE_TIME_ELAPSED = 0;

  SER_COM.println("SWITCH EVERYTHING OFF");
  PUMP_ONOFF(0);
  STIRRER_ONOFF(0);
  COOLER_ONOFF(0);
  GASV_1_D_ONOFF(0);
  GASV_2_D_ONOFF(0);
  analogWrite(OUT_GASV_1_A, 0);
  analogWrite(OUT_GASV_2_A, 0);
  digitalWrite(OUT_LED, HIGH);
  BUZZER();
}

void loop() {
  if (Serial.available()) {
    BUZZER();
    Serial.read();
    DEBUG();
  }

  // Onewire Temperature read
  DS18B20.setWaitForConversion(false);
  DS18B20.requestTemperatures();
  TEMP_1 = DS18B20.getTempC(DS18B20_0) * 100;
  TEMP_2 = DS18B20.getTempC(DS18B20_1) * 100;
  TEMP_3 = DS18B20.getTempC(DS18B20_2) * 100;

  // Modbus
  modbus_update();

  // Master to Slave
  for (int i = 0; i < 10; i++) {
    int offset_;
    offset_ = 100;
    S_MAN_SETTING[i] = MOD_REG[offset_ + i];
  }

  for (int i = 0; i < 8; i++) {
    int offset_;
    offset_ = 200;
    for (int j = 0; j < 20; j++) {
      S_AUTO_SETTING[i][j] = MOD_REG[offset_ + i * j];
    }
  }

  // Slave to Master
  MOD_REG[500] = TEMP_1;
  MOD_REG[501] = TEMP_2;
  MOD_REG[502] = TEMP_3;
  MOD_REG[503] = PID_OUT_1;
  MOD_REG[504] = PID_OUT_2;

  ///////// MAN AUTO ////////////////

  if (MOD_REG[0]) {
    MOD_REG[1] = 0;
    if ((!S_AUTO_SETTING[0][MOD_REG[3]]) && (!MOD_REG[2])) {
      BUZZER();
    }
  } else {
    MOD_REG[2] = 0;
  }

  ///////// ALL OFF ////////////////

  if ((!MOD_REG[1]) || (!MOD_REG[2])) {
    PUMP_ONOFF(0);
    STIRRER_ONOFF(0);
    COOLER_ONOFF(0);
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
      TIME_ELAPSED = 0;
      if (!S_AUTO_SETTING[0][MOD_REG[3]]) {
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
    } else {
      STIRRER_ONOFF(0);
    }
    if (S_AUTO_SETTING[8][MOD_REG[3]]) {
      COOLER_ONOFF(1);
    } else {
      COOLER_ONOFF(0);
    }
    if ((S_AUTO_SETTING[3][MOD_REG[3]]) && (DS18B20.getDeviceCount() == 3)) {
      PID_IN_1 = TEMP_1;
      PID_SET_1 = S_AUTO_SETTING[5][MOD_REG[3]];
      PID1.Compute();
      analogWrite(OUT_GASV_1_A, PID_OUT_1);
      if (PID_OUT_1) {
        GASV_1_D_ONOFF(1);
      } else  {
        GASV_1_D_ONOFF(0);
      }
    } else {
      GASV_1_D_ONOFF(0);
      analogWrite(OUT_GASV_1_A, 0);
    }
    if ((S_AUTO_SETTING[4][MOD_REG[3]]) && (DS18B20.getDeviceCount() == 3)) {
      PID_IN_2 = TEMP_2;
      PID_SET_2 = S_AUTO_SETTING[6][MOD_REG[3]];
      PID2.Compute();
      analogWrite(OUT_GASV_2_A, PID_OUT_2);
      if (PID_OUT_2) {
        GASV_2_D_ONOFF(1);
      } else  {
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
    if (S_MAN_SETTING[10]) {
      COOLER_ONOFF(1);
    } else {
      COOLER_ONOFF(0);
    }
    if (S_MAN_SETTING[2]) {
      if ((S_MAN_SETTING[6]) && (DS18B20.getDeviceCount() == 3)) {
        PID_IN_1 = TEMP_1;
        PID_SET_1 = S_MAN_SETTING[8];
        PID1.Compute();
        analogWrite(OUT_GASV_1_A, PID_OUT_1);
        if (PID_OUT_1) {
          GASV_1_D_ONOFF(1);
        } else {
          GASV_1_D_ONOFF(0);
        }
      } else {
        GASV_1_D_ONOFF(1);
        analogWrite(OUT_GASV_1_A, map(S_MAN_SETTING[4], 0, 100, 0, 1023));
      }
    } else {
      GASV_1_D_ONOFF(0);
      analogWrite(OUT_GASV_1_A, 0);
    }
    if (S_MAN_SETTING[3]) {
      if ((S_MAN_SETTING[7]) && (DS18B20.getDeviceCount() == 3)) {
        PID_IN_2 = TEMP_2;
        PID_SET_2 = S_MAN_SETTING[9];
        PID2.Compute();
        analogWrite(OUT_GASV_2_A, PID_OUT_2);
        if (PID_OUT_2) {
          GASV_2_D_ONOFF(1);
        } else {
          GASV_2_D_ONOFF(0);
        }
      } else {
        GASV_2_D_ONOFF(1);
        analogWrite(OUT_GASV_2_A, map(S_MAN_SETTING[5], 0, 100, 0, 1023));
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

void COOLER_ONOFF (bool onoff) {
  if (onoff) {
    digitalWrite(OUT_COOLER_CIRCULATION, LOW);
  } else {
    digitalWrite(OUT_COOLER_CIRCULATION, HIGH);
  }
}

void BUZZER() {
  if (TIME_BUZZER >= 300) {
    TIME_BUZZER = 0;
    noTone(OUT_BUZZER);
  }
  noTone(OUT_BUZZER);
  tone(OUT_BUZZER, 300 + TIME_BUZZER, 100);
}

void DEBUG() {
  SER_COM.print("TIME:");
  SER_COM.print(WHOLE_TIME_ELAPSED / 1000);
  SER_COM.print(" TIMER:");
  SER_COM.print(TIME_ELAPSED / 1000);
  SER_COM.print(" DEVICES:");
  SER_COM.print(DS18B20.getDeviceCount(), DEC);
  SER_COM.print(" MODE:");
  if (MOD_REG[0]) {
    SER_COM.print("AUTO");
  } else {
    SER_COM.print("MAN");
  }
  SER_COM.print(" AUTO:");
  if (MOD_REG[2]) {
    SER_COM.print("ON");
  } else {
    SER_COM.print("OFF");
  }
  SER_COM.print(" MAN:");
  if (MOD_REG[1]) {
    SER_COM.print("ON");
  } else {
    SER_COM.print("OFF");
  }
  SER_COM.print(" CYCLE:");
  SER_COM.print(MOD_REG[2]);
  SER_COM.print(" TEMP1:");
  SER_COM.print(TEMP_1);
  SER_COM.print(" TEMP2:");
  SER_COM.print(TEMP_2);
  SER_COM.print(" TEMP3:");
  SER_COM.print(TEMP_3);
  SER_COM.print(" PID1:");
  SER_COM.print(PID_OUT_1);
  SER_COM.print(" PID2:");
  SER_COM.print(PID_OUT_2);
  SER_COM.print(" PUMP:");
  if (!digitalRead(OUT_PUMP)) {
    SER_COM.print("ON");
  } else {
    SER_COM.print("OFF");
  }
  SER_COM.print(" STIRRER:");
  if (!digitalRead(OUT_STIRRER)) {
    SER_COM.print("ON");
  } else {
    SER_COM.print("OFF");
  }
  SER_COM.print(" COOLER:");
  if (!digitalRead(OUT_COOLER_CIRCULATION)) {
    SER_COM.print("ON");
  } else {
    SER_COM.print("OFF");
  }
  SER_COM.print(" GASVALVE1:");
  if (!digitalRead(OUT_GASV_1_D)) {
    SER_COM.print("ON");
  } else {
    SER_COM.print("OFF");
  }
  SER_COM.print(" GASVALVE2:");
  if (!digitalRead(OUT_GASV_2_D)) {
    SER_COM.print("ON");
  } else {
    SER_COM.print("OFF");
  }
  SER_COM.println();
}









