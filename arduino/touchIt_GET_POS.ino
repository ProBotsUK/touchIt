/****************************************************************************************************
* touchIt_GET_POS
*
* Get started with touchIt on Arduino! Go to www.probots.co.uk for more information.
*
* Configures touchIt to interrupt in response to touches on in its x and y axis, and outputs the
* Fine x/y data over serial. Uses basic polling for data collection.
*
* Demonstrates: "0x00 - Get Version", "0x01 - Set Configuration", "0x03 - Get Position"
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

// touchIt address
#define TCHADD1  0x70

unsigned char data[10];
unsigned char crc = 0, c = 0;

/**************************************************
* Get the touchIt Firmware Version
**************************************************/
void getVersion(void)
{ 
  // Generate crc
  crc =  data[0] = '#';              // Start
  crc ^= data[1] = GET_VER;          // Command
  crc ^= data[2] = 0x00;             // Data length
  data[3] = crc;                     // Calculated crc
  
  // Send command
  Wire.beginTransmission(TCHADD1);
  Wire.write(data, 4);
  Wire.endTransmission();
  
  // Receive confirmation
  Wire.beginTransmission(TCHADD1);
  Wire.requestFrom(TCHADD1, 6);
  
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
* Configure the events which cause an interrupt
**************************************************/
void setConfig(unsigned char config)
{ 
  // Generate crc
  crc =  data[0] = '#';              // Start
  crc ^= data[1] = SET_CFG;          // Command
  crc ^= data[2] = 0x01;             // Data length
  crc ^= data[3] = config;           // Data
  data[4] = crc;                     // Calculated crc
  
  // Send command
  Wire.beginTransmission(TCHADD1);
  Wire.write(data, 5);
  Wire.endTransmission();
  
  // Receive confirmation
  Wire.beginTransmission(TCHADD1);
  Wire.requestFrom(TCHADD1, 5);
  
  c = 0;
  while(Wire.available())
  {
    data[c++] = Wire.read();
  }
  
  // Validate returned crc
  if(data[0] == '$')
  {
    crc = 0;
    for(c=0;c<4;c++)
    {
      crc ^= data[c];
    }
    
    if(crc == data[4])
      Serial.print("Config Write OK!\n\n");
    else
      Serial.print("Config Write Failed\n\n");
  }
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
  
  getVersion();
  
  // Interrupt on X/Y movement
  setConfig(XMOVE | YMOVE);
  // Interrupt on X/Y tap
  //setConfig(TAP);
}

/**************************************************
* Do all the cool stuff
**************************************************/
void loop() 
{ 
  unsigned short pos = 0x0000;
  
  // Let's get Fine X/Y data when someone touches the sensor 
  if(0 == digitalRead(8))            // touchIt Interrupt output pulled low
  {
    // Generate crc
    crc =  data[0] = '#';            // Start                         
    crc ^= data[1] = GET_POS;        // Command
    crc ^= data[2] = 0x00;           // Data Length (no data)
    data[3] = crc;                   // Calculated crc
   
    // Send command
    Wire.beginTransmission(TCHADD1); 
    Wire.write(data, 4);
    Wire.endTransmission();                                     
    
    // Receive confirmation
    Wire.beginTransmission(TCHADD1);
    Wire.requestFrom(TCHADD1, 8);   
   
    c = 0;    
    while(Wire.available())
    {
      data[c++] = Wire.read();
    }
   
    // Validate returned crc
    if(data[0] == '$')
    {
      crc = 0;
      for(c=0;c<7;c++)
      {
        crc ^= data[c];
      }
      
      if(crc == data[7])            
      {
        pos = data[3];
        pos <<= 8;
        pos += data[4];
      
        // Display X position
        Serial.print(pos);
        Serial.write('\t');
      
        pos = data[5];
        pos <<= 8;
        pos += data[6];
      
        // Display Y position
        Serial.print(pos);
        Serial.write('\n');
      }
    }
  }
}
