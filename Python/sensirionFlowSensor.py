"""
Created on Wed Jun 8 16:35:16 2022

@author: Rusty Nicovich

Sensirion flow sensor Python class

Supports functionality of Sensirion flow sensor through sensirionFlowSensor.ino sketch. 

"""

returnMessages = {
    'CMD_NOT_DEFINED': 0x15,
    'DeviceID' : 'flowSensor',
    'ENCODING' : 'utf8'
    }


class flowSensorDriver():
    """
    Class for communicating with NicoLase Arduino Shield
    
    """
    def __init__(self, serialDevice, verbose = False):
        
        self.serial = serialDevice
        self.verbose = verbose

        # Once connected, check that port actually has NicoLase on the receiving end
        devID = self.getIdentification()
        if devID == returnMessages['DeviceID']:
            if self.verbose:
                print('Connected to NicoLase on port ' + self.serial.port)
        else:
            print("Initialization error! Disconnecting!\n")
            self.serial.close()
            

    def writeAndRead(self, sendString):
        """
        Helper function for sending to serial port, returning line
        """
        self.serial.reset_input_buffer()
        self.serial.reset_output_buffer()
        sendString = sendString + '\n'
        self.serial.write(sendString.encode(returnMessages['ENCODING']))
        ret = self.serial.readline().decode(returnMessages['ENCODING']).strip()
        
        if self.verbose:
            print(ret)
            
        return ret

        
    # B
    def queryBubbbleFault(self):
        """ 
        Set pre-collection sequence
        Input seq can be any numerical form (binary, hex, or decimal)
        """
        rtn = self.writeAndRead('B')
        return rtn
        
    # F
    def getFlowRate(self):
        """ 
        Set sequence to 'all on'
        """
        rtn = self.writeAndRead('F')
        return rtn

    
    # T
    def getTemp(self):
        """ 
        Cycle outputs through main sequence
        One count per pattern
        """
        rtn = self.writeAndRead('T')
        return rtn

        
    # W
    def queryFlowFault(self):
        """ 
        Clear existing and set singular sequence, then turn on
        Command when switching 'channels' directly over USB
        """
        rtn = self.writeAndRead('W')
        return rtn

        
    # Y 
    def getIdentification(self):
        """ identification query """
        idn = self.writeAndRead('Y')
        return idn
        


if __name__ == '__main__':
    print("Testing Senririon flow sensor...")
    
    import serial
    import time
    
    try:
        ser = serial.Serial('COM10', baudrate = 115200)
        time.sleep(5)
        
        
        fS = flowSensorDriver(ser)
        
        print("Device ID : {}".format(fS.getIdentification()))
        print("Flow rate : {}".format(fS.getFlowRate()))
        print("Temp : {}".format(fS.getTemp()))
        print("Bubble Fault : {}".format(fS.queryBubbbleFault()))
        print("Flow Fault : {}".format(fS.queryFlowFault()))
    finally:
    
        fS.serial.close()
    

