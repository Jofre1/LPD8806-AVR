
/*

This program is intended to control the LPD8806 RGB LED Driver
It uses a software SPI ( Bit Banging) so the data and clock pins 
can be configured as desired in this case uses PB3 and PB4.

The LEDs or pixels are controlled sending 3 bytes with color data.
each byte has the MSB set to 1 so the remaining 7 bits control the 
brighness mangnitude being zero the OFF state and 127 the maximum
brightness

PB3 ck
PB4 Data OUT

The helper functions below and most part of the code was ported for the AVR from the 
Arduino library posted by LadyAda on  

https://github.com/adafruit/LPD8806/blob/7e9d4b6a2d4789ab4ad584fddd5ca2b44a00cbea/LPD8806.cpp

as well as other Arduino forums by different posters


*/


#define F_CPU 8000000UL

#include <avr/io.h>                       


// Define Pins for ATTiny45 ************

#define SPIDI	X	// PB4 MISO: data in (data in to Master) // NOT USED on ATTINY45
#define SPIDO	4	// PB4 MOSI: data out (data out from Master)
#define SPICLK	3	// PB3: clock
#define SPICS	2	// PB2:  

#define NOP asm("nop");
#define PULSE_SCK  {PORTB |= (1<< SPICLK); PORTB &= ~(1<<SPICLK);} // Note, clock is active low for SPI mode 3


// This variable must be filled with the actual number of LEDs or pixels to be controlled 
int  numLEDs = 32;

uint32_t pixels[36];


//General short delays
void delay_ms(uint16_t x)
{
  uint8_t y, z;
  for ( ; x > 0 ; x--){
    for ( y = 0 ; y < 94 ; y++){
      for ( z = 0 ; z < 16 ; z++){
        asm volatile ("nop");
      }
    }
  }
}


/* Software SPI initialization */

void sSPIinit(void) {
     
//	DDRB &= ~(1 << SPIDI);	// set port Bx SPI data input to input (Not used)
	DDRB |= (1 << SPICLK);	// set port B3 SPI clock to output
	DDRB |= (1 << SPIDO);	// set port B4 SPI data out to output 
//	DDRB |= (1 << SPICS);	// set port Bx SPI chip select to output (not used)

	PORTB |= (1<<4); // Data output and clockinitialized high.
	
	/*      
	
	*/
	
}


/* soft SPI function */

void sSPI(unsigned char Send) // Tested OK! 20010917 TF
{
    register unsigned char BitCount = 8;

    do
    {
        // Send bit to LPD string, MSB first
        if (Send & 0x80) 
        {
            PORTB |= (1 << SPIDO);
			
        }
        else
        {
            PORTB &= ~(1 << SPIDO);
        }
        
        // Toggle DFCK pin to send current bit
        
		PULSE_SCK;      
        
        // Get next bit to send
        Send <<= 1;      
    }while (--BitCount);


PORTB |= (1<< 4); // Added this to return MOSI to high after completing the byte transfer

} // sSPI


/*Show function . It will send out the pixel array */




void show(void) {
	uint16_t i;
	
	// get the strip ready to receive data ( send one zero  byte every 32 pixels or fraction)
	// ie 1 zero for up to 32, two zeros for more than 32 and up to 64, etc, etc.
	
	sSPI(0);
	

	// Send 3 bytes (24 bits)  per pixel
	// The LPD8806 order is g,r,b
	for (i=0; i<numLEDs; i++ ) {
    	sSPI(pixels[i]>>16 & 0xff);
    	sSPI(pixels[i]>>8 & 0xff);
    	sSPI(pixels[i] & 0xff);
	}
	
	
	delay_ms(2);
}


/* store an rgb component in the pixels array */



void setPixelColorgb(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t data;
    if (n > numLEDs) return;

    data = (g | 0x80); // OR'ed with 0x80 in order th set the most significant bit to 1
    data <<= 8;
    data |= (r | 0x80);
    data <<= 8;
    data |= (b | 0x80);

    pixels[n] = data;
}


/* Clear function to turn all  the Pixels OFF */

