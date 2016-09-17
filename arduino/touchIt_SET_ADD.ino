/****************************************************************************************************
* touchIt_SET_ADD
*
* Get started with touchIt on Arduino! Go to www.probots.co.uk for more information.
*
* Configures the touchIt slave address.
*
* Demonstrates: "0x00 - Get Version", "0x02 - Set Address"
*
* Tested on Arduino Nano, Arduino Mega2560
*
* P. Coates, ProBots, 29/06/16
* Free to use, modify, share!
****************************************************************************************************/

// Needed for I2C
#include <Wire.h>

// touchIt commands
#define GET_VER  0x00
#define SET_CFG  0x01
#define SET_ADD  0x02
#define GET_POS  0x03
#define GET_TCH  0x04

// touchIt config
#define XMOVE    0x01
#define YMOVE    0x02
#define TMOVE    0x04
#define TAP      0x08

// touchIt default address
#define TCHADD1  0x70

// touchIt new address (1-127)
#define TCHADD2  0x71

unsigned char data[10];
unsigned char crc = 0, c = 0;

/**************************************************
* Get the touchIt Firmware Version
**************************************************/
void getVersion(unsigned char Add)
{ 
  // Generate crc
  crc =  data[0] = '#';              // Start
  crc ^= data[1] = GET_VER;          // Command
  crc ^= data[2] = 0x00;             // Data length
  data[3] = crc;                     // Calculated crc
  
  // Send command
  Wire.beginTransmission(Add);
  Wire.write(data, 4);
  Wire.endTransmission();
  
  // Receive confirmation
  Wire.beginTransmission(Add);
  Wire.requestFrom(Add, 6);
  
  c = 0;
  while(Wire.available())
  {
    data[c++] = Wire.read();
  }
  
  // Validate returned crc
  if(data[0] == '$')
  {
    crc = 0;
    for(c=0;c<5;c++)
    {
      crc ^= data[c];
    }
    
    if(crc == data[5])
    {
      Serial.write("touchIt FW Ver: ");
      Serial.print(data[3]);
      Serial.print('.');
      Serial.print(data[4]);
      Serial.print('\n');
    }
  }
}

/**************************************************
* Set a new touchIt address
**************************************************/
void setAddress(unsigned char oldAdd, unsigned char newAdd)
{ 
  // Generate crc
  crc =  data[0] = '#';              // Start
  crc ^= data[1] = SET_ADD;          // Command
  crc ^= data[2] = 0x01;             // Data length
  crc ^= data[3] = newAdd;           // Data
  data[4] = crc;                     // Calculated crc
  
  // Send command
  Wire.beginTransmission(oldAdd);
  Wire.write(data, 5);
  Wire.endTransmission();
}

/**************************************************
* Configure the arduino
**************************************************/
void setup() 
{
  // Configure input for reading the touchIt interrupt state
  pinMode(8, INPUT);
  
  // Initialise I2C 
  Wire.begin();
  
  // Initialise Serial for debugging
  Serial.begin(9600);
  
   // Set new address
  setAddress(TCHADD1, TCHADD2);
  
  delay(5000);
  
  // Get Firmware Version (test comms with new address)
  getVersion(TCHADD2);
  
  // Don't forget to set touchIt config correctly!
}

/**************************************************
* Do all the cool stuff
**************************************************/
void loop() 
{ 
}
