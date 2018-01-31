
#include "header.h"

PROGMEM const char usbHidReportDescriptor[22] = {    /* USB report descriptor */
    0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 128,                    //   REPORT_COUNT (33) //WAS 128
    0x09, 0x00,                    //   USAGE (Undefined)
    0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
    0xc0                           // END_COLLECTION
};

/* ------------------------------------------------------------------------- */

/* usbFunctionRead() is called when the host requests a chunk of data from
 * the device. For more information see the documentation in usbdrv/usbdrv.h.
 */
uchar   usbFunctionRead(uchar *data, uchar len) //sends data to HOST
{
    if(len > bytesRemaining)
        len = bytesRemaining;
    ////eeprom_write_block((uchar *)stream, (uchar *)0 + currentAddress, len);
    //	eeprom_read_block(data, (uchar *)0 + currentAddress, len);
    //memset(data,0xDD,len);
    //data = 0;
    memcpy (data,(uchar *)stream+currentAddress,len);
    currentAddress += len;
    bytesRemaining -= len;
    if (bytesRemaining == 0)
    {
        usb_data_read = 1;

    }
    return len;
}

/* usbFunctionWrite() is called when the host sends a chunk of data to the
 * device. For more information see the documentation in usbdrv/usbdrv.h.
 */
uchar   usbFunctionWrite(uchar *data, uchar len)
{
    usb_alive = 0;
    if(bytesRemaining == 0)
    {
        new_usb_data = 1;                   // ADDED BY IKALOGIC
        return 1;               /* end of transfer */
    }
    if(len > bytesRemaining)
        len = bytesRemaining;
    //eeprom_write_block(data, (uchar *)0 + currentAddress, len);
    memcpy ((uchar *)stream+currentAddress,data,len);
    currentAddress += len;
    bytesRemaining -= len;
    //return bytesRemaining == 0; /* return 1 if this was the last chunk */

    // ADDED BY IKALOGIC
    if(bytesRemaining == 0)
    {
        new_usb_data = 1;
        return 1;               /* end of transfer */
    }
    return bytesRemaining == 0;
}

/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    usbRequest_t    *rq = (void *)data;
    usb_alive = 0;
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {    /* HID class request */
        if(rq->bRequest == USBRQ_HID_GET_REPORT) {  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* since we have only one report type, we can ignore the report-ID */
            bytesRemaining = 128;
            currentAddress = 0;
            return USB_NO_MSG;  /* use usbFunctionRead() to obtain data */
        }
        else
        if(rq->bRequest == USBRQ_HID_SET_REPORT) {
            /* since we have only one report type, we can ignore the report-ID */
            bytesRemaining = 128;
            currentAddress = 0;
            return USB_NO_MSG;  /* use usbFunctionWrite() to receive data from host */
        }
    }
    else
    {
        /* ignore vendor type requests, we don't use any */
    }
    return 0;
}
