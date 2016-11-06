#include <Arduino.h>
#include <SimpleModbusSlave.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Bounce.h>

#define BUZZER 3
#define BUS_PIN 10
#define TEMPERATURE_PRECISION 11
#define Num_setings 10
#define Set_temp_diff 2




// Modbus
const bool DO[] ={14,15,16,17,20,21,22,23};

unsigned int REG[200];


elapsedMillis Timer;

OneWire ONEWIRE_BUS(BUS_PIN);
DallasTemperature DS18B20(&ONEWIRE_BUS);
DeviceAddress T_0, T_1, T_2;

void setup()
{
  memset(REG, 0, sizeof(REG));
  modbus_configure(&Serial1, 19200, SERIAL_8N1, 1, 2, sizeof(REG), REG);
  modbus_update_comms(19200, SERIAL_8N1, 1);
  modbus_update();
  DS18B20.begin();
  DS18B20.getAddress(T_0, 0);
  DS18B20.getAddress(T_1, 1);
  DS18B20.getAddress(T_2, 2);
  DS18B20.setResolution(T_0, TEMPERATURE_PRECISION);
  DS18B20.setResolution(T_1, TEMPERATURE_PRECISION);
  DS18B20.setResolution(T_2, TEMPERATURE_PRECISION);
  DS18B20.setWaitForConversion(false);
  for (int x = 0;x<sizeof(DO);x++){
    pinMode(DO[x],OUTPUT);
  }
  pinMode(BUZZER, OUTPUT);
}

void loop()
{
  modbus_update();
  unsigned int offset;
  // Read Temperatures
  DS18B20.requestTemperatures();
  offset=0;
  REG[offset]= DS18B20.getDeviceCount();
  offset=1;
  float_to_int(DS18B20.getTempC(T_0), REG[offset+1], REG[offset]);
  offset+=2;
  float_to_int(DS18B20.getTempC(T_1), REG[offset+1], REG[offset]);
  offset+=2;
  float_to_int(DS18B20.getTempC(T_2), REG[offset+1], REG[offset]);

  // Write Outputs
  offset+=20;
  for (int x = 0;x<sizeof(DO);x++){
    digitalWrite(DO[x],!REG[offset]);
    offset++;
  }

  if (REG[30]) {                              // Auto On
    if (Timer>REG[40+REG[32]]*60000) {
      if ((REG[50+REG[32]])&&(!REG[31])){     // Startknopf
        ////Buzzer
      } else {
        offset = 60 + REG[32];
        REG[7] = REG[offset++];
        REG[8] = REG[offset++];
        offset = 80 + REG[32];
        REG[9] = REG[offset++];
        REG[10] = REG[offset++];
        offset = 100;
        for (int x=24;x<28;x++) {
          if (REG[offset]+REG[32]) {
            REG[x]=1;
          } else {
            REG[x]=0;
          }
          offset+=10;
        }
        REG[32]++;                                // Zyklus Ähm do unten laft der durch...
        Timer=0;
        REG[31]=1; ////////////////////// Fix it
      }
    }
    float Temp = int_to_float(REG[2], REG[1]);
    float Set_Temp = int_to_float(REG[8], REG[7]);
    offset = 120+REG[32];
    if (REG[offset]){
      if (Temp + REG[34]<= Set_Temp){
        REG[20]=1;
      }
      if (Temp >= Set_Temp){
        REG[20]=0;
      }
      if (REG[20]){
        if (Temp + REG[35]<= Set_Temp){
          REG[21]=1;
        }
        if (Temp + REG[36]>= Set_Temp){
          REG[21]=0;
        }
      } else {
        REG[21]=0;
      }
    }
  }
}


float int_to_float(unsigned int lng_1, unsigned int lng_2) {
  float flt;
  unsigned long lng;
  lng = (unsigned long)lng_1 << 16 | lng_2;
  flt = *(float*)&lng;
  return flt;
}

//float int_to_float(unsigned int lng_1, unsigned int lng_2, float flt) {
//  unsigned long lng;
//  lng = (unsigned long)lng_1 << 16 | lng_2;
//  flt = *(float*)&lng;
//  return flt;
//}

void float_to_int(float flt, unsigned int lng_1, unsigned int lng_2) {
  unsigned long lng;
  lng = *(long *)(&flt);
  lng_1 = (int) (lng >> 16) ;
  lng_2 = (int) (lng & 0xFFFF) ;
}
