import serial
import time

def str_to_intarray(str_in):
    intarray = []
    for c in str_in:
        intarray.append(ord(c))
    return intarray

def valid_checksum(data_array):
    if len(data_array) != 10: # check if size is rigth
        return False
    
    sum = reduce((lambda a,b: a+b),data_array[2:8]) & 0xFF
    return True if sum == data_array[8] else False

class Monitor:
    pm25 = 0
    pm10 = 0
    new_data = False

    ser = serial.Serial('/dev/ttyAMA0',9600)

    def update(self):
        print("Updating...")
        #self.ser.write(b'\xAA\xB4\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xA1\x60\x05\xAB')
        #self.ser.write(b'\xAA\xB4\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xFF\xFF\x00\xAB')
        # Note: Set to query data mode.
        self.ser.write(b'\xAA\xB4\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xFF\xFF\x02\xAB')
        data = self.ser.read(10)
        data = str_to_intarray(data)
        if valid_checksum(data): # Valid command received
            if data[1] == 0xC0:
                self.pm25 = (data[3]*256) + (data[2]/10.0)
                self.pm10 = (data[5]*256) + (data[4]/10.0)
                self.new_data = True

    def has_new_data(self):
        response = self.new_data
        self.new_data = False
        return response

    def cleanup(self):
        self.ser.close()

    def get_pm25(self):
        return self.pm25

    def get_pm10(self):
        return self.pm10
