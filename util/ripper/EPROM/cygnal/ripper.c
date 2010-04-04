/*
EPROM ripper
Copyring 2009 John McMaster <JohnDMcMaster@gmail.com>
*/

#include <c8051_SDCC.h>// include files. This file is available online
#include <stdio.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------
void Port_Init(void);// Initialize ports for input and output
void Timer_Init();// Initialize Timer 0

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------


/*
Port 0: input
Port 1: Lowest address byte
Port 2: Middle address byte
Port 3: High address bits
*/

#define PORT_ADDR_LOW			P1
#define PORT_ADDR_MEDIUM		P2
#define PORT_ADDR_HIGH			P3
#define PORT_READ				P4

/*
Make volatile any numbers accessed in ISR
*/
//How many overflows have occured
volatile unsigned int g_counts = 0;
//What to overflow counter at
volatile int g_timer_start = 0;

#define TIMER_START		0xF0

//***************
void Port_Init(void)
{
	// Port 3: output port
	P1MDOUT |= 0xFF;
	P2MDOUT |= 0xFF;
	P3MDOUT |= 0xFF;

	// Port 2: input port
	//P0MDOUT = 0x00;
	P4MDOUT &= 0x00;
	PORT_READ = 0xFF;
}

//***************
void Timer_Init(void)
{
	//pg 226
	CKCON |= 0x04; // Timer0 uses SYSCLK as source
	CKCON |= 0x40; // Timer0 does not divide sysclock by 12
	//pg 232
	TMOD &= 0xF0; // Clear the 4 least significant bits
	TMOD = TMOD & 0xFC + 0x01; // Make T1 intact and T0 use mode 1
	TR0 = 0;// Stop Timer0
	TL0 = 0; // Clear low byte of register T0
	TH0 = 0;// Clear high byte of register T0
}

//***************
void Timer0_ISR(void) interrupt 1
{
	TH0 = g_timer_start >> 8;
	TL0 = g_timer_start & 0xFF;
	++g_counts; // increment overflow counter
}


void mem_pause(void)
{
	/*
	//Buzz for 1/4 second
	g_counts = 0;
	TR0 = 1; // Timer 0 enabled
	//Wait for our overflow: 22.1184 cycles / sec / (8192 * 8) cycles = 338
	while( g_counts < (338 / 300) )
	{
	}
	TR0 = 0;// Timer 0 disabled
	*/
}

void start_pause(void)
{
	/*
	//Buzz for 1/4 second
	g_counts = 0;
	TR0 = 1; // Timer 0 enabled
	//Wait for our overflow: 22.1184 cycles / sec / (8192 * 8) cycles = 338
	while( g_counts < (338 / 4) )
	{
	}
	TR0 = 0;// Timer 0 disabled
	*/
}

void print_pause(void)
{
	/*
	//Buzz for 1/4 second
	g_counts = 0;
	TR0 = 1; // Timer 0 enabled
	//Wait for our overflow: 22.1184 cycles / sec / (8192 * 8) cycles = 338
	while( g_counts < (338 / 300) )
	{
	}
	TR0 = 0;// Timer 0 disabled
	*/
}

void print_padded_hex_byte(unsigned char byte)
{
	int byte_int = byte;
	if( byte < 0x10 )
	{
		printf("0%X", byte_int);
	}
	else
	{
		printf("%X", byte_int);
	}
	print_pause();
}

//***************
void main(void)
{
	Sys_Init();// System Initialization
	putchar(' ');
	Port_Init();// Initialize ports 2 and 3
	Timer_Init();// Initialize Timer 0
	//pg 119
	IE |= 0x02;// enable Timer0 Interrupt request
	//This must be a bit, its part of the IE SFR
	EA = 1;// enable global interrupts
	printf("Start\r\n");

	for( ;; )
	{
		unsigned char cur_col = 0;
		unsigned char max_addr_low = 0xFF, max_addr_middle = 0xFF, max_addr_high = 0x03;
		//unsigned char max_addr_low = 0x7F, max_addr_middle = 0x00, max_addr_high = 0x00;

		unsigned char addr_low, addr_middle, addr_high;

		printf("\r\n\r\nPreparing to read\r\n");
		start_pause();
		printf("Reading\r\n\r\n");

		for( addr_high = 0; 1; ++addr_high )
		{
			P3 = addr_high;
			for( addr_middle = 0; 1; ++addr_middle )
			{
				P2 = addr_middle;
				for( addr_low = 0; 1; ++addr_low )
				{
					unsigned char read = 0;
					P1 = addr_low;
					mem_pause();
					read = PORT_READ;
					if( cur_col == 0 )
					{
						printf("00");
						print_pause();
						print_padded_hex_byte(addr_high);
						print_padded_hex_byte(addr_middle);
						print_padded_hex_byte(addr_low);
						printf("  ");
						print_pause();
					}

					print_padded_hex_byte(read);
					if( cur_col == 0x0F )
					{
						printf("\r\n");
						print_pause();
						cur_col = 0;
					}
					else
					{
						printf(" ");
						print_pause();
						++cur_col;
					}

					if( addr_low == max_addr_low )
					{
						break;
					}
				}
				if( addr_middle == max_addr_middle )
				{
					break;
				}
			}
			if( addr_high == max_addr_high )
			{
				break;
			}
		}

		
		printf("\r\n\r\nDone");
		for( ;; )
		{
		}
	}
 }

