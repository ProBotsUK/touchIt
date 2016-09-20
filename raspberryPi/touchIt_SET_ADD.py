#****************************************************************************************************
# touchIt_SET_ADD.py
#
# Get started with touchIt on Raspberry Pi! Go to www.probots.co.uk for more information.
#
# Configures the touchIt slave address. Once changed you'll have to edit TCHADD1!
#
# Demonstrates: "0x00 - Get Version", "0x02 - Set Address"
#
# Tested on Raspberry Pi Zero. Use "sudo apt-get install i2c-tools" (if necessary) and
#                                  "sudo i2cdetect -y 1" to check it is on the bus (device 0x70)
#
# If you get "no module named smbus", you'll need to "sudo apt-get install python-smbus" also ;)
#
# Written in Python 2.7.9
#
# P. Coates, ProBots, 17/09/16
# Free to use, modify, share!
#***************************************************************************************************

# Needed for I2C + GPIO
import time
import smbus
import struct
import RPi.GPIO as GPIO

DEVICE_BUS = 1                      # using pins 3 and 5 on Raspberry PI Zero
intPin = 17                         # using BCM 17 for touchIt interrupt output (pin 11)
bus = smbus.SMBus(DEVICE_BUS)

# touchIt commands
GET_VER = 0
SET_CFG = 1
SET_ADD = 2
GET_POS = 3
GET_TCH = 4

# touchIt config
XMOVE = 0x01
YMOVE = 0x02
TMOVE = 0x04
TAP   = 0x08

# touchIt address
TCHADD1 = 0x70

# touchIt new address (1-127)
TCHADD2 = 0x72

crc = 0
c = 0

#**************************************************
# Get the touchIt Firmware Version
#**************************************************
def getVersion():

    # Generate crc
    data = []
    data.append('#')                # Start
    data.append(chr(GET_VER))       # Command
    data.append(chr(0))             # Data Length

    crc = 0
    for x in range(len(data)):
        crc ^= ord(data[x])         # Calculated crc

    # Send command
    # note: no target register so we're sticking our first byte '#' in its place
    bus.write_i2c_block_data(TCHADD1, ord('#'), [GET_VER, 0, crc])
    time.sleep(0.6)                 # prevents smbus missing return data

    # Receive confirmation
    data = bus.read_i2c_block_data(TCHADD1, 0, 6)
    
    # Validate returned crc
    if data[0] == ord('$'):
        crc = 0

        for c in range(5):          # Calculate crc from incoming data (this command should return 5 bytes)
            crc ^= data[c]

        if crc == data[5]:          # check against received crc
            print "touchIt FW Ver: " + str(data[3]) + '.' + str(data[4])

#**************************************************
# Set a new touchIt address
#**************************************************
def setAddress(newAdd):

    # Generate crc
    data = []
    data.append('#')                # Start
    data.append(chr(SET_ADD))       # Command
    data.append(chr(1))             # Data Length
    data.append(chr(newAdd))        # Data

    crc = 0
    for x in range(len(data)):
        crc ^= ord(data[x])         # Calculated crc

    # Send command (using current address)
    bus.write_i2c_block_data(TCHADD1, ord('#'), [SET_ADD, 1, newAdd, crc])
    time.sleep(0.6)
    
    # No confirmation, address has changed

#**************************************************
# Configure the pi and touchIt
#**************************************************

# Configure input for reading the touchIt interrupt state
GPIO.setmode(GPIO.BCM)              # Use Broadcom pin-numbering scheme
GPIO.setup(intPin, GPIO.IN)

# Get touchIt FW version
getVersion()

# Interrupt on X/Y movement
setAddress(TCHADD2)

print "touchIt I2C address changed from %s to %s" % (hex(TCHADD1), hex(TCHADD2))
