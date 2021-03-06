/****************************************************************************************************
* touchIt_SET_ADD.c
*
* Get started with touchIt on Raspberry Pi! Go to www.probots.co.uk for more information.
*
* Configures the touchIt slave address. Once changed you'll have to edit TCHADD1!
*
* Demonstrates: "0x00 - Get Version", "0x02 - Set Address"
*
* Tested on Raspberry Pi Zero. Use "sudo apt-get install i2c-tools" (if necessary) and
*                                  "sudo i2cdetect -y 1" to check it is on the bus (device 0x70)
*
* If you get "no module named smbus", you'll need to "sudo apt-get install python-smbus" also ;)
*
* Written in nano 2.2.6
* Compiled using "gcc -o touchIt_SET_ADD touchIt_SET_ADD.c"
*
* P. Coates, ProBots, 17/09/16
* Free to use, modify, share!
***************************************************************************************************/

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

// touchIt commands
#define GET_VER	0x00
#define SET_CFG	0x01
#define SET_ADD	0x02
#define GET_POS	0x03
#define GET_TCH	0x04

// touchIt config
#define XMOVE	0x01
#define YMOVE 	0x02
#define TMOVE	0x04
#define TAP	0x08

// touchIt address
#define TCHADD1 0x70

// touchIt new address (1-127)
#define TCHADD2 0x72

// GPIO params
#define BUFMAX 	3
#define DIRMAX	35
#define IN	0
#define OUT	1
#define LOW	0
#define HIGH 	1

int file_i2c;
int file_gpio;
int length;
unsigned char buffer[60] = {0};

unsigned char data[10];
unsigned char crc = 0, c = 0;
unsigned char intPin = 17;

/**************************************************
* Get the touchIt Firmware Version
**************************************************/
void getVersion(void)
{
	unsigned char len = 4;

	// Generate crc
	crc  = data[0] = '#';			// Start
	crc ^= data[1] = GET_VER;		// Command
	crc ^= data[2] = 0x00;			// Data Length
	data[3] = crc;

	// Send command
	if(write(file_i2c, data, len) != len)
	{
		printf("getVersion(): Failed to write to the i2c bus.\n");
		return;
	}

	// Receive confirmation
	len = 6;
	if(read(file_i2c, data, len) != len)
	{
		printf("getVersion(): Failed to read from the i2c bus.\n");
	}
	else
	{
		if(data[0] == '$')
		{
			crc = 0;
			for(c=0;c<5;c++)
			{
				crc ^= data[c];
			}

			if(crc == data[5])
			{
				printf("touchIt FW Ver: %i.%i\n", data[3], data[4]);
			}
		}
	}
}

/**************************************************
* Set a new touchIt address
**************************************************/
void setAddress(unsigned char newAdd)
{
	unsigned char len = 5;

	// Generate crc
	crc  = data[0] = '#';			// Start
	crc ^= data[1] = SET_ADD;		// Command
	crc ^= data[2] = 0x01;			// Data length
	crc ^= data[3] = newAdd;		// Data
	data[4] = crc;

	// Send command
	if(write(file_i2c, data, len) != len)
	{
		printf("setAddress(): Failed to write to the i2c bus\n");
		return;
	}

	// No confirmation, address has changed
}

/**************************************************
* GPIO read function for interrupt polling
**************************************************/
unsigned char gpioRead(void)
{
	unsigned char val[3];

	read(file_gpio, val, 3);

	lseek(file_gpio, 0, SEEK_SET);	// restore file pointer for next read

	return(atoi(val));
}

/**************************************************
* Exit method - clean up file handles etc
**************************************************/
void exitHandler(int s)
{
	if(s == SIGINT)
	{
		close(file_gpio);
		close(file_i2c);

		exit(1);
	}
}

/**************************************************
* Configure the Pi / touchIt
**************************************************/
void setup()
{
	unsigned char len = 0;
	unsigned char dir = IN;
	static const char dir_str[] = "in\0out";

	signal (SIGINT, exitHandler);

	// Open the I2C bus (enable file access)
	char *filename = (char *)"/dev/i2c-1";
	if((file_i2c = open(filename, O_RDWR)) < 0)
	{
		printf("setup(): Failed to open the i2c bus\n");
		return;
	}

	if(ioctl(file_i2c, I2C_SLAVE, TCHADD1) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave\n");
		return;
	}

	// Enable GPIO to read touchIt 'int' pin
	if((file_gpio = open("/sys/class/gpio/export", O_WRONLY)) < 0)
	{
		printf("setup(): Failed to open export for writing\n");
		return;
	}
	else
	{
		len = snprintf(buffer, BUFMAX, "%d", intPin);
		write(file_gpio, buffer, len);
		close(file_gpio);
	}

	// Configure GPIO Direction as input
	snprintf(buffer, DIRMAX, "/sys/class/gpio/gpio%d/direction", intPin);
	if((file_gpio = open(buffer, O_WRONLY)) < 0)
	{
		printf("setup(): Failed to open gpio direction for writing\n");
		return;
	}
	else
	{
		write(file_gpio, &dir_str[IN == dir ? 0:3], IN  == dir ? 2:3);
		close(file_gpio);
	}

	// Open gpio file for reading
	snprintf(buffer, DIRMAX, "/sys/class/gpio/gpio%d/value", intPin);
	if((file_gpio = open(buffer, O_RDONLY)) < 0)
	{
		printf("gpioRead(): failed to open gpio value for reading\n");
		return;
	}

	getVersion();

	setAddress(TCHADD2);

	printf("touchIt I2C address changed from %X to %X\n", TCHADD1, TCHADD2);
}

/**************************************************
* Do all the cool stuff
**************************************************/
int main()
{
	setup();

	while(1)
	{
	}
}

