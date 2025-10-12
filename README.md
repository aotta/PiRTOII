# PiRTO II
Intellivision flash multicart based on Pico clone

PiRTO II  is the new version of my PiRTO multicart DIY yourself based on cheap "purple" Raspberry Pi Pico clone.

(use "alternate_boot" version if your PiRTO doesn't start without resetting it and/or the console).

added v. 1.01: bug fixed (added test if more than 64 files in a folder)

added v. 1.02: bug fixed (fix for Coleco Rom from previous version)

added v. 1.03: bug fixed (issue in Chess after move #21)

added v. 1.04: bug fixed Missile Domination (added support for both RAM8 and RAM16) and added initialization fix used by gtortone for his "Minty" for better compatibility

**WARNING!** "purple" Pico has not the same pinout of original Raspberry "green" ones, you MUST use the clone or you may damage your hardware.

**NOTE** FAT FS used for flash file management is quite simple, you can't use FILENAME LONGER THAN 32 BYTES INCLUDED SUFFIX!!!!! 
PLEASE RENAME YOUR BIN FILES OR THE GAMES WON'T START AT ALL!!! 

![ScreenShot](https://raw.githubusercontent.com/aotta/PiRTOII/main/Pictures/pirtoII1.jpg)
![ScreenShot](https://raw.githubusercontent.com/aotta/PiRTOII/main/Pictures/pirtoII2.jpg)

Kicad project and gerbers files for the pcb are in the PCB folder, you need only a diode and a push buttons for resetting the cart if needed or want restart. 
Add you pico clone, and flash the firmware ".uf2" in the Pico by connecting it while pressing button on Pico and drop it in the opened windows on PC.
After flashed with firmware, and every time you have to change your ROMS repository, you can simply connect the Pico to PC and drag&drop "BIN" and "CFG“ files  into.

More info on AtariAge forum: https://forums.atariage.com/


Even if the diode should protect your console, **DO NOT CONNECT PICO WHILE INSERTED IN A POWERED ON CONSOLE!**
You can use any schottky diode, i usually put 1n4148

## Credits
Thank to Robin Robin Edwards and his A8PicoCart (https://github.com/robinhedwards/A8PicoCart), i found very smart his way to manage the Flash RAM and the USB updates, so i admit i took large parte of his code for it!.



![ScreenShot](https://raw.githubusercontent.com/aotta/PiRTOII/main/Pictures/pirtoII3.jpg)
![ScreenShot](https://raw.githubusercontent.com/aotta/PiRTOII/main/Pictures/pirtoII4.jpg)
