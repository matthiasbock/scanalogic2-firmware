//Serial SRAM access

#include "header.h"


BOOTLOADER_SECTION void bootloader()
{
	volatile uint16_t page;
	volatile uint16_t page_add;
	volatile uint16_t word;
	volatile uint16_t xeeprom_add;
	volatile uint16_t i;
	volatile uint32_t temp_1,key,byte_counter,local_cycle,local_cycle_period;
	volatile uint16_t temp_2;

	key = 0x1FE2;
//	volatile uchar page_ok;

	cli();
	//Turn off the WDT
	MCUSR &= ~(1<<WDRF);
	WDTCSR |= (1<<WDCE) | (1<<WDE); //Start timed sequence to edit watchdog configuration
	WDTCSR = 0;
	
	usbDeviceDisconnect();
	
	
	for (page = 0; page < 112;page++)
	{
		boot_spm_busy_wait ();
		eeprom_busy_wait ();
		boot_page_erase_safe (page*SPM_PAGESIZE);
		boot_spm_busy_wait ();      // Wait until the memory is erased.
	}
	LED_ON

	// DECRYPTION
	local_cycle = 0;
  local_cycle_period = 3;
	temp_1 = 0;
	// END DECRIPTION

	for (page = 0; page < 112;page++)
	{

		word = 0;
		page_add = page * SPM_PAGESIZE;
		xeeprom_add = (page_add);

		
		boot_spm_busy_wait();
		eeprom_busy_wait ();
		boot_page_erase(page_add);
		boot_spm_busy_wait ();
		eeprom_busy_wait ();
		boot_rww_enable ();
		boot_spm_busy_wait();
		eeprom_busy_wait ();


		for (i=0; i<SPM_PAGESIZE; i+=2)
    {
        // Set up little-endian word.
        word = sram_read_byte(xeeprom_add++,0);
        word += (sram_read_byte(xeeprom_add++,0)) << 8;
				
				//DECRYPT THE WORD
					temp_2 = word;
					local_cycle += 1;
	        if (local_cycle > local_cycle_period)
					{
	            local_cycle = 0;
	            local_cycle_period += 1;
	            if (local_cycle_period > 99)
							{
	                local_cycle_period = 3;
	            }
	        }

					temp_1 = temp_1 + key ;//+ (local_cycle * 256) + (256 - local_cycle) + local_cycle_period ;

					while (temp_1 > 65535)
					{
                    temp_1 -= 65536;
          }

	        temp_2 = temp_2 ^ (uint16_t)temp_1;
					
					
					//****************************************************************************************************
					word = temp_2;

				//END OF DECRYPT THE WORD

        boot_page_fill_safe (page_add+ i, word);
				boot_spm_busy_wait();
				eeprom_busy_wait ();


				//DECRYPTION RELATED PART
				byte_counter++;
    }

 		boot_page_write (page_add);     // Store buffer in flash page.
    boot_spm_busy_wait();  
		eeprom_busy_wait ();
		boot_rww_enable ();
		boot_spm_busy_wait();
		eeprom_busy_wait ();
		while (boot_rww_busy());
		
		//if (pgm_read_byte((page_add)) != 0xBB){
		//	page--;
		//	LED_OFF
		//}
		
	}


	wdt_enable(WDTO_4S);
	
	while(1){ 
		LED_OFF
	}
}


BOOTLOADER_SECTION void sram_mixed_setup(uchar ch01_inst, uchar ch23_inst, unsigned int add)
{
	//	DECRIPTION :  sends adequate  instruction for channel0 and channels 1,2 
	// 	and 3. Then sets the starting address to "add" for all channels
	//	It can be used to setup the SRAMs for a mixed Playback/Record operation
	//	Can also be used for non mixed mode (all record or all playback)
	//
	//	This function is manly used for sequantial read/write of the SRAM
	//	and does not includes the SPI sequence termination on purpose.
	ISOLATE_PROBES
	sram_cs_reset();
	sram_shit_out_mixed_byte(ch01_inst, ch23_inst); //send instructions
	sram_shit_out_word(add, CH_ALL);	//send start address
}

BOOTLOADER_SECTION void sram_single_setup(uchar inst, uchar ch, unsigned int add)
{
	//	DECRIPTION :  sends adequate  instruction for channe "ch" 
	// 	Then sets the starting address to "add" for channels "ch"
	//	It can be used to setup the SRAMs for a mixed Playback/Record operation
	//	Can also be used for non mixed mode (all record or all playback)
	//
	//	This function is manly used for sequantial read/write of the SRAM
	//	and does not includes the SPI sequence termination on purpose.
	ISOLATE_PROBES
	sram_cs_reset();
	sram_shit_out_byte(inst, ch); //send instructions
	sram_shit_out_word(add, ch);	//send start address
}

