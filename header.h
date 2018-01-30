//Versions
/*

V0.4
Added the USB command 11 to clear all SRAMS's content. Useful before the generator mode.

V0.5
Trying to solve the timing problem when probes are floating

V0.6
Added pro PID and VID to the USB library

V0.7
Fixed the 0 bits of the SRAMs status register (were left unchanged before...maybe that was the cause for un normal behaviour)


V0.8
Trying to solve sync problems between channels //SOLVED BY H/W
Added slow blink in idle mode

v0.9 trying to solve the GNDs problem..


V1.0
correcting the Gen mode problem...

V1.3
corrected a bug that appeared only in V13 PCBs

*/


//Main header file



#define FWV_MAJ	1
#define FWV_MIN	3

//SRAM interface pins
#define			PORT_MOSI	PORTC
#define			DDR_MOSI	DDRC
#define			PIN_MOSI	PINC

#define			MOSI0			PC0
#define			MOSI1			PC1
#define			MOSI2			PC2
#define			MOSI3			PC3
#define			MOSI_MASK	((1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3))

#define			PORT_MISO	PORTD
#define			DDR_MISO	DDRD
#define			PIN_MISO	PIND

#define			MISO0			PD3
#define			MISO1			PD4
#define			MISO2			PD5
#define			MISO3			PD6
#define			MISO_MASK	((1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD6))

#define			PORT_CS		PORTD
#define			DDR_CS		DDRD
#define			PIN_CS		PIND
#define			CS				(1<<PD7)

#define			PORT_CLK	PORTB
#define			DDR_CLK		DDRB
#define			PIN_CLK		PINB
#define			CLK				(1<<PB3)

#define			PORT_CLK_ENA	PORTB
#define			DDR_CLK_ENA		DDRB
#define			PIN_CLK_ENA		PINB
#define			CLK_ENA				(1<<PB7)

#define 		CH_ALL		99

#define 		SRAM_CS_ENA	DDR_CS |= CS;			PORT_CS &= ~(CS);	//set pin as output		//set pin low	
#define 		SRAM_CS_DIS	DDR_CS &= ~(CS);	PORT_CS &= ~(CS);		//set pin as inputs //disActivate pull-up on pin

 
#define			SRAM_HS_CLK_ENA PORT_CLK_ENA &= ~CLK_ENA; DDR_CLK |= (CLK); PORT_CLK |= (CLK); //SET low speed CLK pin as INPUT //activate high speed clock in case it is not active	 
#define			SRAM_HS_CLK_DIS PORT_CLK_ENA |= CLK_ENA;  DDR_CLK |= (CLK); PORT_CLK &= ~(CLK); //SET low speed CLK pin as OUTPUT //disactivate high speed clock in case it is active


#define 		CH12_IN_DIS		DDRB |= (1<<PB1); PORTB |= (1<<PB1);	
#define 		CH12_IN_ENA		DDRB |= (1<<PB1); PORTB &= ~(1<<PB1);
#define 		CH34_IN_DIS		DDRB |= (1<<PB0); PORTB |= (1<<PB0);	
#define 		CH34_IN_ENA		DDRB |= (1<<PB0); PORTB &= ~(1<<PB0);

#define 		CH12_OUT_DIS		DDRB |= (1<<PB2); PORTB |= (1<<PB2);	
#define 		CH12_OUT_ENA		DDRB |= (1<<PB2); PORTB &= ~(1<<PB2);
#define 		CH34_OUT_DIS		DDRB |= (1<<PB4); PORTB |= (1<<PB4);	
#define 		CH34_OUT_ENA		DDRB |= (1<<PB4); PORTB &= ~(1<<PB4);

#define 		SET_CH12_OUT	CH12_IN_DIS CH12_OUT_ENA
#define 		SET_CH12_IN		CH12_IN_ENA CH12_OUT_DIS

#define 		SET_CH34_OUT	CH34_IN_DIS CH34_OUT_ENA
#define 		SET_CH34_IN		CH34_IN_ENA CH34_OUT_DIS

#define 		ISOLATE_PROBES	CH12_IN_DIS CH34_IN_DIS CH12_OUT_DIS CH34_OUT_DIS

#define 		ISOLATE_ATMEGA	DDR_MOSI &= ~(MOSI_MASK);

#define LED_ON 			DDRD |= (1<<PD0); PORTD |= (1<<PD0);
#define LED_OFF 		DDRD |= (1<<PD0); PORTD &= ~(1<<PD0);
		

#define START_TIMER1	TCCR1B = 1; //set clock input as clk_io/1, no prescalling, 20MHz clock.
#define STOP_TIMER1		TCCR1B = 0; 

