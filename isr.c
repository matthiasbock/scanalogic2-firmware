// ISRs

#include "header.h"


ISR(TIMER0_COMPA_vect,ISR_NOBLOCK)
{
	//Executed at 50Hz
 usbPoll();

 if (new_usb_data == 1)
	{
		new_usb_data = 0;
		pc_connected = 1;
		switch(stream[0])
		{
			case USB_SEQUENCE_CONFIG:
				
				//memset	((void*)(&stream[0]),0,128); //clear the data stream
				if (state_machine ==0)
				{
					state_machine = 1; //start a new sequences
				}
			break;
			case USB_SEQUENCE_LIVE_MODE:
				state_machine = 6;
			break;
				
			case USB_CLR_SRAMS:
					state_machine = 98;
			break;

			case USB_UPDATE_START:
				//start a new sequencial write at address 0
				ISOLATE_PROBES;
				update_channel = stream[1];		
				//sram_mixed_setup(INST_WRITE, INST_WRITE, 0);
				sram_single_setup(INST_WRITE, update_channel, 0);
			break;
			case USB_UPDATE:
				state_machine = 7;
			break;
			case USB_END_SEQUENCE:
				sram_cs_end(); //Stop the SRAM sequence
			break;
			case USB_UPDATE_BOOT:
				//sram_internal_clk(0x0, 0xFF,0xFF); // alow SPM
				//sram_internal_clk(0x10, 0xFF,0xFF); // alow SPM in app section
				bootloader();
			break;
			case USB_SEQUENCE_ABORT: //***************************************************
				abort_sequence = 1;
				state_machine = 99;
			break;
			case USB_GET_SN:
				state_machine = USB_GET_SN;
				usb_data_read = 0;
			break;
			default:	
			break;
		}

 
	}//end of new_usb_data

}

/*ISR(TIMER1_COMPA_vect,ISR_NOBLOCK)
{
		SRAM_CS_DIS; //terminate the stream, even if no stream is started
		PORTD &= ~(1<<0);
		//stop T1
		TCNT1 = 0;
		TCCR1B = 0;
		TIMSK1 &= ~(1<<OCIE1A); //prevent timer to re-interrupt
	
		//stop T2
		TCCR2B = 0;
		TCCR2A = 0; //TCCR2A &= ~(1<<COM2A0);
		
		sram_sequence_busy = 0;
}*/



ISR(TIMER1_OVF_vect,ISR_NOBLOCK)
{
	
	timer1_post_scaller++;
	//LED_ON;
}


ISR(PCINT1_vect,ISR_NAKED)
{
	//
//	uchar i,valid_low,;
	asm("push R24");
	stream[136] = PINC;
	asm("IN R0,0x3F"); //save SREG
	asm("push R0");
	asm("push R1");
	stream[135] ++;
	if (stream[135] > 1) PCMSK1 = 0x0;
	//
	//uchar sreg_bruk = SREG;
	//cli();
	
	asm("pop R1");
	asm("pop R0");
	asm("out 0x3F,R0");
	asm("pop R24");
	//sei();
	//SREG = sreg_bruk;
	reti();
}
