LPD8806-AVR
===========

This is just a program with functions to control the LPD8806 with AVR controllers.

The file has the typical functions for demo on LED flexible strips. But the LPD8806 can be used on any application
to control RGB leds , for instance, it can simplify the design on embedded applications or industrial applications
where the user will interact with a control board or device where lamps are used to display status , operation or alerts.

To use this program , just create a project using the ATTiny45 with the fuses programmes for the internal oscillator at 8 MHZ. Copy the file SPI_Led3_1.c in the project root and build. The default value on numLEDs is 32 to be used with 1 meter strip. Change this number according to your project.
