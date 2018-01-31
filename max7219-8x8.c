
/*

 max7219.c
 
 Raspberry Pi driving the Max7219

 to compile : gcc max7219.c -o max7219 -lwiringPi
 for odroid build : gcc -W -Wall -o max7219 max7219.c -lwiringPi -lpthread

*/
 
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "fontmap.h"


// define our pins :

#define DATA        0 // GPIO 17 (WiringPi pin num 0)  header pin 11
#define CLOCK       3 // GPIO 22 (WiringPi pin num 3)   header pin 15
#define LOAD        4 // GPIO 23 (WiringPi pin num 4)   header pin 16


// The Max7219 Registers :

#define DECODE_MODE   0x09                       
#define INTENSITY     0x0a                        
#define SCAN_LIMIT    0x0b                        
#define SHUTDOWN      0x0c                        
#define DISPLAY_TEST  0x0f                         

#define MODULENUMBER 8

char dbuf[MODULENUMBER*8];
int led_x,led_y;

// ************************************************************************
//
// ************************************************************************

static void Send16bits (uint16_t output)
{
	
  unsigned char i;
  uint16_t mask;

  digitalWrite(CLOCK, 0);  // set clock to 0

  for (i=16; i>0; i--) 
  {
    mask = 1 << (i - 1); // calculate bitmask
    // Send one bit on the data pin
    
    if (output & mask)   
      digitalWrite(DATA, 1);          
		else                              
      digitalWrite(DATA, 0);  
    digitalWrite(CLOCK, 1);  // set clock to 1
    usleep(1);
    digitalWrite(CLOCK, 0);  // set clock to 0  	 
  }
}

//**********************************************************************
// Take a reg numer and data and send to the max7219
//**********************************************************************

void MAX7219Send (unsigned char reg_number, unsigned char dataout)
{
  digitalWrite(LOAD, 0);
  Send16bits((reg_number << 8) + dataout);   // send 16 bits ( reg number + dataout )
  digitalWrite(LOAD, 1);
}

//************************************************************************
//
//************************************************************************

static void MAX7219SendX4 (unsigned char reg_number, unsigned char dataout)
{
  int i;
  
  digitalWrite(LOAD, 0);
  for(i=0;i<MODULENUMBER;i++) {
   Send16bits((reg_number << 8) + dataout);   // send 16 bits ( reg number + dataout )
  }
  digitalWrite(LOAD, 1);
}

//************************************************************************
//
//************************************************************************

void putpixel(int x, int y)
{
 int a,b;
 
 if(x>(MODULENUMBER*8-1)) return;
 if(x<0) return;
 if(y>8) return;
 if(y<0) return;
  
 a = x & 0x07;
 b = x >> 3;
 dbuf[b+(MODULENUMBER*(7-y))] |= (0x80>>a);	
}

//************************************************************************
//
//************************************************************************

void clrpixel(int x, int y)
{
 int a,b;
 
 if(x>MODULENUMBER*8-1) return;
 if(x<0) return;
 if(y>8) return;
 if(y<0) return;
  
 a = x & 0x07;
 b = x >> 3;
 dbuf[b+(MODULENUMBER*(7-y))] &= ~(0x80>>a);	
}

//************************************************************************
//
//************************************************************************

char led_putchar(char c)
{
  uint8_t mask,t;
  int i,k,s;

  mask = 0x80;
  s = c*CHARWIDTH;
  
  for(i=0;i<8;i++) {
  	for(k=0;k<CHARWIDTH;k++) {
  	  t=fontmap[s+k];
      if(t & mask)
        putpixel(led_x+k,led_y+i);
      else
       clrpixel(led_x+k,led_y+i);
   }
   mask = mask >> 1;   
  }
  led_x += CHARWIDTH;
//  led_x--;					// nur für ganz schmal
  return c;
}


void led_puts(char *c)
{
	while(*c) led_putchar(*c++);
}
//************************************************************************
//
//************************************************************************

void showit()
{
	int i,k;
	
   for(k=0;k<8;k++) {
  	digitalWrite(LOAD, 0);
   for(i=0;i<MODULENUMBER;i++) {
   	Send16bits(((k+1)<<8)|dbuf[k*MODULENUMBER+i]);
   };
   digitalWrite(LOAD, 1);
  };  
}

//************************************************************************
//
//************************************************************************

int main (int argc, char *argv[])
{
  int i,k;
	
  printf ("\n\nRaspberry Pi Max7219 Test using WiringPi\n\n");
  
  if( argc == 2 ) {
     printf("Displaying %s\n", argv[1]);
  }
  else if( argc > 2 ) {
     printf("Too many arguments supplied.\n");
  }
  
  if (wiringPiSetup() == -1) exit (1) ;

  //We need 3 output pins to control the Max7219: Data, Clock and Load

  pinMode(DATA, OUTPUT);  
  pinMode(CLOCK, OUTPUT);
  pinMode(LOAD, OUTPUT);
  digitalWrite(LOAD, 1);
  digitalWrite(CLOCK, 0);  // set clock to 0  

  MAX7219SendX4(SCAN_LIMIT, 7);     // set up to scan all eight digits
  MAX7219SendX4(DECODE_MODE, 0x00);   // Set BCD decode mode on for all digits to off
  MAX7219SendX4(DISPLAY_TEST, 0);  // Disable test mode
  MAX7219SendX4(INTENSITY, 8);     // set brightness 0 to 15
  MAX7219SendX4(SHUTDOWN, 1);      // come out of shutdown mode	/ turn on the digits
  
  led_x=0;
  led_y=0;
  
  for(i=0;i<MODULENUMBER*8;i++) {
  	dbuf[i]=0;
  }
/*    
  for(k=0;k<8;k++) {
  	digitalWrite(LOAD, 0);
   for(i=0;i<4;i++) {
   	Send16bits(((k+1)<<8)|dbuf[k*4+i]);
   };
   digitalWrite(LOAD, 1);
  };  
*/

/*  
  for(i=0;i<8;i++) {
  	for(k=0;k<MODULENUMBER*8;k++) {
  	  putpixel(k,i);
      showit();
      clrpixel(k,i);
   }
  };
  
  
  for(k=0;k<8;k++)
  {
    led_y = -7+k;
//    led_y = 7-k;
    led_x = 0;
    
    led_puts("O----O");
    showit();
  }

  for(i=0;i<32;i++) {
  	dbuf[i]=0;
  }
  showit();
*/
/*  
  for(k=0;k<64+(CHARWIDTH*strlen(argv[1]));k++)
  {
//    led_x = -31+k;
    led_x = (MODULENUMBER*8-1)-k;
    led_y = 0;
    
    led_puts(argv[1]);
    showit();
    usleep(50000);
  }
*/
  led_puts(argv[1]);
  showit();  
/*  
  for(i=0;i<32;i++) {
  	dbuf[i]=0;
  }
  showit();
*/  
  return 0;  
}
