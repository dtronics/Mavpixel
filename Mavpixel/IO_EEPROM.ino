/*
///////////////////////////////////////////////////////////////////////
//
// Please read licensing, redistribution, modifying, authors and 
// version numbering from main sketch file. This file contains only
// a minimal header.
//
// Copyright (c) 2013, Jani Hirvinen, jDrones & Co.
// All rights reserved.
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
//
//   EEPROM Functions for jD-IOBoard.
*/

// EEPROM reader/writers
// Utilities for writing and reading from the EEPROM
byte readEEPROM(int address) {
  return EEPROM.read(address);
}

void writeEEPROM(int address, byte value) {
  EEPROM.write(address, value);
}

void writeEP2(int address, byte val1, byte val2) {
  EEPROM.write(address, val1);
  EEPROM.write(address + 1, val2); 
}

void writeEP16(int address, int value) {
  EEPROM.write(address, (value >> 8) & 0xff);
  EEPROM.write(address + 1, value & 0xff);
}


int readEP16(int address) {
  int value = EEPROM.read(address) << 8;
  value += EEPROM.read(address + 1);
  return value;
}

void writeStruct(int address, uint8_t* pointer, uint16_t size) {
  for (int i = 0; i < size; i++) {
    EEPROM.write(address + i, *pointer++);
  }
}

void readStruct(int address, uint8_t* pointer, uint16_t size) {
  for (int i = 0; i < size; i++) {
    *pointer++ = EEPROM.read(address + i);
  }
}

void writeModeColor(uint8_t mode, uint8_t index, uint8_t color) {
  int loc = MODE_CONFIGS + (mode * 6) + index;
  EEPROM.write(loc, color);
}

uint8_t readModeColor(uint8_t mode, uint8_t index) {
  int loc = MODE_CONFIGS + (mode * 6) + index;
  return EEPROM.read(loc);
}


// Default patterns should look like these:
//  { 0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0  },    // 0   0x00 , 0x00
//  { 1,1,1,1,0,0,0,0 ,1,1,1,1,0,0,0,0  },    // 1   0xF0 , 0xF0    
//  { 1,1,1,1,1,0,0,0 ,0,0,0,0,0,1,0,0  },    // 2   0xF8 , 0x04
//  { 1,1,0,0,1,1,0,0 ,1,1,0,0,1,1,0,0  },    // 3   0xCC , 0xCC
//  { 1,0,0,0,1,0,0,0 ,1,0,0,0,1,0,0,0  },    // 4   0x88 , 0x88
//  { 1,0,1,0,1,0,1,0 ,1,0,1,0,0,0,0,0  },    // 5   0xAA , 0xA0
//  { 1,0,1,0,1,0,1,0 ,1,0,1,0,1,0,1,0  },    // 6   0xAA , 0xAA
//  { 1,0,1,0,0,0,0,0 ,1,0,1,0,0,0,0,0  },    // 7
//  { 1,1,0,0,0,0,0,0 ,1,1,0,0,0,0,0,0  },    // 8
//  { 1,0,0,0,0,0,0,0 ,1,0,0,0,0,0,0,0  },    // 9
//  { 1,0,0,0,0,0,0,0 ,1,0,0,0,0,0,0,0  },    // 10
//  { 1,0,0,0,0,0,0,0 ,1,0,0,0,0,0,0,0  },    // 11
//  { 1,0,0,0,0,0,0,0 ,1,0,0,0,0,0,0,0  },    // 12
//  { 1,0,0,0,0,0,0,0 ,1,0,0,0,0,0,0,0  },    // 13
//  { 1,0,0,0,0,0,0,0 ,1,0,0,0,0,0,0,0  },    // 14
//  { 1,0,0,0,0,0,0,0 ,1,0,0,0,0,0,0,0  }};   // 15