BOOTLOADER_SECTION uchar sram_read_byte(unsigned int add, uchar channel)
{

	//	DECRIPTION : Reads a byte of data at the address "add" 
	// 	from the SRAM determined by "channel" which is a 
	//	value from 0 to 3. 

	uchar val;
	ISOLATE_PROBES
	sram_cs_reset();
	sram_shit_out_byte(INST_READ,channel);
	sram_shit_out_word(add, channel);
	val = sram_shift_byte_in(channel);

	sram_cs_end();
	return val;
}


BOOTLOADER_SECTION void sram_read_block(unsigned int add, uchar channel,uchar len, uchar* data_store) //TESTED OK
{

	// NOTE : The SRAM must be in sequencial mode for this function to work properly

	//	DECRIPTION : Reads a block of data at the address "add" 
	// 	from the SRAM determined by "channel" which is a 
	//	value from 0 to 3. 
	// 	len : number of bytes to read
	//	

	uchar pointer=0;
	ISOLATE_PROBES
	sram_cs_reset();
	sram_shit_out_byte(INST_READ,channel);
	sram_shit_out_word(add, channel);
	while (pointer < len)
	{
		*(data_store+pointer) = sram_shift_byte_in(channel);
		pointer++;
	}
	sram_cs_end();
}

BOOTLOADER_SECTION void sram_write_block(uchar channel,uchar len, uchar* data_store) //TO BE TESTED
{

	// NOTE : The SRAM must be in sequencial mode for this function to work properly, also, 
	// a new WRITE sequance must have been allready started with the correct start address

	uchar pointer=0;
	ISOLATE_PROBES
	//sram_cs_reset();
	//sram_shit_out_byte(INST_READ,channel);
	//sram_shit_out_word(add, channel);
	while (pointer < len)
	{
		sram_shit_out_byte(*(data_store+pointer),channel);
		pointer++;
	}
	//sram_cs_end();
}

BOOTLOADER_SECTION void sram_write_status(uchar val)
{
	//	DECRIPTION : Writes STATUS register with the byte "val" 
	//	for all SRAMS at the same time
	ISOLATE_PROBES
	sram_cs_reset();
	sram_shit_out_byte(1,CH_ALL);
	sram_shit_out_byte(val,CH_ALL);
	sram_cs_end();
}

BOOTLOADER_SECTION uchar sram_read_status()
{
	//	DECRIPTION : Reads the STATUS register from SRAM number 0
	uchar val;
	ISOLATE_PROBES
	val = 0;
	sram_cs_reset();
	sram_shit_out_byte(5,CH_ALL);
	val = sram_shift_byte_in(0);
	sram_cs_end();
	return val;
}


BOOTLOADER_SECTION void sram_write_byte(unsigned int add, uchar val, uchar channel)
{
	//	DECRIPTION : Writes the byte "val" at the address "add" on the 
	//	SRAM determined by "channel" which is value ranging from 0 to 3
	ISOLATE_PROBES
	sram_cs_reset();
	sram_shit_out_byte(INST_WRITE,channel);
	sram_shit_out_word(add, channel);
	sram_shit_out_byte(val,channel);
	sram_cs_end();
}


BOOTLOADER_SECTION void sram_shit_out_byte(uchar val, uchar channel)
{
	//	DECRIPTION : Intermediate level function used by High level functions.
	//	Used to clock out the 8 bits of "val" on the MOSI of the SRAM defined
	//	by "channel"
	uchar i;
	for(i=8;i>0;i--) 
	{
		sram_out(((uchar)(val>>(i-1))&1), channel); // write bit 
		sram_clk(0);
		sram_clk(1);
	}
}

BOOTLOADER_SECTION void sram_shit_out_mixed_byte(uchar val1, uchar val2)
{
	//	DECRIPTION : Intermediate level function used by High level functions.
	// Shift out differnet data on the channels : 
	// on channel 0,1 : shift out the byte "val1"
	// on channel 2,3 : shift out the byte "val2"
	uchar i;
	for(i=8;i>0;i--) 
	{
		sram_out(((uchar)(val2>>(i-1))&1), CH_ALL); // write bit 
		sram_out(((uchar)(val1>>(i-1))&1), 0); // write bit 
		sram_out(((uchar)(val1>>(i-1))&1), 1); // write bit 
		sram_clk(0);
		sram_clk(1);
	}
}


BOOTLOADER_SECTION void sram_shit_out_word(unsigned int val, uchar channel)
{
	//	DECRIPTION : Intermediate level function used by High level functions.
	//	Used to clock out the 16 bits of "val" on the MOSI of the SRAM defined
	//	by "channel". Mainly used to send a 16 bit address

	uchar i;
	for(i=16;i>0;i--) 
	{
		sram_out(((uchar)(val>>(i-1))&1), channel); // write bit 
		sram_clk(0);		
		sram_clk(1);
	}
}

