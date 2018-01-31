
#include "header.h"

volatile uchar test,t2;

int main(void)
{

    volatile uchar i,j;
    volatile uchar signature;
    volatile unsigned int trigger_count_delay,delay_count;
    //sram_internal_clk(0x14, 0xFF,0xFF); // prohibit SPM
    //
    sram_sequence_busy = 0;
    //wdt_disable();
    MCUSR &= ~(1<<WDRF);
    WDTCSR |= (1<<WDCE) | (1<<WDE);     //Start timed sequence to edit watchdog configuration
    WDTCSR = 0;
    _delay_ms(200);
    cli();
    usbInit();

    usbDeviceDisconnect();      /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    j = 1;
    //while(--j){             /* fake USB disconnect for > 250 ms */
    while(--i)                      /* fake USB disconnect for > 250 ms */
    {
        _delay_ms(1);
    }
    //}
    usbDeviceConnect();

    setup_timers();

    sei();

    ISOLATE_PROBES;

    DDR_CLK_ENA |= CLK_ENA;     //Set CLK_ENA as output
    DDR_MISO &= ~(MISO_MASK);     //set MOSI pins as inputs
    DDR_CLK |= CLK;                 //set pin as output

    //LED_ON;
    //_delay_ms(100);
    LED_OFF;

    //sram_cs(DIS);
    _delay_ms(10);
    //sram_high_speed_clk(DIS);
    //sram_clk(0);
    //sram_clk(0);
    SRAM_HS_CLK_DIS;
    //sram_out(0,0);
    _delay_ms(90);
    //test = sram_read_status();
    sram_write_status((1<<0)|(1<<6));     //Set sequencial read/write mode
    _delay_ms(90);
    sram_write_byte(0x00AA,0xA0, 0);
    sram_write_byte(0x00AA,0xA1, 1);
    sram_write_byte(0x00AA,0xA2, 2);
    sram_write_byte(0x00AA,0xA3, 3);
    _delay_ms(90);
    sram_write_byte(0x0AAA,0x0A, CH_ALL);
    _delay_ms(90);
    //added on 08/03/2012
    sram_write_status((1<<0)|(1<<6));     //Set sequencial read/write mode
    _delay_ms(90);

    memset((void*)stream,0,128);     //very important, STREAM must not contain 0's
    stream[1] = 99;     //Tell PC i finished sending data, and/or currently IDLE
    stream[0] = USB_FEEDBACK;

    my_sn[0] = eeprom_read_byte((void*)3);
    my_sn[1] = eeprom_read_byte((void*)2);
    my_sn[2] = eeprom_read_byte((void*)1);
    my_sn[3] = eeprom_read_byte((void*)0);

    state_machine= 99;

    pc_connected = 0;
    while(1)
    {

        switch(state_machine)
        {
        case 99:         //IDLE
        case 0:         //IDLE
            //gen_mode = 0;
            if (gen_mode == 1)
            {
                LED_ON;
            }
            else
            {
                idle_blink_counter++;
                if (idle_blink_counter > 640000)
                {
                    idle_blink_counter=0;
                }

                if (pc_connected == 1)
                {
                    if ((idle_blink_counter == 0) || (idle_blink_counter == 300000))
                    {
                        if (usb_alive < 0xF0) usb_alive++;
                    }
                    if (usb_alive < 3) {
                        //LED_ON;

                    }
                    else
                    {
                        //LED_OFF; //error.. there should be a comunication with the PC software...
                        wdt_enable(WDTO_250MS);
                        while(1)
                        {
                            LED_OFF;
                        }
                    }
                }
                else
                {
                    //LED_ON;
                }
                if (idle_blink_counter < 160000)
                {
                    LED_ON;
                }
                else
                {
                    LED_OFF;
                }
            }
            break;
        }

        switch(state_machine)
        {
        case 0:         //IDLE
            //gen_mode = 0;
            trigged = 0;
            break;
        case USB_GET_SN:
            stream[1] = my_sn[0];
            stream[2] = my_sn[1];
            stream[3] = my_sn[2];
            stream[4] = my_sn[3];
            stream[5] = FWV_MAJ;
            stream[6] = FWV_MIN;
            //LED_ON;
            if (usb_data_read == 1)         //make sure previous data have been read by HOST
            {
                //LED_OFF;
                memset((void*)stream,0,128);
                state_machine=0;             //IDLE STATE
                stream[1] = 99;             //Tell PC i finished sending data, and/or currently IDLE
                stream[0] = USB_FEEDBACK;
                stream[2] = 0;
                stream[3] = 0;
                stream[4] = 0;
                usb_data_read = 0;

            }

            break;
        case 6:         //fast read mode (for live update on PC end), to exit this mode, PC have to send ABORT (0x2)
            usb_data_read = 0;
            ISOLATE_ATMEGA;         //set MOSI lines as input
            //all IN
            SET_CH12_IN;
            SET_CH34_IN;

            //stream[0] = 199;
            stream[1] = (PIN_MOSI & (1<<MOSI0))>>MOSI0;
            stream[2] = (PIN_MOSI & (1<<MOSI1))>>MOSI1;
            stream[3] = (PIN_MOSI & (1<<MOSI2))>>MOSI2;
            stream[4] = (PIN_MOSI & (1<<MOSI3))>>MOSI3;
            break;
        case 1:         //Setting up sequence
            LED_OFF;

            //Re initialise the SRAMs
            sram_write_status((1<<0)|(1<<6));         //Set sequencial read/write mode
            DDR_CLK_ENA |= CLK_ENA;         //Set CLK_ENA as output
            DDR_MISO &= ~(MISO_MASK);         //set MOSI pins as inputs
            DDR_CLK |= CLK;                     //set pin as output
            _delay_ms(10);
            SRAM_HS_CLK_DIS;

            //Start a new sequence
            sampling_done = 0;
            abort_sequence = 0;
            PCMSK1 = 0x0;
            PCIFR = 0xFF;

            //Workaround to reduce noize on floating probes
            /*
               sram_out(0,CH_ALL);
               SET_CH12_IN
               SET_CH34_IN
               CH12_OUT_ENA
               CH34_OUT_ENA
               _delay_us(5);
               SET_CH12_IN
               SET_CH34_IN
             */

            ISOLATE_PROBES;

            //*** clear SRAM content
            switch(stream[1])
            {
            case 0:             //all in, write to SRAM
                //clear  all SRAMS
                sram_mixed_setup(INST_WRITE, INST_WRITE, 0);
                sram_out(0,CH_ALL);
                SRAM_HS_CLK_ENA;
                _delay_ms(250);
                SRAM_HS_CLK_DIS;
                break;
            case 1:            //all output, read from SRAM
                               //don't clear anything, in case playback is needed
                break;
            case 2:            //1,2 OUT,
                sram_mixed_setup(INST_READ, INST_WRITE, 0);
                sram_out(0,2);
                sram_out(0,3);
                SRAM_HS_CLK_ENA;
                _delay_ms(250);
                SRAM_HS_CLK_DIS;
                break;
            case 3:            //3,4 out
                sram_mixed_setup(INST_WRITE, INST_READ, 0);
                sram_out(0,0);
                sram_out(0,1);
                SRAM_HS_CLK_ENA;
                _delay_ms(250);
                SRAM_HS_CLK_DIS;
                break;
            default:
                break;
            }

            //***** end of clear SRAM content

            sram_cs_end();         //Stop the SRAM sequence in case it was still active

            //read number of samples to store before trigger
            memcpy((void*)(&pre_trigger_samples), (void*)(&stream[2]),2);
            pre_trigger_samples = pre_trigger_samples * 8;
            //pre_trigger_samples = 50;

            //read number of samples to store after trigger
            memcpy((void*)(&post_trigger_samples), (void*)(&stream[4]),2);
            post_trigger_samples = post_trigger_samples * 8;

            //Setup data direction
            switch(stream[1])
            {
            case 0:             //all in, write to SRAM
                sram_mixed_setup(INST_WRITE, INST_WRITE, 0);
                gen_mode = 0;
                break;
            case 1:            //all output, read from SRAM
                sram_mixed_setup(INST_READ, INST_READ, 0);
                LED_ON;
                gen_mode = 1;
                break;
            case 2:            //1,2 OUT,
                sram_mixed_setup(INST_READ, INST_WRITE, 0);
                LED_ON;
                gen_mode = 0;
                break;
            case 3:            //3,4 out
                sram_mixed_setup(INST_WRITE, INST_READ, 0);
                LED_ON;
                gen_mode = 0;
                break;
            default:
                break;
            }
            SRAM_HS_CLK_DIS;
            ISOLATE_ATMEGA;         //Isolate MOSI line of the atmega, set all to inputs, no need to send anymore instructions to SRAMS

            //Activate buffers
            switch(stream[1])
            {
            case 0:             //all IN
                SET_CH12_IN;
                SET_CH34_IN;
                break;
            case 1:            //all OUT
                SET_CH12_OUT;
                SET_CH34_OUT;
                break;
            case 2:            //1,2 OUT
                SET_CH12_OUT;
                SET_CH34_IN;
                break;
            case 3:            //3,4 out
                SET_CH12_IN;
                SET_CH34_OUT;
                break;
            default:
                break;
            }

            //Set trigger type
            trigger_type = stream[7];

            trigger_channel = stream[8];
            //Set trigger channel
            if (trigger_type != TRIG_NULL)
            {
                PCICR = (1<<PCIE1);
                switch(stream[8])
                {
                case 0:             //any channel can cause trigger
                    PCMSK1 = 0xF;
                    break;
                default:             //single channel tigger
                    PCMSK1 = (1<< (stream[8] -1));
                    break;
                }
            }

            stream[135] = 0;         //JFT***********

            memcpy((void*)(&stream[133]), (void*)(&stream[10]),2);         //copy the trigger count or delay value

            //Reset timer 1
            TCNT1 = 0; timer1_post_scaller = 0;

            // Start sampling clock, can be internal or external high speed clock.
            switch(stream[6])
            {
            case SR_20M:
                sample_length = 1;             //sample_length will determine the relation between timer 1 and the actual number of samples
                SRAM_HS_CLK_ENA;
                break;
            case SR_10M:
                OCR2A = 0;
                sample_length =2;              //sample_length will determine the relation between timer 1 and the actual number of samples
                SRAM_HS_CLK_DIS;
                START_TIMER2_DIV1;
                break;
            case SR_5M:
                OCR2A = 1;
                sample_length =4;              //sample_length will determine the relation between timer 1 and the actual number of samples
                SRAM_HS_CLK_DIS;
                START_TIMER2_DIV1;
                break;
            case SR_2M5:
                OCR2A = 3;
                sample_length =8;              //sample_length will determine the relation between timer 1 and the actual number of samples
                SRAM_HS_CLK_DIS;
                START_TIMER2_DIV1;
                break;
            case SR_1M:
                OCR2A = 9;
                sample_length =20;              //sample_length will determine the relation between timer 1 and the actual number of samples
                SRAM_HS_CLK_DIS;
                START_TIMER2_DIV1;
                break;
            case SR_500K:
                OCR2A = 19;
                sample_length =40;              //sample_length will determine the relation between timer 1 and the actual number of samples
                SRAM_HS_CLK_DIS;
                START_TIMER2_DIV1;
                break;
            case SR_250K:
                OCR2A = 39;
                sample_length =80;              //sample_length will determine the relation between timer 1 and the actual number of samples
                SRAM_HS_CLK_DIS;
                START_TIMER2_DIV1;
                break;
            case SR_100K:
                OCR2A = 99;
                sample_length =200;              //sample_length will determine the relation between timer 1 and the actual number of samples
                SRAM_HS_CLK_DIS;
                START_TIMER2_DIV1;
                break;
            case SR_50K:
                OCR2A = 199;
                sample_length =400;              //sample_length will determine the relation between timer 1 and the actual number of samples
                SRAM_HS_CLK_DIS;
                START_TIMER2_DIV1;
                break;
            case SR_10K:
                OCR2A = 124;
                sample_length =2000;              //sample_length will determine the relation between timer 1 and the actual number of samples
                SRAM_HS_CLK_DIS;
                START_TIMER2_DIV8;
                break;
            case SR_1K25:
                OCR2A = 249;
                sample_length =16000;              //sample_length will determine the relation between timer 1 and the actual number of samples
                SRAM_HS_CLK_DIS;
                START_TIMER2_DIV32;
                break;
            default:
                break;
            }

            if (gen_mode == 1)
            {
                state_machine=0;         //IDLE STATE
                stream[1] = 99;         //Tell PC i finished sending data, and/or currently IDLE
                stream[0] = USB_FEEDBACK;
            }
            else
            {
                //Start Timer 1
                START_TIMER1;

                //
                //sample_length *= 2;
                min_valid_signal_period = sample_length*5;         //used to reject noise for the trigger detection

                //note: Now the sampling is launched. the rest of the sequence control is carried out in the Pin Change interrupt routine
                //			which starts as soon as we get a trigger. In that function, timer 1 is pooled until the

                // to do : take care of always being able to exit sampling using the "cancel sequence" command via USB, even when
                //					scanalogic is waiting for trigger, we have to disable PinChange interrupt and, if we are in the post-trigger loop, exit that loop.
                //				The best way to do that is to add a variable called "stop_sampling", and in timer0 interrupt (that does usb pool, it also checks for
                //				USB stop command, and sets "stop_sampling = 1". in the post sampling pooling loop, add a condition to exit that loop in case "stop_sampling = 1"

                if (trigger_type == TRIG_NULL)
                {
                    //trigger_instant = TCNT1 +(timer1_post_scaller*65536);
                    //trigger_instant = 0;
                    trigger_instant = 0;        //pre_trigger_samples;
                    pre_trigger_samples = 0;
                    address_t = 0;
                    //LED_ON;
                    //pre_trigger_samples = 0;
                }

                state_machine = 2;
                //Tell PC software that we are busy waiting for trigger
                stream[1] = 97;         //Waiting for trigger
                stream[0] = USB_FEEDBACK;
            }
            break;

        case 2:         //waiting for trigger, then wait for end of sampling
            if (stream[135] > 1)
            {
                trigger_instant = TCNT1 +(timer1_post_scaller*65536);
                address_t = (0x3FFFF&((0x3FFFF & (trigger_instant/sample_length))-pre_trigger_samples))/8;         //address to start reading data
                trigged = 1;
                //LED_ON;
                PCMSK1 = 0x0;         //disable any further interrupts
            }
            else
            if (stream[135] > 0)
            {
                switch(trigger_type)
                {
                case 0:             //falling edge
                    if ( (stream[136]  & (1<<(trigger_channel - 1))) == 0)              //make sure the selected channel had the requested edge
                    {
                        if (trigged == 0)
                        {
                            trigger_instant = TCNT1 +(timer1_post_scaller*65536);
                            address_t = (0x3FFFF&((0x3FFFF & (trigger_instant/sample_length))-pre_trigger_samples))/8;             //address to start reading data
                            trigged = 1;
                            //LED_ON;
                            PCMSK1 = 0x0;             //disable any further interrupts
                        }
                    }
                    break;
                case 1:
                    if ( (stream[136] & (1<<(trigger_channel - 1))) != 0)              //make sure the selected channel had the requested edge
                    {
                        if (trigged == 0)
                        {
                            trigger_instant = TCNT1 +(timer1_post_scaller*65536);
                            address_t = (0x3FFFF&((0x3FFFF & (trigger_instant/sample_length))-pre_trigger_samples))/8;             //address to start reading data
                            trigged = 1;
                            //LED_ON;
                            PCMSK1 = 0x0;             //disable any further interrupts
                        }
                    }
                    break;
                case 2:
                    if (trigged == 0)
                    {
                        trigger_instant = TCNT1 +(timer1_post_scaller*65536);
                        address_t = (0x3FFFF&((0x3FFFF & (trigger_instant/sample_length))-pre_trigger_samples))/8;             //address to start reading data
                        trigged = 1;
                        //LED_ON;
                        PCMSK1 = 0x0;             //disable any further interrupts
                    }
                    break;
                default:
                    break;
                }
            }
            //wait for trigger to start pooling timer1 (to catch exactly n_samples
            if ((trigged == 1) || (trigger_type == TRIG_NULL))
            {

                stream[1] = 98;         //Tell PC i am collecting data
                stream[0] = USB_FEEDBACK;
                trigger_instant = TCNT1 +(timer1_post_scaller*65536);

                memcpy((void*)(&trigger_count_delay), (void*)(&stream[133]),2);

                if (trigger_count_delay > 0)
                {
                    for (delay_count=0; delay_count< trigger_count_delay; delay_count++)
                    {
                        //_delay_ms(trigger_count_delay);
                        _delay_ms(1);
                    }
                    trigger_instant = TCNT1 +(timer1_post_scaller*65536);
                    address_t = (0x3FFFF&((0x3FFFF & (trigger_instant/sample_length))-pre_trigger_samples))/8;         //address to start reading data

                }

                while((abort_sequence == 0) && (sampling_done == 0))
                {
                    //check if sampling is done, according to the number of post trigger samples requested by the end user
                    //if (((((TCNT1 +(timer1_post_scaller*65536))/sample_length)) - trigger_instant) > post_trigger_samples) //******** added -trigger_instant
                    if  (
                        (
                            (
                                (TCNT1 +(timer1_post_scaller*65536))
                                - trigger_instant
                            )
                            /sample_length
                        )
                        > post_trigger_samples
                        )
                    {
                        //LED_OFF;
                        DDR_CS |= CS;   PORT_CS |= (CS);

                        //disable the clock to the SRAMS
                        SRAM_HS_CLK_DIS;
                        DDR_CLK |= (CLK); PORT_CLK &= ~(CLK);
                        STOP_TIMER2;
                        STOP_TIMER1;
                        SRAM_CS_DIS;        //sram_cs_end(); //Stop the SRAM sequence
                        //Send the data to PC
                        sampling_done = 1;
                        state_machine = 3;
                    }

                }

                if(abort_sequence == 1)
                {
                    SRAM_CS_DIS;
                    SRAM_HS_CLK_DIS;
                    STOP_TIMER2;
                    STOP_TIMER1;
                    state_machine = 99;
                }

            }

            if (abort_sequence == 1)
            {
                SRAM_CS_DIS;
                SRAM_HS_CLK_DIS;
                STOP_TIMER2;
                STOP_TIMER1;
                trigged = 1;
                state_machine = 99;
            }
            //stream[135] = PINC;
            break;

        case 3:         //sampling done send data to PC
            //Send data to PC if any

            stream[1] = 96;             //Tell PC i finished collecting data, waiting for transfere
            stream[0] = USB_FEEDBACK;
            usb_data_read = 0;

            //boost usb speed
            STOP_USB_TIMER;
            for (j=0; j<4; j++)             //loop to cycle between the 4 channels
            {
                //address = (0x3FFFF&((0x3FFFF & trigger_instant)-pre_trigger_samples))/8; //address to start reading data
                address = address_t;
                transfered_bytes=0;
                sending_done = 0;
                signature=0;
                while((abort_sequence == 0) && (sending_done == 0))
                {
                    usbPoll();
                    if (new_usb_data == 1)
                    {
                        new_usb_data = 0;
                        abort_sequence = 1;
                        state_machine = 99;
                    }
                    if (usb_data_read == 1)             //put new data in stream[] table, only if the previous data have been read by host
                    {
                        LED_ON;
                        //LED_OFF;
                        usb_data_read=0;
                        memset((void*)stream,0,128);
                        sram_read_block(address, j,124, (uchar*)&stream[4]);
                        //stream[4] = (address/124);
                        stream[1] = j;
                        stream[2]= signature++;             //just an indicator for the HOST to differentiate btw old/new data
                        stream[0] = USB_FEEDBACK;
                        address += 124;             //124 bytes have been sent
                        transfered_bytes += 124;
                        //if ((address & 0x7FFF) >= ((pre_trigger_samples+post_trigger_samples)/8))
                        if (transfered_bytes >= ((pre_trigger_samples+post_trigger_samples)/8))
                        {
                            sending_done=1;
                        }
                        LED_OFF;
                    }            //endif (usb_data_read == 1)
                }            //end while ((abort_sequence == 0) && (sending_done == 0))
            }            //end for loop to cycle between the 4 channels

            //back to normal speed
            START_USB_TIMER;

            state_machine=99;             //IDLE STATE
            break;
        case 7:         //Store usb update data in SRAM0

            STOP_USB_TIMER;
            new_usb_data = 1;
            sending_done = 0;
            gen_mode = 1;
            while(sending_done == 0)
            {
                usbPoll();
                if (new_usb_data == 1)
                {
                    if (stream[0] == 7)
                    {
                        new_usb_data = 0;
                        LED_ON;
                        //memcpy((void*)&temp_add,(void*)&stream[1],2);
                        //memset((void*)&stream[4],test_counter++,124);
                        sram_write_block(update_channel,124, (uchar*)&stream[4]);

                        LED_OFF;
                    }
                    else
                    {
                        sending_done = 1;
                    }
                }
            }
            START_USB_TIMER;
            state_machine = 0;         //after storing the data go back to idle
            break;
        case 98:
            //clear  all SRAMS
            sram_mixed_setup(INST_WRITE, INST_WRITE, 0);
            sram_out(0,CH_ALL);
            SRAM_HS_CLK_ENA;
            _delay_ms(250);
            SRAM_HS_CLK_DIS;
            sram_cs_end();
            state_machine = 0;
            break;

        case 99:
            if(gen_mode == 1)
            {
                SRAM_CS_DIS;
                SRAM_HS_CLK_DIS;
                STOP_TIMER2;
                sram_cs_end();             //Stop the SRAM sequence

                ISOLATE_PROBES;
                gen_mode = 0;
            }
            PCMSK1 = 0x0;
            //LED_OFF
            ISOLATE_PROBES;
            if (usb_data_read == 1)             //make sure previous data have been read by HOST
            {
                memset((void*)stream,0,128);
                state_machine=0;             //IDLE STATE
                stream[1] = 99;             //Tell PC i finished sending data, and/or currently IDLE
                stream[0] = USB_FEEDBACK;
                usb_data_read = 0;
            }
            pre_trigger_samples=0;
            post_trigger_samples=0;
            break;
        }    //end of switch(state_machine)

    }
    return 0;
}
