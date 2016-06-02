/*
///////////////////////////////////////////////////////////////////////
Mavpixel Mavlink Neopixel bridge
(c) 2016 Nick Metcalfe

Derived from: jD-IOBoard_MAVlink Driver
// Version      : v0.5-FrSky, 06-11-2013
// Author       : Jani Hirvinen, jani@j....com
// Co-Author(s) : 
//      Sandro Beningo     (MAVLink routines)
//      Mike Smith         (BetterStream and FastSerial libraries)



// If you use, modify, redistribute, Remember to share your modifications and 
// you need to include original authors along with your work !!
//
// Copyright (c) 2013, Jani Hirvinen, jDrones & Co.
// All rights reserved.
//
// - Redistribution and use in source and binary forms, with or without 
//   modification, are permitted provided that the following conditions are met:
//
// - Redistributions of source code must retain the above copyright notice, this 
//  list of conditions and the following disclaimer.
//
// - Redistributions in binary form must reproduce the above copyright notice, 
//  this list of conditions and the following disclaimer in the documentation 
//  and/or other materials provided with the distribution.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
//  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
//  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
//  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
//  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
//  POSSIBILITY OF SUCH DAMAGE.
//
/////////////////////////////////////////////////////////////////////////////
*/
 
/*
//////////////////////////////////////////////////////////////////////////
//  Description: 
// 
//  This is an Arduino sketch on how to use Mavpixel LED Driver board
//  that listens MAVLink commands and changes patterns accordingly.
//
//  If you use, redistribute this please mention original source.
//
//  This version uses four strips, maximum eight leds per strip.
//  WS2812 Led strip outputs:
//   pins A0 (1-8), A1(9-16), A2(17-24), A3(25-32)

 
/* ************************************************************ */
/* **************** MAIN PROGRAM - MODULES ******************** */
/* ************************************************************ */

#undef PROGMEM 
#define PROGMEM __attribute__(( section(".progmem.data") )) 

#undef PSTR 
#define PSTR(s) (__extension__({static prog_char __c[] PROGMEM = (s); &__c[0];})) 

#define MAVLINK10     // Are we listening MAVLink 1.0 or 0.9   (0.9 is obsolete now)
#define HEARTBEAT     // HeartBeat signal
//#define JD_IO
//#define SERDB         // Output debug information to SoftwareSerial 
//#define FRSKY          // FrSky serial output, cannot be run same time with SERDB
//#define ONOFFSW       // Do we have OnOff switch connected in pins 
//#define membug
//#define HWSWITCH    // Hardware factory reset option
#define LED_STRIP
#define USE_LED_ANIMATION

/* **********************************************/
/* ***************** INCLUDES *******************/

#define membug   // undefine for real firmware
//#define FORCEINIT  // You should never use this unless you know what you are doing 

#define hiWord(w) ((w) >> 8)
#define loWord(w) ((w) & 0xff)

// AVR Includes
#include <FastSerial.h>
#include <AP_Common.h>
#include <AP_Math.h>
#include <math.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
//#include <Wire.h>

// Get the common arduino functions
#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "wiring.h"
#endif
#include <EEPROM.h>
//#include <SimpleTimer.h>
#include <GCS_MAVLink.h>

#ifdef membug
#include <MemoryFree.h>
#endif

#include <SoftwareSerial.h>

//#include <Adafruit_NeoPixel.h>
#include <FastLED.h>

// Configurations
#include "IOBoard.h"
#include "color.h"
#include "ledstrip.h"
#include "IOEEPROM.h"

//#define DUMPEEPROM            // Should not be activated in repository code, only for debug
//#define DUMPEEPROMTELEMETRY   // Should not be activated in repository code, only for debug
#define NEWPAT
//#define HWRESET

/* *************************************************/
/* ***************** DEFINITIONS *******************/

#define VER "2.0"   // Software version
#define CHKVER 43    // Version number to check from EEPROM

// These are not in real use, just for reference
//#define O1 8      // High power Output 1
//#define O2 9      // High power Output 2, PWM
//#define O3 10     // High power Output 3, PWM
//#define O4 4      // High power Output 4 
//#define O5 3      // High power Output 5, PWM
//#define O6 2      // High power Output 6

#define Circle_Dly 1000

#define ledPin 13     // Heartbeat LED if any
#define LOOPTIME  50  // Main loop time for heartbeat
//#define BAUD 57600    // Serial speed