void clear() {
	for (uint16_t i=0; i < numLEDs; i++) {
		setPixelColorgb(i, 0, 0, 0);
	}
	show();
}



// Chase a dot down the strip
// good for testing purposes
void colorChase(uint8_t r,uint8_t g,uint8_t b, uint8_t wait) {
	int i;
	
	for (i=0; i < numLEDs; i++) {
		setPixelColorgb(i, 0,0,0);	// turn all pixels off
	} 
	
	for (i=0; i < numLEDs; i++) {
		setPixelColorgb(i, r,g,b);
		if (i == 0) { 
			setPixelColorgb(numLEDs -1, 0,0,0);
		} else {
			setPixelColorgb(i-1, 0,0,0);
		}
		show();
		delay_ms(wait);
	}
}

/* store a 3-byte color component in our array */

void setPixelColor(uint16_t n, uint32_t c) {
    uint32_t data;
    if (n > numLEDs) return;

    data = ((c>>16) | 0x80);
    data <<= 8;
    data |= ((c>>8) | 0x80);
    data <<= 8;
    data |= ((c) | 0x80);

    pixels[n] = data;
}


/* create a 3-byte color string from individual r,g,b values */

uint32_t Color(char r, char g, char b) {
	
	//Take the lowest 7 bits of each value and append them end to end
	// We have the top bit set high (its a 'parity-like' bit in the protocol
	// and must be set!)
	
	// (the LPD8806 wants the order to be green, red, blue)

	uint32_t x;
	x = g | 0x80;
	x <<= 8;
	x |= r | 0x80;
	x <<= 8;
	x |= b | 0x80;

	return(x);
}



/* Helper functions */

//Input a value 0 to 384 to get a color value.
//The colours are a transition r - g -b - back to r

uint32_t Wheel(uint16_t WheelPos)
{
	char r, g, b ;
	
	switch(WheelPos / 128)
	{
		case 0:
			r = 127 - WheelPos % 128;		//Red down
			g = WheelPos % 128;			 // Green up
			b = 0;									//blue off
			break; 
		case 1:
			g = 127 - WheelPos % 128;	 //green down
			b = WheelPos % 128;			 //blue up
			r = 0;									//red off
			break; 
		case 2:
			b = 127 - WheelPos % 128;	 //blue down 
			r = WheelPos % 128;			 //red up
			g = 0;									//green off
			break; 
	}
	return(Color(r,g,b));
}


void rainbowCycle(uint8_t wait) {
	
	uint16_t i, j;
	
	for (j=0; j < 384 * 5; j++) {			// 5 cycles of all 384 colors in the wheel
		for (i=0; i < numLEDs; i++) {
			// tricky math! we use each pixel as a fraction of the full 384-color wheel
			// (thats the i / strip.numPixels() part)
			// Then add in j which makes the colors go around per pixel
			// the % 384 is to make the wheel cycle around
			setPixelColor(i, Wheel( ((i * 384 / numLEDs + j) % 384) ));
		}	 
		show();		// write all the pixels out
		delay_ms(wait);
	}
}

void rainbow(uint8_t wait) {
	int i, j;
	 
	for (j=0; j < 384; j++) {			// 3 cycles of all 384 colors in the wheel
		for (i=0; i < numLEDs; i++) {
			setPixelColor(i, Wheel( (i + j) % 384));
		}	 
		show();		// write all the pixels out
		delay_ms(wait);
	}
}


/*

	THIS MAIN FUNCTION IS JUST A DEMO THE USER CAN DEVELOP ANY APPLICATION
	MAINLY USING show(), setPixelColorgb(uint16_t n, uint8_t r, uint8_t g, uint8_t b) and clear()
	
	


*/

int main(void) {


char wtime = 200;
char a;

sSPIinit();

     while(1) {
	 
	 colorChase(0,0,127,wtime);  // BLUE chase
	 
	 rainbowCycle(wtime);
	 
	  colorChase(0,127,0,wtime); //   GREEN chase
	  
	    rainbow(wtime);     
	  
	   colorChase(127,0,0,wtime); // RED chase
	   
	   
	   
	  
			   
			   
			   }
			   
					  
				  
				  
				  
    }




