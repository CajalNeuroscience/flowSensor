# flowSensor
Firmware + PC software supporting Sensirion Flow Sensor.

Demonstrated with SLF3S-1300F with many additional compatible products. See the Sensirion Github repo for other members of the same sensor family:

https://github.com/Sensirion/arduino-i2c-sf06-lf

Likely a number of other i2c sensors will work with the same or similar code.  Adapting those is an exercise left to the reader.

Display is a standard SSD1306 OLED.  This is optional for reading the sensor but nice to have as a quick read of sensor status. Can pick these up on various websites for a few USD each. 

Connections are all through the i2c bus of an Arduino Uno.  The .\schematic\Sensirion_flowSensor.sch file shows the required connections (4 pins for each device, plus a couple 10k pull-up resistors. Adding in some connectors makes assembly and maintenance easier; I like the Qwiic connectors from Sparkfun for cheap + small 4 pin ports.

Pay careful attention to the pin order in each of the i2c connections.  The connector type and pin order for an i2c port has no pervasive standard so each device could have the pins in a different order. Worth checking you have all pins connected as expected before going down a different rabbit hole trying to troubleshoot.  

.\Arduino\ has a sketch for an Arduino Uno.  This will run headless with the display reporting flow rates and fault status if you plug it all up and power the Uno over USB.  Code in .\python\ is a simple class to communicate with the sensor via the Uno's USB.  Use this if you'd like to integrate your sensor into a larger system orchestrated by PC. 

 