#define TELEMETRY_SPEED  57600  // How fast our MAVLink telemetry is coming to Serial port


/* Patterns and other variables */
//static byte LeRiPatt = NOMAVLINK; // default pattern is full ON
//static int curPwm;
//static int prePwm;


static long p_preMillis;
static long p_curMillis;
static int p_delMillis = LOOPTIME;

int messageCounter;
static bool mavlink_active;
static bool cli_active;
byte hbStatus;

char cmdBuffer[32];
uint8_t cmdLen = 0;

byte voltAlarm;  // Alarm holder for internal voltage alarms, trigger 4 volts

float boardVoltage;
int i2cErrorCount;

#ifdef FRSKY
//==========================================================//
//            Global Variable Battery  System               //
//==========================================================//

//---------Public variable of Battery-------------
unsigned char Frsky_Count_Order_Batt;
long Frsky_Batt_Volt_A; 
static byte Batt_Cell_Detect=0;
float Batt_Volte_Backup;
byte Batt_SR_Select;


//--------Define Variable for caculator Percent  LED_Alart 
//#define Batt_Percent_Alert  15

static float Batt_Volt_Cell6_Config = 18;
static float Batt_Volt_Cell5_Config = 15;
static float Batt_Volt_Cell4_Config = 12;
static float Batt_Volt_Cell3_Config = 9;

// Moved to alarm calculations by jp, 061113
// #define Batt_Cell6_Volt_Alert     (Batt_Volt_Cell6_Config+((Batt_Volt_Cell6_Config/100)*Batt_Percent_Alert))
// #define Batt_Cell5_Volt_Alert     (Batt_Volt_Cell5_Config+((Batt_Volt_Cell5_Config/100)*Batt_Percent_Alert))
// #define Batt_Cell4_Volt_Alert     (Batt_Volt_Cell4_Config+((Batt_Volt_Cell4_Config/100)*Batt_Percent_Alert))
// #define Batt_Cell3_Volt_Alert     (Batt_Volt_Cell3_Config+((Batt_Volt_Cell3_Config/100)*Batt_Percent_Alert))




//---------Public Data Struct of Battery ---------
struct{  //Status register battery
  boolean Plugin_Frist : 1;  
  boolean Buckup_EEP : 1;  // Enable save data to eeprom 
  boolean Batt_SR2 : 1;   
  boolean Batt_SR3 : 1;    
  boolean Batt_SR4 : 1;
  boolean Batt_SR5 : 1;
  boolean Batt_SR6 : 1;     
  boolean Batt_SR7 : 1;     
} Batt_SR;  //Status Flag of Battery


//===================|Battery System|======================//




//==========================================================//
//            Globle Variable Altitude  System              //
//==========================================================//

struct Altitude_Status{
  boolean En_Alt:1;
}Alti_SR;

//===================|Altitude System|======================//
#endif

byte ledState;
byte baseState;  // Bit mask for different basic output LEDs like so called Left/Right 

//byte debug = 1;  // Shoud not be activated on repository code, only for debug
//byte deb2 = 1;

//byte ANA;
//byte bVER = 10;

// Objects and Serial definitions
FastSerialPort0(Serial);

//SimpleTimer  mavlinkTimer;

#ifdef LED_STRIP

#define NEO_PIN1 14
#define NEO_PIN2 15
#define NEO_PIN3 16
#define NEO_PIN4 17
CRGB ledrgb[32];
#endif


#ifdef FRSKY
SoftwareSerial frSerial(6,5,true);     // Initializing FrSky Serial on pins 5,6 and Inverted Serial
#endif

#ifdef SERDB
SoftwareSerial dbSerial(12,11);        // For debug porposes we are using pins 11, 12
#define DPL if(debug) dbSerial.println
#define DPN if(debug) dbSerial.print
byte debug = 1;
#else
#define DPL if(debug) {}
#define DPN if(debug) {}
byte debug = 0;
#endif
/*
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 32) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

*/

/* **********************************************/
/* ***************** SETUP() *******************/

