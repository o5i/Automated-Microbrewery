#include <OneWire.h>              //http://playground.arduino.cc/Learning/OneWire
#include <PID_v1.h>               //http://playground.arduino.cc/Code/PIDLibrary
#include <SimpleModbusSlave.h>    //http://forum.arduino.cc/index.php?topic=176142.new#new https://drive.google.com/folderview?id=0B0B286tJkafVSENVcU1RQVBfSzg&usp=drive_web'
#include <elapsedMillis.h>        //http://playground.arduino.cc/Code/ElapsedMillis

#define Pump 11
#define Ruehrwerk 12
#define Fire1pwm 13
#define Fire1digital 14
#define Fire2pwm 15
#define Fire2digital 16
#define Numofsensors 3
#define HOLDING_REGS_SIZE 2000
#define serial_com Serial1
#define serial_baud 57600
/* todo
  add start stop option on every cycle
  check timers if out of range
  Modbus
  Read Temp
  Check conversion int double temp
  check matematics int/double = int
  Unify Fire algorythm
  Add fire hysteresis to fire algorythm

*/

// User Input
// Start
bool  SetStartStop = 0;
// Mode
bool  SetModeAutoMan = 0;
// Manual Settings
bool  SetManPump = 0, SetManRuehrwerk = 0, SetManFire1ONOFF = 0, SetManFire2ONOFF = 0, SetManModeTmpPwr1 = 0, SetManModeTmpPwr2 = 0;
double  SetManTmp1 = 0, SetManTmp2 = 0;
int SetManPwr1 = 0, SetManPwr2 = 0;
// Automatic
int SetAutoNumofCycle = 0, SetAutoCycle = 0;
double SetAutoTemp1[20], SetAutoTime1[20], SetAutoTemp2[20], SetAutoTime2[20], SetAutoFire1ONOFF[20], SetAutoFire2ONOFF[20];
bool  SetAutoPump[20], SetAutoRuehrwerk[20], SetAutoConfirmtoproceed[20], AutoReadytoproceed = 0, SetAutoproceed = 0;

// Fire Control
// PID
double Setpid1, Inpid1, Outpid1, Setpid2, Inpid2, Outpid2;
int SetFireOnOffHysteresis = 2;

// Controler Input
// INPUTS
double TempFire1, TempFire2, TempFire3;
byte TempSensAddr[Numofsensors][8];

// Modbus Communication
unsigned int  holdingRegs[HOLDING_REGS_SIZE];

// Libs
elapsedMillis timeElapsed;
OneWire TempSens(10);
PID Pid1(&Inpid1, &Outpid1, &Setpid1, 2, 5, 1, DIRECT);
PID Pid2(&Inpid2, &Outpid2, &Setpid2, 2, 5, 1, DIRECT);

void setup() {
  modbus_configure(&serial_com, serial_baud, SERIAL_8N2, 1, 2, HOLDING_REGS_SIZE, holdingRegs);
  modbus_update_comms(serial_baud, SERIAL_8N2, 1);
  memset(SetAutoTemp1, 0, sizeof(SetAutoTemp1));
  memset(SetAutoTemp2, 0, sizeof(SetAutoTemp2));
  memset(SetAutoTime1, 0, sizeof(SetAutoTime1));
  memset(SetAutoTime2, 0, sizeof(SetAutoTime2));
  memset(SetAutoPump, 0, sizeof(SetAutoPump));
  memset(SetAutoRuehrwerk, 0, sizeof(SetAutoRuehrwerk));
  memset(holdingRegs, 0, sizeof(holdingRegs));
  pinMode(Pump, OUTPUT);
  pinMode(Ruehrwerk, OUTPUT);
  pinMode(Fire1pwm, OUTPUT);
  pinMode(Fire1digital, OUTPUT);
  pinMode(Fire2pwm, OUTPUT);
  pinMode(Fire2digital, OUTPUT);
  // Switch Outputs OFF
  fire1_onoff_pwr(0, 0);
  fire2_onoff_pwr(0, 0);
  pump_onoff(0);
  ruerwerk_onoff(0);
}