BOOTLOADER_SECTION uchar sram_shift_byte_in(uchar channel)
{	
	//	DECRIPTION : Intermediate level function used by High level functions.
	//	Used to clock in 8 bit from the MISO line of the SRAM defined by
	//	"channel". 
	volatile uchar i,val = 0;
	for(i=0;i<7;i++) //Get first 7 MSBs 
	{
		sram_clk(0);
		//sram_out_all((uchar)(val>>(7-i))&1); // write bit 
		sram_clk(1);
		val |= ((PIN_MISO & (1<<(channel+3)))>>(channel+3));
		val = (val << 1);//shift left, make place for comming bit
	}
	sram_clk(0); 	// Clock in last LSB
	sram_clk(1);	// without shofting anymore to the left
	val |= ((PIN_MISO & (1<<(channel+3)))>>(channel+3));
	return val;
}


BOOTLOADER_SECTION void sram_out(uchar val,uchar channel)
{
	//	DECRIPTION : Lowest level function used by intermediate level functions.
	//	Used to set/clear the MISO line of the SRAM defines by "channel"
	if (channel == CH_ALL)
	{
		switch(val)
		{
			case 0: //Hard low level
				DDR_MOSI |= MOSI_MASK;				//set all pins as output
				PORT_MOSI &= ~(MOSI_MASK);		//set all pins low
			break;
			case 1:	//Soft high level via pull-ups (to respect 3V6 SRAM level)
				DDR_MOSI &= ~(MOSI_MASK);		//set all pins as inputs
				PORT_MOSI |= MOSI_MASK;		//Activate pull-up on all pins
			break;
			default:
			break;
		}
	}else{
		switch(val)
		{
			case 0: //Hard low level
				DDR_MOSI |= (1<<(channel));				//set all pins as output
				PORT_MOSI &= ~(1<<(channel));		//set all pins low
			break;
			case 1:	//Soft high level via pull-ups (to respect 3V6 SRAM level)
				DDR_MOSI &= ~(1<<(channel));		//set all pins as inputs
				PORT_MOSI |= (1<<(channel));		//Activate pull-up on all pins
			break;
			default:
			break;
		}
	}
}

BOOTLOADER_SECTION void sram_clk(uchar val)
{
	//	DECRIPTION : Lowest level function used by intermediate level functions.
	// Directly set/clear the CLK pin
	// To be used for low speed clock or for SRAM setup phases.
	switch(val)
	{
		case 0: //Hard low level
			DDR_CLK |= CLK;				//set pin as output
			PORT_CLK &= ~(CLK);		//set pin low
		break;
		case 1:	//Soft high level via pull-ups (to respect 3V6 SRAM level)
			DDR_CLK |= CLK;				//set pin as output
			PORT_CLK |= CLK;		//Activate pull-up on pin
		break;
		default:
		break;
	}
	//_delay_us(300);
}

BOOTLOADER_SECTION void sram_cs_reset()
{
	//	DECRIPTION : Initiate a new SPI sequence in a safe way on all SRAMS
	SRAM_CS_DIS;
	sram_clk(0);
	SRAM_CS_ENA;
	sram_clk(0);
}

BOOTLOADER_SECTION void sram_cs_end()
{
	//	DECRIPTION : Terminates an SPI sequence on any/all SRAMS
	sram_clk(0);
	SRAM_CS_DIS;
}


BOOTLOADER_SECTION void sram_sequence_setup(uchar stop_clk_sel,uchar freq_clk_sel,unsigned int stop_val, uchar freq_val)
{
	//	DECRIPTION : Setups 16bit timer 1 to generate an interrupt after a predetermined
	//  time that is synchronised with the clock of the system (internal or external).
	//
	//	"freq_clk_sel" and "stop_clk_sel" controls the the CS10 to CS12 bits of the 
	//	TCCR2 and TCCR1 reppectively.

	//	"stop_val" controls the value of OCR1A register

	//	Controlling the frequency : 	
	//  "freq_val" Controls the value of OCR2A register
	
	//	
	sram_sequence_busy = 0;
	SRAM_HS_CLK_DIS
	//timer 1 is used to stop timer 2
	TCCR1A = 0;
	OCR1A = stop_val; 
	TCCR1B = 0;
	TCCR1B_temp = (1<<WGM12)|((stop_clk_sel&0x7)<<CS00); 
	TCNT1 = 0;
	TIMSK1 |= (1<<OCIE1A); //activate interrupts


	//timer 2 is used to generate the clk
	TCCR2A |= (1<<WGM21);
	TCCR2B |= (freq_clk_sel&0x7);
	OCR2A = freq_val;	
}

