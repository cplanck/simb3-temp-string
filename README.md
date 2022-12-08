This is the main repository for the SIMB3 One-Wire Digital Temperature String

Code in this repository includes development, testing, and production deployment code

File structure and explanations are below: 

-Development
|
---read-one-DS18B20.ino:
|   A simple sketch used to read binary data from a standard DS18B20 temperature sensor connected to a Feather M0 board. It was used in development to read SIMB3 air temperature sensor.
|
---read-one-DS28EA00.ino:
|   Reads temperature off a single DS28EA00 connected to a Feather M0. Used in development to debug DS28EA00 communication without exclusive reliance on the Dallas Temperature library.
|
---read-multiple-DS28EA00.ino:
|    Same basic routine as read-oneDS28EA00, but it performs a ROM discovery routine to find all chips on the bus. It then loops through each chip and records the temperature. 
|
---basic-transmit.ino:
|   Bare-bones routine for testing Iridium transmission using SIMB3 hardware. Doesn't require any peripheral devices, just writes and transmits a hardcoded test value.
|
---transmit-DS28EA00s
|   Development code for sending the packaged DS28EA00 binary across Iridium. Used to develope and validate and packing/unpacking routine for the 12-bit temperature values from the chain.
|   Includes a bare-bones transmit Iridium transmit routine.
---I2C Basic++
|       |
|       ---basic-i2c-master_pp.ino
|       |   Development code for testing communication between the SIMB3 mainboard and the "slave" one-wire control board over I2C. The mainboard code iteratively requests data from the one-wire slave
|       |   (Feather M0) which is connected to the various one-wire devices, including the one-wire temperature string and the air temperature sensor. The master issues an I2C command and the slave    |   responds with the data. The code also "packages" the data to 8-byte chunks for transmission over Iridium.
|       |
|       ---basic-i2c-slave_pp.ino
|       |   Development for the "slave" Feather M0 one-wire controller. Listens to requests from master and responds with data which it requests from the several one-wire devices connected to it. 
|
---One-Wire Controller Roundtrip
|       |
|       ---ow-controller-master.ino
|       |   Development code for sending one-wire temperature string data over Iridium. Master code issues I2C command to slave for data from ow-temp string and air temperature sensors, packs it up, 
        |   and transmits it.
|       |
|       ---ow-controller-slave.ino
|       |   Effectively the same code as basic-i2c-slave_pp.ino, but enhanced to prepare it to be depployment ready.
