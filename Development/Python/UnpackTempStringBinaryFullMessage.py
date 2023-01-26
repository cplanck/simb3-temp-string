# This script opens an SBD binary file and converts the binary to temperatures to display
# It's used to validate the packing and unpacking of the values from the SIMB3 One-Wire Temperature String
# Values output from this code should match those in the serial console on SIMB3 transmission
# Code was validated for positive and negative binary numbers from the DS28EA00 datasheet

# Note: the packing sequence which this code undoes is defined in BitPacking.xlsx

# Written 8 December 2022 by Cameron Planck

import collections
import csv
import datetime
import os
import struct

from binaryreader import *


def applyBitmap(binary_list):
    """ Applies the bitmap from the DS28EA00 datasheet to derive the temperature from the bits.
    Page 8 of https://datasheets.maximintegrated.com/en/ds/DS28EA00.pdf """

    convertedTemp = ((binary_list[5]*2**6) + (binary_list[6]*2**5) + (binary_list[7]*2**4) + (binary_list[8]*2**3) + (binary_list[9]*2**2) + (
        binary_list[10]*2**1) + (binary_list[11]*2**0) + (binary_list[12]*2**-1) + (binary_list[13]*2**-2) + (binary_list[14]*2**-3) + (binary_list[15]*2**-4))

    if binary_list[4] == 1:
        convertedTemp = -convertedTemp

    return convertedTemp


def convertNegativeTemps(binary):
    """ Takes a 16-bit raw negative binary value and converts it to temperature by undoing two's complement """

    binary = int(binary) - 1
    binary_str = str(bin(binary))
    binary_str = binary_str[2:].zfill(16)

    binary_str_inv = ''
    for item in binary_str:

        if item == '1':
            binary_str_inv = binary_str_inv + '0'
        else:
            binary_str_inv = binary_str_inv + '1'

    binary_list = [int(x) for x in binary_str_inv]
    convertedTemp = applyBitmap(binary_list)
    return -convertedTemp


def convertPositiveTemps(binary):
    """ Takes a 16-bit raw positive binary value and converts it to temperature """

    binary_str = str(bin(binary))
    binary_str = binary_str[2:].zfill(16)
    binary_list = [int(x) for x in binary_str]
    convertedTemp = applyBitmap(binary_list)

    return convertedTemp


def unpackBinary(temp_bytes, num_sensors):
    """ Function for unpacking the SIMB3 One-Wire Temperature String binary.
            Input: the chunk of packed temperature string binary from the SIMB3 SBD message
            Returns: a list of converted temperature values """

    # walk through each packed block (there are two temperature readings per block)
    # 80 total sensors in a string, so run this 40 times
    temperatures = []
    i = 0
    for k in range(0, int(num_sensors/2)):

        byteString = format(temp_bytes[i], 'b').zfill(8)

        # first value
        Byte1 = temp_bytes[i]
        Byte2 = (temp_bytes[i+1])

        # isolate Most and Least Significant Bits and combine into a 16-bit number (TempBin)
        LSB1 = Byte1
        MSB1 = (Byte2 & 0b11110000) >> 4
        MSB1 = (MSB1 & 0b0000000011111111)
        MSB1 = (MSB1 << 8)
        TempBin = MSB1 | LSB1

        if TempBin & 0x800 != 0:  # checks if the sign bit is set to 1
            temperature = convertNegativeTemps(TempBin)
        else:
            temperature = convertPositiveTemps(TempBin)

        temperatures.append(temperature)

        # second value
        Byte3 = temp_bytes[i+2]

        LSB2 = Byte3
        MSB2 = Byte2
        MSB2 = (Byte2 & 0b00001111)
        MSB2 = (MSB2 & 0b0000000000001111)
        MSB2 = (MSB2 << 8)
        TempBin = MSB2 | LSB2

        if TempBin & 0x800 != 0:  # checks if the sign bit is set to 1
            temperature = convertNegativeTemps(TempBin)
        else:
            temperature = convertPositiveTemps(TempBin)

        temperatures.append(temperature)

        i = i+3
    return temperatures


# path to the satellite file to download
sbd_path = 'test-data/300434064562590_000364.sbd'
# sbd_path = 'test-data/300434063384820_000006.sbd1563928758000'

# with open(sbd_path, mode='rb') as file: # b is important -> binary
#     fileContent = file.read()

# data = open(sbd_path, "rb").read()
# print(struct.calcsize(data))

# (eight, N) = struct.unpack("@iiiiiiii", data)
# print(N)
# print(eight)

# print(fileContent)

binaryReader = BinaryReader(sbd_path)

data = collections.OrderedDict()   
data['wdt_counter'] = binaryReader.read('uint8')
data['program_version'] = binaryReader.read('uint8')
data['time_stamp'] = binaryReader.read('int32')
data['latitude'] = binaryReader.read('int32')/1000000.0
data['longitude'] = binaryReader.read('int32')/1000000.0
data['air_temp'] = binaryReader.read('int16')*0.0625
data['air_pressure'] = binaryReader.read('uint16')/10.0
data['bottom_distance'] = binaryReader.read('uint16')/100.0
data['water_temp'] = binaryReader.read('int16')/100.0
data['surface_distance'] = binaryReader.read('uint16')/1000.0
data['extra_1'] = binaryReader.read('int16')
data['extra_2'] = binaryReader.read('int16')
data['extra_3'] = binaryReader.read('int16')
data['battery_voltage'] = binaryReader.read('uint16')/100.0
data['gps_satellites'] = binaryReader.read('uint8')
data['iridium_signal'] = binaryReader.read('uint8')
data['iridium_retries'] = binaryReader.read('uint8')

# read bytes in as unsigned integers
top_temp_bytes = []
for byte in range(0,120):
	top_temp_bytes.append(binaryReader.read('uint8'))

bottom_temp_bytes = []
for byte in range(0,120):
	bottom_temp_bytes.append(binaryReader.read('uint8'))

top_temperatures = unpackBinary(top_temp_bytes, 80)

print('WDT: {}'.format(data['wdt_counter']))

print('Converted Time stamp: {}'.format(datetime.datetime.fromtimestamp(data['time_stamp'])))

print('Excel Time stamp: {}'.format(float(data['time_stamp'])*1000/60.0/60.0/24.0/1000.0+25569.0))

print('Air Temperature: {}'.format(data['air_temp']))

print('Air Pressure: {}'.format(data['air_pressure']))

print('Latitude: {}'.format(data['latitude']))

print('Longitude: {}'.format(data['longitude']))

print('Bottom Distance: {}'.format(data['bottom_distance']))

print('Water Temp: {}'.format(data['water_temp']))

print('Surface Distance: {}'.format(data['surface_distance']))

print('Battery Voltage: {}'.format(data['battery_voltage']))

print('Program Version: {}'.format(data['program_version']))

print('Top Temperature String:')
print(top_temperatures)

print('Bottom Temperature String:')
bottom_temperatures = unpackBinary(bottom_temp_bytes, 80)
print(list(reversed(bottom_temperatures)))