// Write our latest FACTORY settings to EEPROM
void writeFactorySettings() {
 // Writing all default parameters to EEPROM
 
#ifdef DUMPEEPROMTELEMETRY
  Serial.println(F("EEPROM Reset to factory settings")); 
#endif  

 DPL(F("EEPROM Reset to factory settings"));

#ifdef JD_IO
 // Default patterns 1-16, BIN 
 writeEP2(pat01_ADDR, 0b11000000, 0b00000000);
 writeEP2(pat02_ADDR, 0b11110000, 0b11110000);
 writeEP2(pat03_ADDR, 0b11111000, 0b00000100);
 writeEP2(pat04_ADDR, 0b11001100, 0b11001100);
 writeEP2(pat05_ADDR, 0b10001000, 0b10001000);
 writeEP2(pat06_ADDR, 0b10101010, 0b10100000);
 writeEP2(pat07_ADDR, 0b10101010, 0b10101010);
 writeEP2(pat08_ADDR, 0b10100000, 0b10100000);
 writeEP2(pat09_ADDR, 0b11000000, 0b11000000);
 writeEP2(pat10_ADDR, 0b10000000, 0b10000000);
 writeEP2(pat11_ADDR, 0b10000000, 0b10000000);
 writeEP2(pat12_ADDR, 0b10000000, 0b10000000);
 writeEP2(pat13_ADDR, 0b10000000, 0b10000000);
 writeEP2(pat14_ADDR, 0b10000000, 0b10000000);
 writeEP2(pat15_ADDR, 0b10000000, 0b10000000);
 writeEP2(pat16_ADDR, 0b10000000, 0b10000000);

 // Default patterns 17-32
 // Reserved, write full NULL, BIN 
 writeEP2(pat17_ADDR, 0b00000000, 0b00000000);
 writeEP2(pat18_ADDR, 0b00000000, 0b00000000);
 writeEP2(pat19_ADDR, 0b00000000, 0b00000000);
 writeEP2(pat20_ADDR, 0b00000000, 0b00000000);
 writeEP2(pat21_ADDR, 0b00000000, 0b00000000);
 writeEP2(pat22_ADDR, 0b00000000, 0b00000000);
 writeEP2(pat23_ADDR, 0b00000000, 0b00000000);
 writeEP2(pat24_ADDR, 0b00000000, 0b00000000);
 writeEP2(pat25_ADDR, 0b00000000, 0b00000000);
 writeEP2(pat26_ADDR, 0b00000000, 0b00000000);
 writeEP2(pat27_ADDR, 0b00000000, 0b00000000);
 writeEP2(pat28_ADDR, 0b00000000, 0b00000000);
 writeEP2(pat29_ADDR, 0b00000000, 0b00000000);
 writeEP2(pat30_ADDR, 0b00000000, 0b00000000);
 writeEP2(pat31_ADDR, 0b00000000, 0b00000000);
 writeEP2(pat32_ADDR, 0b00000000, 0b00000000);
 
 // Default pattern - flight mode binds, HEX
 writeEP2(mbind01_ADDR, 0x00, 0xA1);
 writeEP2(mbind02_ADDR, 0x01, 0xA2);
 writeEP2(mbind03_ADDR, 0x02, 0xA3);
 writeEP2(mbind04_ADDR, 0x03, 0xA4);
 writeEP2(mbind05_ADDR, 0x04, 0xA5);
 writeEP2(mbind06_ADDR, 0x05, 0xA6);
 writeEP2(mbind07_ADDR, 0x06, 0xA7);
 writeEP2(mbind08_ADDR, 0x07, 0xA8);
 writeEP2(mbind09_ADDR, 0x08, 0xA9);
 writeEP2(mbind10_ADDR, 0x09, 0xAA);
 writeEP2(mbind11_ADDR, 0x0A, 0xAB);
 writeEP2(mbind12_ADDR, 0x0B, 0xAC);
 writeEP2(mbind13_ADDR, 0x0C, 0xAD);
 writeEP2(mbind14_ADDR, 0x0D, 0xAE);
 writeEP2(mbind15_ADDR, 0x0E, 0xAF);
 writeEP2(mbind16_ADDR, 0x0F, 0xBF);



 // Default locations of different output LEDs, DEC
 writeEEPROM(LEFT_IO_ADDR, 1);   // Was direct IOPIN 8, now using order number 
 writeEEPROM(RIGHT_IO_ADDR, 2);  // Was direct IOPIN 4
 writeEEPROM(FRONT_IO_ADDR, 3);  // Was direct IOPIN 9
 writeEEPROM(REAR_IO_ADDR, 4);   // Was direct IOPIN 7  (v1.0) PinOld 10
 writeEEPROM(FLASH_IO_ADDR, 0);    // Future extra flasher 
 writeEEPROM(LEDPIN_IO_ADDR, 10);  // HeartBeat LEDPIN, should be 13 

 writeEEPROM(ISFRSKY, 1);           // Activate FrSky protocol output from D5, D6. By default yes
 
 writeEEPROM(BatteryAlarm_Percentage_ADDR, 15);  // Percentage to trigger battery alarm, default minvoltage + 15% 

#endif

#ifdef LED_STRIP
  writeStruct(LED_CONFIGS, (uint8_t*)ledConfigs, sizeof(ledConfigs));  
  writeStruct(COLOR_CONFIGS, (uint8_t*)colors, sizeof(colors));
  writeModeColorsDefault();
#endif

 writeEEPROM(STRIP_IO_ADDR, 7);
 writeEP16(MAVLINK_BAUD, 57600);
 writeEEPROM(LOWBATT_PCT, 20);
 writeEP16(LOWBATT_VOLT, 3300); //*1000

 // Write details for versioncheck to EEPROM
 writeEEPROM(CHK1, 22);
 writeEEPROM(CHK2, 21);
 writeEEPROM(VERS, CHKVER);
 
 // Factory reset request flag 
 writeEEPROM(FACTORY_RESET, 0);
}