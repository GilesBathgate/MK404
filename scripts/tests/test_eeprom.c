/*
	atmega88_uart_echo.c

	This test case enables uart RX interupts, does a "printf" and then receive characters
	via the interupt handler until it reaches a \r.

	This tests the uart reception fifo system. It relies on the uart "irq" input and output
	to be wired together (see simavr.c)
 */

#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <util/delay.h>

/*
 * This demonstrate how to use the avr_mcu_section.h file
 * The macro adds a section to the ELF file with useful
 * information for the simulator
 */
#include "avr_mcu_section.h"
AVR_MCU(16000000, "atmega2560");
// tell simavr to listen to commands written in this (unused) register
// AVR_MCU_SIMAVR_COMMAND(&GPIOR0);
// AVR_MCU_SIMAVR_CONSOLE(&GPIOR1);

/*
 * This small section tells simavr to generate a VCD trace dump with changes to these
 * registers.
 * Opening it with gtkwave will show you the data being pumped out into the data register
 * UDR0, and the UDRE0 bit being set, then cleared
 */
// const struct avr_mmcu_vcd_trace_t _mytrace[]  _MMCU_ = {
// 	{ AVR_MCU_VCD_SYMBOL("UDR3"), .what = (void*)&UDR3, },
// 	{ AVR_MCU_VCD_SYMBOL("UDRE3"), .mask = (1 << UDRE3), .what = (void*)&UCSR3A, },
// 	{ AVR_MCU_VCD_SYMBOL("GPIOR1"), .what = (void*)&GPIOR1, },
// };
// #ifdef USART3_RX_vect_num	// stupid ubuntu has antique avr-libc
// AVR_MCU_VCD_IRQ(USART3_RX);	// single bit trace
// #endif
// AVR_MCU_VCD_ALL_IRQ();		// also show ALL irqs running

#define PIN(x,y) PORT##x>>y & 1U

volatile uint8_t done = 0;

static int uart_putchar(char c, FILE *stream)
{
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,
                                         _FDEV_SETUP_WRITE);

ISR(USART0_RX_vect)
{
	//uint8_t b = UDR0;
	//done++;
//	sleep_cpu();
}

int main()
{
	stdout = &mystdout;
	sei();

 	DDRH = 0x00;
	//PORTH = 0xFF;

	DDRJ = 0x00;

	printf("READY\n");

	loop_until_bit_is_clear(PINH,6);

	for(int i=0; i<4096; i++)
	{
		if (eeprom_read_byte((uint8_t*)i) !=0xFF)
			printf("EE NOT EMPTY\n");
	}

	printf("EEPROM EMPTY\n");

	for(int i=0; i<4096; i++)
	{
		eeprom_update_byte((uint8_t*)i,i%256);
	}

	printf("EEPROM WRITTEN\n");

	loop_until_bit_is_clear(PINH,6);

	for(int i=0; i<4096; i++)
	{
		if (eeprom_read_byte((uint8_t*)i) !=0xFF)
			printf("EE NOT EMPTY\n");
	}

	printf("EEPROM EMPTY\n");

	loop_until_bit_is_clear(PINH,6);

	for(int i=0; i<4096; i++)
	{
		if (eeprom_read_byte((uint8_t*)i) !=i%256)
			printf("EE VERIFY FAILED\n");
	}

	printf("EE VERIFY FINISH\n");

	loop_until_bit_is_clear(PINH,6);

	printf("BYTE 123: %02x\n",eeprom_read_byte((uint8_t*)123));


	cli();

	printf("FINISHED\n");

	// this quits the simulator, since interupts are off
	// this is a "feature" that allows running tests cases and exit
	sleep_cpu();
}