void loop() {
  modbus_update();
  modbus_read_write();
  if (SetStartStop == 1) {
    // Auto Mode
    if (SetModeAutoMan == 1) {
      // Pumpe
      pump_onoff(SetAutoPump[SetAutoCycle]);
      // Ruehrwerk
      ruerwerk_onoff(SetAutoRuehrwerk[SetAutoCycle]);



      
      if (SetAutoFire1ONOFF[SetAutoCycle]) {
        if ((TempFire1 >= SetAutoTemp1[SetAutoCycle] + SetFireOnOffHysteresis) && (fire1())) {
          fire1(0, 0, TempFire1, SetAutoTemp1[SetAutoCycle], 0);
        } else ((TempFire1 <= SetAutoTemp1[SetAutoCycle] - SetFireOnOffHysteresis) && (!fire1())) {
          fire1(1, 0, TempFire1, SetAutoTemp1[SetAutoCycle], 0);
        }
      }
    } else {
      fire1(0, 1, TempFire1, SetAutoTemp1[SetAutoCycle], 0);
    }










  } else {                            // Man Mode
    if (SetManPump == 1) {            // Pump
      pump_onoff(1);
    } else {
      pump_onoff(0);
    }
    if (SetManRuehrwerk == 1) {       // Ruerwerk
      ruerwerk_onoff(1);
    } else {
      ruerwerk_onoff(0);
    }
    if (SetManFire1ONOFF == 1) {      // Brenner 1
      if (SetManModeTmpPwr1 == 1) {           // Leistung Manuell
        fire1_onoff_pwr(1, SetManPwr1);
      } else {                                // Leistung Automatik
        Inputfire1 = TempFire1;
        Setpointfire1 = SetManTmp1;
        Fire1pid.Compute();
        fire1_onoff_pwr(1, Outputfire1);
      }
    } else {
      fire1_onoff_pwr(0, 0);
    }
    if (SetManFire2ONOFF == 1) {      // Brenner 1
      if (SetManModeTmpPwr2 == 1) {           // Leistung Manuell
        fire2_onoff_pwr(1, SetManPwr2);
      } else {
        Inputfire2 = TempFire2;               // Leistung Automatik
        Setpointfire2 = SetManTmp2;
        Fire1pid.Compute();
        fire2_onoff_pwr(1, Outputfire2);
      }
    } else {
      fire2_onoff_pwr(0, 0);
    }
  }
} else {
  fire2_onoff_pwr(0, 0);
  fire1_onoff_pwr(0, 0);
  ruerwerk_onoff(0);
  pump_onoff(0);
}
}


void pump_onoff(bool onoff) {
  if (onoff == 1) {
    digitalWrite(Pump, LOW);
  } else {
    digitalWrite(Pump, HIGH);
  }
}

void ruerwerk_onoff(bool onoff) {
  if (onoff == 1) {
    digitalWrite(Ruehrwerk, LOW);
  } else {
    digitalWrite(Ruehrwerk, HIGH);
  }
}

bool fire2(bool onoff, bool _man, double _ref, double _set, int _pwr) {
  if (onoff == 1) {
    digitalWrite(Fire2digital, HIGH);
    return 1;
  } else {
    digitalWrite(Fire2digital, LOW);
    return 0;
  }
  if (_man) {
    analogWrite(Fire2pwm, _pwr);
  } else {
    Inpid_1 = _ref;
    Setpid_1 = _set;
    Pid_1.Compute();
    fire1_onoff_pwr(1, Outputfire1);
  }
}





void modbus_read_write() {
  //Write to controller
  // Commands
  SetStartStop = holdingRegs[0];
  SetModeAutoMan = holdingRegs[1];
  // Manual
  SetManPump = holdingRegs[10];
  SetManRuehrwerk = holdingRegs[11];
  SetManFire1ONOFF = holdingRegs[12];
  SetManFire2ONOFF = holdingRegs[13];
  SetManModeTmpPwr1 = holdingRegs[14];
  SetManModeTmpPwr2 = holdingRegs[15];
  SetManTmp1 = holdingRegs[16];
  SetManTmp2 = holdingRegs[17];
  SetManPwr1 = holdingRegs[18];
  SetManPwr2 = holdingRegs[19];
  // Automatic
  SetAutoNumofCycle = holdingRegs[30];
  SetAutoCycle = holdingRegs[31];
  SetAutoproceed = holdingRegs[32];
  for (int i = 0; i < 20; i++) {
    int offset_;
    offset_ = 40;
    SetAutoTemp1[i] = holdingRegs[i + offset_];
    offset_ = 60;
    SetAutoTemp2[i] = holdingRegs[i + offset_];
    offset_ = 80;
    SetAutoTime1[i] = holdingRegs[i + offset_];       // Zyklusdauer in Minuten
    offset_ = 100;
    SetAutoTime2[i] = holdingRegs[i + offset_];       // Zyklusdauer in Minuten
    offset_ = 120;
    SetAutoPump[i] = holdingRegs[i + offset_];
    offset_ = 140;
    SetAutoRuehrwerk[i] = holdingRegs[i + offset_];
    offset_ = 160;
    SetAutoConfirmtoproceed[i] =  holdingRegs[i + offset_];
  }
  // Read Inputs from Controller
  holdingRegs[1000] = TempFire1;
  holdingRegs[1001] = TempFire2;
  holdingRegs[1002] = TempFire3;
  holdingRegs[1010] = !digitalRead(Fire1digital);
  holdingRegs[1011] = !digitalRead(Fire2digital);
  holdingRegs[1012] = !digitalRead(Pump);
  holdingRegs[1013] = !digitalRead(Ruehrwerk);

  // Read Variable from Controller
  holdingRegs[1040] = AutoReadytoproceed;
  holdingRegs[1041] = Outputfire1;  // PID
  holdingRegs[1042] = Outputfire2;  // PID
}