void setup() 
{
  
  // Before all, set debug level if we have any. Comment out if not
debug = 4;  
#ifdef LED_STRIP
  ledStripInit();
  ledStripEnable();
  setBrightness(32);
  //while(1) rainbowCycle(5);
#endif
  
  // Initialize Serial port, speed
  Serial.begin(TELEMETRY_SPEED);


#ifdef SERDB
  // Our software serial is connected on pins D6 and D5
  dbSerial.begin(38400);                    // We don't want to too fast
  DPL(F("Debug port Open"));
  DPL(F("No input from this serialport.  "));
  if(analogRead(A6) == 1023) DPL(F("Running IOBoard v1.1"));
#endif
#ifdef JD_IO
  if(analogRead(A6) == 1023) {
    bVER=11;
    Out[0] = 0;
    Out[1] = 7;
    Out[2] = 8;
    Out[3] = 9;
    Out[4] = 10;
    Out[5] = 3;
    Out[6] = 4;
  } else {
    bVER=10;
    Out[0] = 0;
    Out[1] = 8;
    Out[2] = 9;
    Out[3] = 10;
    Out[4] = 4;
    Out[5] = 3;
    Out[6] = 2;
  }    
#endif  
#ifdef FRSKY
  frSerial.begin(9600);
#endif

#ifdef HWRESET
  digitalWrite(13, HIGH);      // Let's put PULLUP high in pin 13 to avoid accidental erases
  if(digitalRead(13) == 0) {    
    DPL(F("Force erase pin LOW, Eracing EEPROM"));
    DPN(F("Writing EEPROM..."));
    writeFactorySettings();
    DPL(F(" done."));
  }
#endif    
  
  // Check that EEPROM has initial settings, if not write them
  if(readEEPROM(CHK1) + readEEPROM(CHK2) != CHKVER) {
#ifdef DUMPEEPROMTELEMETRY
    Serial.print(CHK1);
    Serial.print(F(","));
    Serial.print(CHK2);
    Serial.print(F(","));
    Serial.println(CHKVER);
#endif    
    // Write factory settings on EEPROM
    Serial.println(F("Factory Reset."));
    DPN(F("Writing EEPROM..."));
    writeFactorySettings();
    DPL(F(" done."));
  }
/* 
  if(readEEPROM(FACTORY_RESET)) {
#ifdef DUMPEEPROMTELEMETRY
    Serial.print(F("Factory reset flag: "));
    Serial.print(FACTORY_RESET);
#endif    
    // Write factory settings on EEPROM
    DPN(F("Writing EEPROM..."));
    writeFactorySettings();
    DPL(F(" done."));
  }
*/    
 
#ifdef DUMPEEPROM
  // For debug needs, should never be activated on real-life
  for(int edump = 0; edump <= 24; edump ++) {
   DPN(F("EEPROM SLOT: "));
   DPN(edump);
   DPN(F(" VALUE: "));
   DPL(readEEPROM(edump));     
  }

  // For debug needs, should never be activated on real-life
  for(int edump = 60; edump <= 80; edump ++) {
   DPN(F("EEPROM SLOT: "));
   DPN(edump);
   DPN(F(" VALUE: "));
   DPL(readEEPROM(edump));     
  }
#endif

#ifdef DUMPEEPROMTELEMETRY
  // For debug needs, should never be activated on real-life
  Serial.println();
  for(int edump = 0; edump <= 180; edump ++) {
   Serial.print(F("EEPROM SLOT: "));
   Serial.print(edump);
   Serial.print(F(" VALUE: "));
   Serial.println(readEEPROM(edump), DEC);     
  }
  for(int edump = 1000; edump <= 1023; edump ++) {
   Serial.print(F("EEPROM SLOT: "));
   Serial.print(edump);
   Serial.print(F(" VALUE: "));
   Serial.println(readEEPROM(edump), DEC);     
  }  
#endif

  // setup mavlink port
  mavlink_comm_0_port = &Serial;

#ifdef FRSKY
  // FrSky Bridge active or not
  isFrSky = readEEPROM(ISFRSKY);
  // Percentage value to calculate Low Battery Alarm  
  BattAlarmPercentage = readEEPROM(BatteryAlarm_Percentage_ADDR);    
#endif

#ifdef JD_IO    
  // Rear most important values from EEPROM to their variables  
  LEFT = readEEPROM(LEFT_IO_ADDR);    // LEFT output location in Out[] array
  RIGHT = readEEPROM(RIGHT_IO_ADDR);  // RIGHT output location in Out[] array
  FRONT = readEEPROM(FRONT_IO_ADDR);  // FRONT output location in Out[] array
  REAR = readEEPROM(REAR_IO_ADDR);    // REAR output location in Out[] array
  ledPin = readEEPROM(LEDPIN_IO_ADDR);
  
  // Initializing output pins
  for(int looper = 0; looper <= 5; looper++) {
    pinMode(Out[looper],OUTPUT);
  }

  // Initial startup LED sequence
  for(int loopy = 0; loopy <= 5; loopy++) {
   SlowRoll(25); 
  }

  // Initial startup LED sequence continues
  for(int loopy = 0; loopy <= 2 ; loopy++) {
    AllOn();
    delay(100);
    AllOff();
    delay(100);
  }

  // Activate Left/Right lights
  updateBase();
#endif

#ifdef LED_STRIP
  // Read led strip configs from EEPROM
  lowBattPct = readEEPROM(LOWBATT_PCT);
  lowBattVolt = readEP16(LOWBATT_VOLT) / 1000.0f;
  readStruct(LED_CONFIGS, (uint8_t*)ledConfigs, sizeof(ledConfigs));  
  readStruct(COLOR_CONFIGS, (uint8_t*)colors, sizeof(colors));
  //readStruct(MODE_CONFIGS, (uint8_t*)modeColors, sizeof(modeColors));
#endif


  Serial.println(F("Mavpixel " VER " initialised."));


  // Jani's debug stuff  
#ifdef membug
  DPN(freeMem());
  DPL(F(" bytes free RAM."));
#endif

  Serial.println(F("Press <Enter> 3 times for CLI."));
  

  // Startup MAVLink timers, 50ms runs
  // this affects pattern speeds too.
//  mavlinkTimer.Set(&OnMavlinkTimer, 50);

  // House cleaning, enable timers
//  mavlinkTimer.Enable();
  
  // Enable MAV rate request, yes always enable it for in case.   
  // if MAVLink flows correctly, this flag will be changed to DIS
  enable_mav_request = DI;  
  
  // for now we are always active, maybe in future there will be some
  // additional features like light conditions that changes it.
  isActive = EN;  
  //DPL("End of setup");
  
#ifdef FRSKY
  //--------Initail Para Battery Systems----------//
  Batt_SR.Plugin_Frist=FALSE;  //start plugin vcc--> board  
  Batt_Cell_Detect=0;
  Batt_SR.Buckup_EEP=0;
  
   Batt_SR_Select=readEEPROM(Batt_SR_ADDR);
   if(bitRead(Batt_SR_Select,7)) //Flag Plugin Frist is set
   {
     Batt_SR.Plugin_Frist=TRUE;
     Batt_Cell_Detect=readEEPROM(Batt_DR_ADDR);
   }
   else
   {
     Batt_SR.Plugin_Frist=0;
   }
    
  
  //-------Initial Para Operat Altiude-----------//
  Alti_SR.En_Alt=0;  //Wait!! Arm==1
  iob_alt = 0;       // altitude
#endif    
} // END of setup();



