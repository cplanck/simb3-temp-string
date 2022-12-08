# Python script to convert the raw 2-byte temperature values into real temperature in degrees C
# it checks the sign bit (bit )
# The bitmap used in convertedTemp is from the DS28EA00 datasheet (page 8) at: https://datasheets.maximintegrated.com/en/ds/DS28EA00.pdf

DS28EA00_binary = 0b1111111001101111 # -25.0625
DS28EA00_binary = 0b1111110110000000 # -40
DS28EA00_binary = 0b1111111111111000 # -0.5
DS28EA00_binary = 0b1111111101011110 # -10.125
DS28EA00_binary = 0b0000000010100010 # 10.125

DS28EA00_binary = 0b0000000110000100  

DS28EA00_binary = 0b0001000101111010

DS28EA00_binary = 0b1000111000010001
DS28EA00_binary = 0b0000011011000001

DS28EA00_binary = 0b0000000101101100
  

  

    

def convertNegativeTemps(binary):

    """ Takes a 16-bit raw negative binary value and converts it to temperature by undoing two's complement """


    binary = int(binary) - 1

    binary_str = str(bin(binary))

    binary_str_inv = ''
    for item in binary_str[2:]:
        
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

def applyBitmap(binary_list):

   """ Applies the bitmap from the DS28EA00 datasheet to derive the temperature from the bits.
	Page 8 of https://datasheets.maximintegrated.com/en/ds/DS28EA00.pdf """

    convertedTemp = ((binary_list[5]*2**6) + (binary_list[6]*2**5) + (binary_list[7]*2**4) + (binary_list[8]*2**3) + (binary_list[9]*2**2) + (binary_list[10]*2**1) + (binary_list[11]*2**0) + (binary_list[12]*2**-1) + (binary_list[13]*2**-2) + (binary_list[14]*2**-3) + (binary_list[15]*2**-4))
    return convertedTemp


if DS28EA00_binary & 0x800 != 0: # checks if the sign bit is set to 1
    temperature = convertNegativeTemps(DS28EA00_binary)

else:
    temperature = convertPositiveTemps(DS28EA00_binary)

print('TEMPERATURE: {} C'.format(temperature))