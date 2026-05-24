# CHEEPROM_2026
Programmer for AT28C64 and AT28C256 EEPROM chips

This is a simple programmer based on an Arduino Nano (ATMega328) module or compatible.  
In addition to the Nano it has two 74hc595 shift registers, a couple of LEDs and resistors, and a 
Zero Insertion Force (ZIF) socket for the EEPROM.

There is a special PC program written in C to run on Linux for controlling the programmer.  I 
try to avoid Windows as much as possible, so if you want to go that route you're on your 
own.

Currently, the only one I have built is on a prototyping board.  I may or may not lay out a 
PCB for the current version.  I am planning to build a much more capable version in the near 
future that will also be able to handle Intel type 27xxx Eproms and perhaps some other chips 
so I may not bother with a PCB for this one.

In this repo are the schematic and parts list, the Arduino code, and the Linux C code.  The 
Arduino code was written and tested using Arduino IDE 1.8.19.  Other versions might cause 
problems.  