/* ***********************************************/
/* ***************** MAIN LOOP *******************/

// The thing that goes around and around and around for ethernity...
// MainLoop()
void loop() 
{

  
#ifdef HEARTBEAT
  HeartBeat();   // Update heartbeat LED on pin = ledPin (usually D13)
#endif

  if(isActive) { // main loop
    p_curMillis = millis();
    if(p_curMillis - p_preMillis > p_delMillis) {
      // save the last time you blinked the LED 
      p_preMillis = p_curMillis;   
#ifdef JD_IO
      // First we update pattern positions 
      patt_pos++;
      if(patt_pos == 16) {
        patt_pos = 0;
        if(debug == 4) dumpVars(); 
      }
#endif
#ifdef LED_STRIP
      updateLedStrip();
#endif
    }
#ifdef JD_IO
    // Update base lights if any
    updateBase();
    //update Modes status
    digitalWrite(ledPin,le_patt[baseState][patt_pos]);
#endif  
    if(enable_mav_request == 1) { //Request rate control. 
     // DPL("IN ENA REQ");
      // During rate requsst, LEFT/RIGHT outputs are HIGH
      //digitalWrite(LEFT, EN);
      //digitalWrite(RIGHT, EN);

      for(int n = 0; n < 3; n++) {
        request_mavlink_rates();   //Three times to certify it will be readed
        delay(50);
      }
      enable_mav_request = 0;

      // 2 second delay, during delay we still update PWM output
      /*for(int loopy = 0; loopy <= 2000; loopy++) {
        delay(1);
        updatePWM();
      }*/
      waitingMAVBeats = 0;
      lastMAVBeat = millis();    // Preventing error from delay sensing
      //DPL("OUT ENA REQ");
    }  
    
    // Request rates again on every 10th check if mavlink is still dead.
    if(!mavlink_active && !cli_active && messageCounter >= 10) {
    //if(!cli_active && !mavlink_active) {
      DPL(F("Enabling requests again"));
      enable_mav_request = 1;
      messageCounter = 0;
      //LeRiPatt = 6;
    } 

    read_mavlink();
//    mavlinkTimer.Run();
#ifdef JD_IO
    updatePWM();
#endif
#ifdef FRSKY   
    if(isFrSky) update_FrSky();   // if isFrSky = TRUE, Construct FrSky Telemetry packet and send out
    //-----Operat Altiude uadate iob_alt
   if( Alti_SR.En_Alt!=1)   //Wait!! Arm==1
   {
     iob_alt = 0;      // altitude
   }
   
   //-------Operat Battery Display Alarm LED------//
    Batt_Alarm_LED();

   //-------Oparat Save Data Batt backup--------//
   if(Batt_SR.Buckup_EEP==1)
   {
     Batt_SR.Buckup_EEP=0;  //Disable Backup data
     Batt_SR_Select=Batt_SR.Plugin_Frist<<7;
     writeEEPROM(Batt_SR_ADDR,Batt_SR_Select);
     writeEEPROM(Batt_DR_ADDR,Batt_Cell_Detect);
   }
#endif
   
    
  } //else AllOff();

}

