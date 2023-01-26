# This script opens an SBD binary file and converts the binary to temperatures to display
# It's used to validate the packing and unpacking of the values from the SIMB3 One-Wire Temperature String
# Values output from this code should match those in the serial console on SIMB3 transmission
# Code was validated for positive and negative binary numbers from the DS28EA00 datasheet

# Note: the packing sequence which this code undoes is defined in BitPacking.xlsx

# Written 8 December 2022 by Cameron Planck

import collections

from binaryreader import *


def applyBitmap(binary_list):

	""" Applies the bitmap from the DS28EA00 datasheet to derive the temperature from the bits.
	Page 8 of https://datasheets.maximintegrated.com/en/ds/DS28EA00.pdf """

	convertedTemp = ((binary_list[5]*2**6) + (binary_list[6]*2**5) + (binary_list[7]*2**4) + (binary_list[8]*2**3) + (binary_list[9]*2**2) + (binary_list[10]*2**1) + (binary_list[11]*2**0) + (binary_list[12]*2**-1) + (binary_list[13]*2**-2) + (binary_list[14]*2**-3) + (binary_list[15]*2**-4))
	
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

		if TempBin & 0x800 != 0: # checks if the sign bit is set to 1
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

		if TempBin & 0x800 != 0: # checks if the sign bit is set to 1
			temperature = convertNegativeTemps(TempBin)
		else:
			temperature = convertPositiveTemps(TempBin)

		temperatures.append(temperature)

		i = i+3
	return temperatures


# path to the satellite file to download
sbd_path = 'test-data/300434064562590_000347.sbd'

binaryReader = BinaryReader(sbd_path)

data = collections.OrderedDict()   
data['air_temp'] = binaryReader.read('uint16')*0.0625

# read bytes in as unsigned integers
top_temp_bytes = []
for byte in range(0,120):
	top_temp_bytes.append(binaryReader.read('uint8'))

bottom_temp_bytes = []
for byte in range(0,120):
	bottom_temp_bytes.append(binaryReader.read('uint8'))

top_temperatures = unpackBinary(top_temp_bytes, 80)
print('TOP TEMPERATURE STRING')
print(top_temperatures)

bottom_temperatures = unpackBinary(bottom_temp_bytes, 80)
print('BOTTOM TEMPERATURE STRING')
print(list(reversed(bottom_temperatures)))

print('AIR TEMPERATURE')
print(data['air_temp'])
