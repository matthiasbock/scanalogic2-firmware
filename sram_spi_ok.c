//Serial SRAM access

#include "header.h"

uchar sram_read_byte(unsigned int add, uchar channel)
{
	uchar i,val;
	val = 0;

	DDR_MISO &= ~(MISO_MASK); //set MOSI pin as inputs
	PORT_MISO |= MISO_MASK;
	SRAM_CS_DIS;
	sram_clk(0);
	SRAM_CS_ENA;
	sram_clk(0);

	sram_shit_out_byte(INST_READ,channel);
	sram_shit_out_word(add, channel);
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

	sram_clk(0);
	SRAM_CS_DIS;
return val;
}


void sram_write_status(uchar val)
{
	uchar i;
	SRAM_CS_DIS;
	sram_clk(0);
	SRAM_CS_ENA;
	sram_clk(0);
	sram_shit_out_byte(1,CH_ALL);
	sram_shit_out_byte(val,CH_ALL);
	sram_clk(0);
	SRAM_CS_DIS;
}

uchar sram_read_status()
{
	uchar i,val;
	val = 0;
	SRAM_CS_DIS;
	DDR_MISO &= ~(MISO_MASK); //set MIS0 pin as inputs
	sram_clk(0);
	SRAM_CS_ENA;
	sram_clk(0);
	
	sram_shit_out_byte(5,CH_ALL);
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
	SRAM_CS_DIS;
return val;
}


void sram_write_byte(unsigned int add, uchar val, uchar channel)
{
	uchar i;
	SRAM_CS_DIS;
	sram_clk(0);
	SRAM_CS_ENA;
	sram_clk(0);
	sram_shit_out_byte(INST_WRITE,channel);
	sram_shit_out_word(add, channel);
	sram_shit_out_byte(val,channel);
	sram_clk(0);
	SRAM_CS_DIS;

}


void sram_shit_out_byte(uchar val, uchar channel)
{
	uchar i;
	for(i=8;i>0;i--) 
	{
		sram_out(((uchar)(val>>(i-1))&1), channel); // write bit 
		sram_clk(0);
		sram_clk(1);
	}
}

void sram_shit_out_mixed_byte(uchar val1, uchar val2)
{
	//shift out differnet data on the channels : 
	// on channel 1 : shift out the byte val1
	// on channel 2,3 and 4 : shift out the byte val2
	uchar i;
	for(i=8;i>0;i--) 
	{
		sram_out_mixed(((uchar)(val>>(i-1))&1), 1); // write bit 
		sram_clk(0);
		sram_clk(1);
	}
}


void sram_shit_out_word(unsigned int val, uchar channel)
{
	uchar i;
	for(i=16;i>0;i--) 
	{
		sram_out(((uchar)(val>>(i-1))&1), channel); // write bit 
		sram_clk(0);		
		sram_clk(1);
	}
}


void sram_out(uchar val,uchar channel)
{
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
		DDR_MOSI &= ~(MOSI_MASK);		//set all pins as inputs
		PORT_MOSI |= MOSI_MASK;		//Activate pull-up on all pins
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

void sram_out_mixed(uchar val,uchar channel)
{
	//same as "sram_out" but the other channels are unchanged
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


void sram_clk(uchar val)
{
	//DDR_CLK_ENA |= CLK_ENA; //SET CLK_ENA pin as OUTPUT
	//PORT_CLK_ENA |= CLK_ENA; //disactivate high speed clock in case it is active
	switch(val)
	{
		case 0: //Hard low level
			DDR_CLK |= CLK;				//set pin as output
			PORT_CLK &= ~(CLK);		//set pin low
		break;
		case 1:	//Soft high level via pull-ups (to respect 3V6 SRAM level)
			//DDR_CLK &= ~(CLK);		//set pin as inputs
			DDR_CLK |= CLK;				//set pin as output
			PORT_CLK |= CLK;		//Activate pull-up on pin
		break;
		default:
		break;
	}
	_delay_us(10);
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
