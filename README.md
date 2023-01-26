# SIMB3 One-Wire Temperature String Repository
Written 26 January, 2023 by Cameron Planck

---

This is the main repository for the SIMB3 One-Wire Digital Temperature String. The temperature string consists of custom made printed circuit boards that contain Maxim Integrated DS28EA00 temperature sensors positioned at 2cm spacing. This repo contains production-grade sketches for using the temperature string with the SIMB3 V4 brain as well as sketches for testing and sketches used in development. The corresponding mainboard sketch (in the SIMB3-embedded-code repo) is SIMB3_BRAIN_V4.ino.

The imports files and folders are listed below:

```
├── Production
│   ├── one-wire-controller-production.ino
│   ├── SIMB3_Onewire_Controller
│   │   ├── SIMB3_Onewire_Controller.cpp
│   │   ├── SIMB3_Onewire_Controller.h
|
├── Testing
│   ├── pcb-benchtop-test.ino
│   ├── test-brain-v4-with-tempstrings.ino
|
├── Development
│   ├── Arduino
│   │   ├── many...
│   ├── Excel
│   │   ├── BitPacking.xlsx
│   ├── Python
│   │   ├── UnpackTempStringBinaryFullMessage.py

```

The /Production directory contains only grade-A production code. `one-wire-controller-production.ino` is the production sketch for the one-wire controller. The SIMB3_Onewire_Controller directory is the library for the one-wire temperature string. Note: these files are just here for version control. When developing, you must modify the files of the same name under in the Arduino library root directory. After development is finished they should be copied back into this repo. 

The /Testing directory has two sketchs.
- `pcb-benchtop-test.ino`: for testing single or multiple soldered SIMB3 one-wire PCBs. This code is intended for use during temp-string manufacturing.
- `test-brain-v4-with-tempstrings.ino`: for testing communication between the SIMB3 V4 brain and the one-wire controller. Simply loops through communication without reading other sensors or transmitting. 

The /Development/Arduino directory contains many sketches that were used in the development of the SIMB3 one-wire temperature string. Caution should be used when taking any code from these sketches, as they may contain old/outdated code that looks correct but is in fact wrong. If debugging or testing features not covered by sketches in the /Testing directory, it's recommended to create new sketches using only code from production grade sketches or the SIMB3 one-wire temperature string library. 

The /Development/Excel directory contains one file, BitPacking.xlsx, which is the map for the bit shifting algorithm used to pack the 12-bit temperature string values to 8-bit byte boundaries for transmission over Iridium. Consult this to understand the embedded packing algorithm or the server-side unpacking algorithm. 

The Development/Python directory contains test files for unpacking and decoding the binary files from Iridium.