BOOTLOADER_SECTION void sram_sequence_start()
{
	//start T2 which generates the clk
	TCCR2A |= (1<<COM2A0);
	//start T1
	TCCR1B = TCCR1B_temp;
	sram_sequence_busy = 1;	
}

BOOTLOADER_SECTION void sram_internal_clk(uchar mode, uchar clk_sel,uchar val) //now this function will be used to set the lock bits
{

	boot_lock_bits_set(mode);
/*
	//Setups the internal clock on OC2A pin to drive the SRAMs.
	// mode: 		directly controls the COM2A1 and COM2A0 bits of TCCRA register
	//=======		0 to disconnect OC2
	//					1 to use it for clock output
	//
	//clk_sel:	Controls the CS20 to CS22 pins of the TCCRB register
	//========	
	//
	//val:			Controls the value of OCR2A register
	sram_sequence_busy = 1;
	//TCNT2 = 0;
	//
	TCCR2A = ((mode&0x3)<<COM2A0) | (1<<WGM21);
	TCCR2B = (clk_sel&0x7);
	OCR2A = val;
	*/

}

/*void sram_cs(uchar val)
{
	switch(val)
	{
		case 0: //Hard low level			
			SRAM_CS_ENA;
		break;
		case 1:	//Soft high level via pull-ups (to respect 3V6 SRAM level)
			SRAM_CS_DIS;
		break;
		default:
		break;
	}
}*/
/*void sram_high_speed_clk(uchar ena)
{
	DDR_CLK_ENA |= CLK_ENA; //SET CLK_ENA pin as OUTPUT
	switch(ena)
	{
		case 0: 
			PORT_CLK_ENA &= ~(CLK_ENA);		//set pin low
			// Disable clock from ATMEGA
			DDR_CLK &= ~(CLK);		//set pin as inputs
			PORT_CLK |= CLK;		//Activate pull-up on pin
		break;
		case 1:	
			PORT_CLK_ENA |= CLK_ENA;		//set pin High
		break;
		default:
		break;
	}
}*/
/*uchar sram_read_byte_old(unsigned int add)
{
	uchar i,val;
	val = 0;

	DDR_MISO &= ~(MISO_MASK); //set MOSI pin as inputs
	PORT_MISO |= MISO_MASK;
	sram_cs(DIS);
	sram_clk(0);
	sram_cs(ENA);
	sram_clk(0);
	for(i=0;i<8;i++) //Send READ instruction
	{
		sram_out_all((uchar)(INST_READ>>(7-i))&1); // write bit 
		sram_clk(0);
		sram_clk(1);
	}
	for(i=0;i<16;i++) //Send address
	{
		sram_out_all((uchar)(add>>(15-i))&1); // write bit 
		sram_clk(0);		
		sram_clk(1);
	}
	for(i=0;i<7;i++) //Get first 7 MSBs 
	{
		sram_clk(0);
		//sram_out_all((uchar)(val>>(7-i))&1); // write bit 
		sram_clk(1);
		val |= ((PIN_MISO & (1<<MISO0))>>MISO0);
		val = (val << 1);//shift left, make place for comming bit
	}
	sram_clk(0); 	// Clock in last LSB
	sram_clk(1);	// without shofting anymore to the left
	val |= ((PIN_MISO & (1<<MISO0))>>MISO0); 

	sram_clk(0);
	sram_cs(DIS);
return val;
}*/


/*void sram_write_byte_old(unsigned int add, uchar val)
{
	uchar i;
	sram_cs(DIS);
	sram_clk(0);
	sram_cs(ENA);
	sram_clk(0);
	for(i=0;i<8;i++) //Send WRITE instruction
	{
		sram_out_all((uchar)(INST_WRITE>>(7-i))&1); // write bit 
		sram_clk(0);
		sram_clk(1);
	}
	for(i=0;i<16;i++) //Send address
	{
		sram_out_all((uchar)(add>>(15-i))&1); // write bit 
		sram_clk(0);
		sram_clk(1);
	}
	for(i=0;i<8;i++) //Send data
	{
		sram_out_all((uchar)(val>>(7-i))&1); // write bit 
		sram_clk(0);		
		sram_clk(1);
	}
	sram_clk(0);
	sram_cs(DIS);
}

void sram_out_all(uchar val)
{
	switch(val)
	{
		case 0: //Hard low level
			DDR_MOSI |= MOSI_MASK;				//set all pins as output
			PORT_MOSI &= ~(MOSI_MASK);		//set all pins low
		break;
		case 1:	//Soft high level via pull-ups (to respect 3V6 SRAM level)
			DDR_MOSI &= ~(MOSI_MASK);		//set all pins as inputs
			PORT_MOSI |= MOSI_MASK;		//Activate pull-up on all pins
		break;
		default:
		break;
	}
}*/