/* *********************************************** */
/* ******** functions used in main loop() ******** */

// Function that is called every 120ms
void OnMavlinkTimer()
{
  if(millis() < (lastMAVBeat + 3000)) {
    // General condition checks starts from here
    //

    // Checks that we handle only if MAVLink is active
    if(mavlink_active) {
/*      if(iob_fix_type <= 2) LeRiPatt = ALLOK;
//      if(iob_fix_type <= 2) LeRiPatt = NOLOCK;
      if(iob_fix_type >= 3) LeRiPatt = ALLOK;
  
      // CPU board voltage alarm  
      if(voltAlarm) {
        LeRiPatt = LOWVOLTAGE;  
//        DPL("ALARM, low voltage");
      }*/     
    }
        
    // If we are armed, run patterns on read output
#ifdef JD_IO
    if(isArmed) RunPattern();
     else ClearPattern(); 
    // Update base LEDs  
    //updateBase();

    //DPN(F("MC:")); 
    //DPL(messageCounter);
#endif    

    if(messageCounter >= 20 && mavlink_active) {
      DPL(F("We lost MAVLink"));
      mavlink_active = 0;
      messageCounter = 0;
      //LeRiPatt = NOMAVLINK;
    }
  //  DPL(messageCounter);
 
  // End of OnMavlinkTimer
  } else {
//    DPN("Beats:");
//   DPL(lastMAVBeat);
    waitingMAVBeats = 1;
  }
}

#ifdef JD_IO
void dumpVars() {
 DPN(F("Sats:"));
 DPL(iob_satellites_visible);
 DPN(F("Fix:"));
 DPL(iob_fix_type);
 DPN(F("Modes:"));
 DPL(iob_mode);
 DPN(F("Armed:"));
 DPL(isArmed);
 DPN(F("Thr:"));
 DPL(iob_throttle);
 DPN(F("CPUVolt:"));
 DPL(boardVoltage);
 DPN(F("BatVolt:"));
 DPL(iob_vbat_A);
 DPN(F("Alt:"));
 DPL(iob_alt);
 DPN(F("Hdg:"));
 DPL(iob_heading);
 DPN(F("Spd:"));
 DPL(iob_groundspeed);
 DPN(F("Lat:"));
 DPL(iob_lat);
 DPN(F("Lon:"));
 DPL(iob_lon);
// DPN("Pitch:");
// DPL(iob_pitch);
// DPN("Yaw:");
// DPL(iob_yaw);
// DPN("Roll:");
// DPL(iob_roll);
 DPN(F("Hdop:"));
 DPL(iob_hdop);


 
         /*          DPN("Cell:");
         DPN(Frsky_Batt_Volt_A);
         //DPN("Cell Hex: ");
         //DPN(Frsky_Batt_Volt_A,HEX);
         DPN("Cell Org: ");
         DPN(iob_vbat_A,BIN);*/

        
 
        // DPN("Cell:");
         //DPN(Batt_Volte_Backup);
 
}
#endif