#define STOP_TIMER2					TCCR2A = 0; TCCR2B = 0; DDRB |= CLK; PORT_CLK &= ~(CLK);
#define START_TIMER2_DIV1		TCCR2A = (1<<COM2A0)|(1<<WGM21); TCCR2B = 1; DDRB |= CLK;
#define START_TIMER2_DIV8		TCCR2A = (1<<COM2A0)|(1<<WGM21); TCCR2B = 2; DDRB |= CLK;
#define START_TIMER2_DIV32	TCCR2A = (1<<COM2A0)|(1<<WGM21); TCCR2B = 3; DDRB |= CLK;
#define START_TIMER2_DIV64	TCCR2A = (1<<COM2A0)|(1<<WGM21); TCCR2B = 4; DDRB |= CLK;


#define START_USB_TIMER	TCCR0B = (1<<CS00)|(1<<CS02);
#define STOP_USB_TIMER	TCCR0B = 0;
// SRAM interface instructions
#define			INST_WRITE	2
#define			INST_READ		3

// General defines
#define			ENA		0
#define			DIS		1

//Trigger types
#define TRIG_FALLING	0
#define TRIG_RISING		1
#define TRIG_CHANGE		2
#define TRIG_NULL		3

//USB commands
#define USB_SEQUENCE_CONFIG 0x01
#define USB_SEQUENCE_ABORT	0x02
#define USB_SEQUENCE_LIVE_MODE	0x06
#define USB_UPDATE				0x07
#define USB_UPDATE_START	0x08
#define USB_UPDATE_BOOT		0x09
#define USB_GET_SN				10
#define USB_END_SEQUENCE	11
#define USB_FEEDBACK 0x05
#define USB_CLR_SRAMS			12
#define USB_ALIVE					13

#define SR_20M	0
#define SR_10M	1
#define SR_5M		2
#define SR_2M5	3
#define SR_1M		4
#define SR_500K	5
#define SR_250K	6
#define SR_100K	7
#define SR_50K	8
#define SR_10K	9
#define SR_1K25	10	



#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/eeprom.h>
#include <string.h>
#include <avr/boot.h> 
#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"



//function declarations
void setup_timers();
void setup_trigger(uchar mask);

//void sram_cs(uchar val);
void sram_out_all(uchar val);
void sram_write_byte(unsigned int add, uchar val, uchar channel);
void sram_write_byte_old(unsigned int add, uchar val);
uchar sram_read_byte(unsigned int add, uchar channel);
uchar sram_read_status();
void sram_clk(uchar val);
void sram_out(uchar val,uchar channel);
void sram_write_status(uchar val);
void sram_shit_out_byte(uchar val, uchar channel);
void sram_shit_out_word(unsigned int val, uchar channel);
void sram_shit_out_mixed_byte(uchar val1, uchar val2);
void sram_single_setup(uchar inst, uchar ch, unsigned int add);
//void sram_out_mixed(uchar val,uchar channel);
void sram_mixed_setup(uchar ch0_inst, uchar ch123_inst, unsigned int add);
uchar sram_shift_byte_in(uchar channel);
void sram_cs_reset();
void sram_cs_end();
void sram_internal_clk(uchar mode, uchar clk_sel,uchar val);
void sram_sequence_setup(uchar stop_clk_sel,uchar freq_clk_sel,unsigned int stop_val, uchar freq_val);
void sram_sequence_start();
void sram_read_block(unsigned int add, uchar channel,uchar len, uchar* data_store);
void sram_write_block(uchar channel,uchar len, uchar* data_store);
void bootloader();
//void sram_sequence_control(uchar clk_sel,unsigned int val);



/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */




/* Since we define only one feature report, we don't use report-IDs (which
 * would be the first byte of the report). The entire report consists of 128
 * opaque data bytes.
 */

/* The following variables store the status of the current data transfer */
uchar    currentAddress;
uchar    bytesRemaining;

volatile uint32_t post_trigger_samples,trigger_instant,pre_trigger_samples,idle_blink_counter,alive_counter;
volatile unsigned char test,sram_sequence_busy,TCCR1B_temp,usb_speed_boost;
//volatile unsigned char stream[32]={1,2,3,4,5,6,7,8,11,12,13,14,15,16,17,18,1,2,3,4,5,6,7,8,11,12,13,14,15,16,17,18};
volatile unsigned char stream[228],state_machine,test_counter;
volatile uchar new_usb_data,trigger_type,trigger_channel,trigged,abort_sequence,sampling_done,usb_data_read,sending_done,min_valid_signal_period;
volatile unsigned int timer1_post_scaller,sample_length,trigger_address,address,address_t,transfered_bytes,temp_add;
volatile signed char trigger_address_offset;
volatile uchar my_sn[4],update_channel,gen_mode,usb_alive,pc_connected;
//volatile uchar pinc_bkup asm("r3");
//volatile unsigned char data_in[31];
