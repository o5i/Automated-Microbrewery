#include <Arduino.h>
#include <SimpleModbusSlave.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h> //http://playground.arduino.cc/Code/PIDLibrary
#include <Bounce.h>

#define PWM_1 22
#define PWM_2 23
#define BUZZER 3
#define DO_1 5
#define DO_2 6
#define DO_3 7
#define DO_4 8
#define DO_5 18
#define DO_6 19
#define DO_7 20
#define DO_8 21
#define BUS_PIN 10
#define TEMPERATURE_PRECISION 11
#define ANALOG_RESOLUTION 16

unsigned int REG[100];
double PID_SET_1, PID_IN_1, PID_OUT_1;
double PID_SET_2, PID_IN_2, PID_OUT_2;
//bool STATE;

//elapsedMicros timer;
OneWire ONEWIRE_BUS(BUS_PIN);
DallasTemperature DS18B20(&ONEWIRE_BUS);
DeviceAddress DS18B20_0, DS18B20_1, DS18B20_2;
PID PID1(&PID_IN_1, &PID_OUT_1, &PID_SET_1, 3, 10, 2, DIRECT);
PID PID2(&PID_IN_2, &PID_OUT_2, &PID_SET_2, 3, 10, 2, DIRECT);

void setup()
{
  memset(REG, 0, sizeof(REG));
  modbus_configure(&Serial1, 19200, SERIAL_8N1, 1, 2, sizeof(REG), REG);
  modbus_update_comms(19200, SERIAL_8N1, 1);
  modbus_update();
  REG[60] = 40000;
  REG[61] = 60000;
  REG[64] = 700;
  REG[81] = 3;
  REG[82] = 10;
  REG[83] = 1;
  pinMode(DO_1, OUTPUT);
  pinMode(DO_2, OUTPUT);
  pinMode(DO_3, OUTPUT);
  pinMode(DO_4, OUTPUT);
  pinMode(DO_5, OUTPUT);
  pinMode(DO_6, OUTPUT);
  pinMode(DO_7, OUTPUT);
  pinMode(DO_8, OUTPUT);
  pinMode(PWM_1, OUTPUT);
  pinMode(PWM_2, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(DO_1, !REG[10]);
  digitalWrite(DO_2, !REG[11]);
  digitalWrite(DO_3, !REG[12]);
  digitalWrite(DO_4, !REG[13]);
  digitalWrite(DO_5, !REG[14]);
  digitalWrite(DO_6, !REG[15]);
  digitalWrite(DO_7, !REG[16]);
  digitalWrite(DO_8, !REG[17]);
  analogWriteFrequency(PWM_1, REG[64] );
  analogWriteFrequency(PWM_2, REG[64] );
  analogWriteResolution(ANALOG_RESOLUTION);
  DS18B20.begin();
  DS18B20.getAddress(DS18B20_0, 0);
  DS18B20.getAddress(DS18B20_1, 1);
  DS18B20.getAddress(DS18B20_2, 2);
  DS18B20.setResolution(DS18B20_0, TEMPERATURE_PRECISION);
  DS18B20.setResolution(DS18B20_1, TEMPERATURE_PRECISION);
  DS18B20.setResolution(DS18B20_2, TEMPERATURE_PRECISION);
  PID1.SetOutputLimits(0, 65535);
  PID2.SetOutputLimits(0, 65535);
  PID1.SetMode(AUTOMATIC);
  PID2.SetMode(AUTOMATIC);
  PID1.SetTunings(REG[81], REG[82], REG[83]);
  PID2.SetTunings(REG[81], REG[82], REG[83]);
}

void loop()
{
  modbus_update();
  PID1.Compute();
  PID2.Compute();

  if (REG[80]) {
    if (PID_OUT_1 >= 30000) {
      REG[10]=1;
    }
    if (PID_OUT_1 <= 0) {
      REG[10]=0;
    }
    if (PID_OUT_2 >= 30000) {
      REG[11]=1;
    }
    if (PID_OUT_2 <= 0) {
      REG[11]=0;
    }
    if (REG[10]){
      REG[1] = PID_OUT_1;
    }
    if (REG[11]){
      REG[2] = PID_OUT_2;
    }
  }


  if (REG[64] > 700) {
    REG[64] = 700;
  }
  if (REG[64] < 200) {
    REG[64] = 200;
  }
  PID_IN_1 = REG[41];
  PID_IN_2 = REG[42];
  PID_SET_1 = REG[70];
  PID_SET_2 = REG[71];
  REG[72] = PID_OUT_1;
  REG[73] = PID_OUT_2;

  PID1.SetTunings(REG[81], REG[82], REG[83]);
  PID2.SetTunings(REG[81], REG[82], REG[83]);
  analogWriteFrequency(PWM_1, REG[64] );
  analogWriteFrequency(PWM_2, REG[64] );
  DS18B20.setWaitForConversion(false);
  DS18B20.requestTemperatures();
  REG[40] = DS18B20.getDeviceCount();
  REG[41] = DS18B20.getTempC(DS18B20_0) * 100;
  REG[42] = DS18B20.getTempC(DS18B20_1) * 100;
  REG[43] = DS18B20.getTempC(DS18B20_2) * 100;


  //if ((REG[10])||  (REG[11])){
  //  if (timer>=1000000/REG[65] ) {
  //    timer = 0;
  //    STATE = !STATE;
  //  }
  //}

  // (((VALUE-VAL_MIN)(SET_MAX-SET_MIN))/(VAL_MAX-VAL_MIN)+SET_MIN)
  if (REG[10]){
    //if (timer>=(1000000/REG[65])-((1000000/REG[65])/100)*REG[66]) {
    //if (STATE){
    //analogWrite(PWM_1, REG[60]);
    //} else {
    //analogWrite(PWM_1, REG[61]);
    //}
    //} else {
    analogWrite(PWM_1, ((REG[1]*(REG[61]-REG[60]))/65535)+REG[60]);
    //}
  } else {
    analogWrite(PWM_1, 0);
  }
  if (REG[11]){
    //if (timer>=(1000000/REG[65])-((1000000/REG[65])/100)*REG[66]) {
    //if (STATE){
    //analogWrite(PWM_2, REG[60]);
    //} else {
    //analogWrite(PWM_2, REG[61]);
    //}
    //} else {
    analogWrite(PWM_2,  ((REG[2]*(REG[61]-REG[60]))/65535)+REG[60]);
    //}
  } else {
    analogWrite(PWM_2, 0);
  }
  digitalWrite(DO_1, !REG[10]);
  digitalWrite(DO_2, !REG[11]);
  digitalWrite(DO_3, !REG[12]);
  digitalWrite(DO_4, !REG[13]);
  digitalWrite(DO_5, !REG[14]);
  digitalWrite(DO_6, !REG[15]);
  digitalWrite(DO_7, !REG[16]);
  digitalWrite(DO_8, !REG[17]);
}