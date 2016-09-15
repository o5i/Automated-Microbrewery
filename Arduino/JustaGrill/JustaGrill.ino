#include <Arduino.h>
#include <SimpleModbusSlave.h> //http://forum.arduino.cc/index.php?topic=176142.new#new https://drive.google.com/folderview?id=0B0B286tJkafVSENVcU1RQVBfSzg&usp=drive_web'


// STUFF
#define OUT_S1 7
#define OUT_S2 8
#define OUT_S3 20
#define OUT_S4 21
#define OUT_VD1 5 // Regulation Valve
#define OUT_VD2 3 // On Off Valve
#define OUT_VA1 4 // Regulation Valve
#define OUT_VA2 6 // On Off Valve

// MODBUS
#define MOD_REGS 220
#define MOD_COM Serial1
#define MOD_BAUD 115200
#define MOD_SER_MOD SERIAL_8N1
unsigned int MOD_REG[MOD_REGS];



void setup() {
  pinMode(OUT_S1, OUTPUT);
  pinMode(OUT_S2, OUTPUT);
  pinMode(OUT_S3, OUTPUT);
  pinMode(OUT_S4, OUTPUT);
  pinMode(OUT_VD1, OUTPUT);
  pinMode(OUT_VD2, OUTPUT);
  pinMode(OUT_VA1, OUTPUT);
  pinMode(OUT_VA2, OUTPUT);
  analogWriteResolution(10);
  analogWriteFrequency(OUT_VA1, 22000);
  analogWriteFrequency(OUT_VA2, 22000);
  modbus_configure(&MOD_COM, MOD_BAUD, MOD_SER_MOD, 1, 2, MOD_REGS, MOD_REG);
  modbus_update_comms(MOD_BAUD, MOD_SER_MOD, 1);
  memset(MOD_REG, 0, sizeof(MOD_REG));
}

void loop() {
  modbus_update();
  if (MOD_REG[0]) {
    digitalWrite(OUT_S1, LOW);
  } else {
    digitalWrite(OUT_S1, HIGH);
  }
  if (MOD_REG[1]) {
    digitalWrite(OUT_S2, LOW);
  } else {
    digitalWrite(OUT_S2, HIGH);
  }
  if (MOD_REG[2]) {
    digitalWrite(OUT_S3, LOW);
  } else {
    digitalWrite(OUT_S3, HIGH);
  }
  if (MOD_REG[3]) {
    digitalWrite(OUT_S4, LOW);
  } else {
    digitalWrite(OUT_S4, HIGH);
  }
  if (MOD_REG[4]) {
    digitalWrite(OUT_VD1, LOW);
  } else {
    digitalWrite(OUT_VD1, HIGH);
  }
  if (MOD_REG[5]) {
    digitalWrite(OUT_VD2, LOW);
  } else {
    digitalWrite(OUT_VD2, HIGH);
  }
  if ((MOD_REG[4]) && (MOD_REG[6])) {
    analogWrite(OUT_VA1, map(MOD_REG[6], 0, 100, 0, 1023));
  }
  else {
    analogWrite(OUT_VA1, 0);
  }
  if ((MOD_REG[5]) && (MOD_REG[7])) {
    analogWrite(OUT_VA2, map(MOD_REG[7], 0, 100, 0, 1023));
  }
  else {
    analogWrite(OUT_VA2, 0);
  }
}


