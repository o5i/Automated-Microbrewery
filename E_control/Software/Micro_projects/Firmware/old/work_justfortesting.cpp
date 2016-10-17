#include <SimpleModbusSlave.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>


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
// #define ANALOG_FREQ 366.2109    // @24Mhz
//#define ANALOG_FREQ 549.3164    // @72Mhz
//#define ANALOG_FREQ 732.4218    // @48-96Mhz


OneWire ONEWIRE_BUS(BUS_PIN);
DallasTemperature DS18B20(&ONEWIRE_BUS);
DeviceAddress DS18B20_0, DS18B20_1, DS18B20_2;
unsigned int REG[100];

void setup()
{
  memset(REG, 0, sizeof(REG));
  modbus_configure(&Serial1, 19200, SERIAL_8N1, 1, 2, sizeof(REG), REG);
  modbus_update_comms(19200, SERIAL_8N1, 1);
  modbus_update();
  REG[64]=366;
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
}

void loop()
{
  modbus_update();
  analogWriteFrequency(PWM_1, REG[64] );
  analogWriteFrequency(PWM_2, REG[64] );
  analogWrite(PWM_1, REG[1]);
  analogWrite(PWM_2, REG[2]);
  //analogWrite(PWM_1, REG[1] >> 4);  // Use if change resolution
  //analogWrite(PWM_2, REG[2] >> 4);  // Use if change resolution
  digitalWrite(DO_1, !REG[10]);
  digitalWrite(DO_2, !REG[11]);
  digitalWrite(DO_3, !REG[12]);
  digitalWrite(DO_4, !REG[13]);
  digitalWrite(DO_5, !REG[14]);
  digitalWrite(DO_6, !REG[15]);
  digitalWrite(DO_7, !REG[16]);
  digitalWrite(DO_8, !REG[17]);
  DS18B20.setWaitForConversion(false);
  DS18B20.requestTemperatures();
  REG[40] = DS18B20.getDeviceCount();
  REG[41] = DS18B20.getTempC(DS18B20_0) * 100;
  REG[42] = DS18B20.getTempC(DS18B20_1) * 100;
  REG[43] = DS18B20.getTempC(DS18B20_2) * 100;
}
