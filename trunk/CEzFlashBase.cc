#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <usb.h>

#include <iostream>

using namespace std;

#include "CEzFlashBase.h"

DWORD CEzFlashBase::Bulk ( HANDLE &hDev, unsigned long ControlCode,
                           unsigned long pipeNum, LPVOID buf,
                           unsigned long ByteCount )
{
//FIXME: this should probably be sending a control code to the dev.

/*
#define IOCTL_EZUSB_BULK_READ CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                       Ezusb_IOCTL_INDEX+19, \
                                       METHOD_OUT_DIRECT, \
                                       FILE_ANY_ACCESS)

#define IOCTL_EZUSB_BULK_WRITE  CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                         Ezusb_IOCTL_INDEX+20, \
                                         METHOD_IN_DIRECT, \
                                         FILE_ANY_ACCESS)
*/
//  result = usb_control_msg( cl20_dev, USB_TYPE_VENDOR | USB_RECIP_DEVICE |
//  USB_ENDPOINT_IN,
//    USB_REQ_GET_STATUS, 0, 0, NULL, 0, 5000 );

//int usb_control_msg(usb_dev_handle *dev, int requesttype, int request,
//        int value, int index, char *bytes, int size, int timeout);

  int r = -1;

  if ( ControlCode == IOCTL_EZUSB_BULK_READ )
  {
//    usb_control_msg ( hDev, 0, 0, ControlCode,
//      Ezusb_IOCTL_INDEX + 19, NULL, 0, EZ_TIMEOUT );

    r = usb_bulk_read ( hDev, pipeNum, buf, ByteCount, EZ_TIMEOUT );
    //r = usb_bulk_read ( hDev, 7, buf, ByteCount, EZ_TIMEOUT );
  }
  else if ( ControlCode == IOCTL_EZUSB_BULK_WRITE )
  {
//    usb_control_msg ( hDev, 0, 0, ControlCode,
//      Ezusb_IOCTL_INDEX + 20, NULL, 0, EZ_TIMEOUT );

    r = usb_bulk_write ( hDev, pipeNum, buf, ByteCount, EZ_TIMEOUT );
    //r = usb_bulk_write ( hDev, 2, buf, ByteCount, EZ_TIMEOUT );
  }

  return r;
}

void CEzFlashBase::CartRead(HANDLE &hDev, DWORD StartAddr, BYTE* pbuf, unsigned long ByteCount)
{
	ctrlbuf[0] = ROM_Read;
	*(DWORD *)&ctrlbuf[1] = StartAddr;
//	Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,3,(char*)ctrlbuf,4);
	Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,4,(char*)ctrlbuf,4);
	//sleep(5);
//	Bulk(hDev,IOCTL_EZUSB_BULK_READ,8,(char*)pbuf,ByteCount);
	Bulk(hDev,IOCTL_EZUSB_BULK_READ,2,(char*)pbuf,ByteCount);
}

void CEzFlashBase::CartReadEx(HANDLE &hDev, DWORD StartAddr, BYTE* pbuf, unsigned long ByteCount)
{
  CartRead ( hDev, ( StartAddr >> 1 ) & 0xFFFFFF, pbuf, ByteCount );
}

//---------------------------------------------------------------------------
void CEzFlashBase::CartRAMRead(HANDLE &hDev, WORD StartAddr, BYTE* pbuf, unsigned long ByteCount)
{
	ctrlbuf[0] = RAM_Read;
	*(WORD *)&ctrlbuf[1] = StartAddr;
//	Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,3,(char*)ctrlbuf,3);
	Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,4,(char*)ctrlbuf,3);
	//sleep(5);
//	Bulk(hDev,IOCTL_EZUSB_BULK_READ,8,(char*)pbuf,ByteCount);
	Bulk(hDev,IOCTL_EZUSB_BULK_READ,4,(char*)pbuf,ByteCount);
}
//---------------------------------------------------------------------------
void CEzFlashBase::CartRAMWrite(HANDLE &hDev, WORD StartAddr, BYTE* pbuf, unsigned long ByteCount)
{
	ctrlbuf[0] = RAM_Write;
	*(WORD *)&ctrlbuf[1] = StartAddr;
//	Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,3,(char*)ctrlbuf,3);
	Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,4,(char*)ctrlbuf,3);
	//sleep(5);
//	Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,1,(char*)pbuf,ByteCount);
	Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,2,(char*)pbuf,ByteCount);
}

void CEzFlashBase::CartRAMWriteEx(HANDLE &hDev, WORD StartAddr, BYTE* pbuf, unsigned long ByteCount)
{
  DWORD realaddress = 0;
  realaddress = (StartAddr>>1)&0xFFFFFF;
  CartRAMWrite(hDev,realaddress,pbuf,ByteCount);
}

DWORD CEzFlashBase::WriteDevice(HANDLE &hDev,UINT address,WORD data)
{
	ctrlbuf[0] = Write_Operation;
	ctrlbuf[1] = (address)&0xFF;
	ctrlbuf[2] = (address>>8)&0xFF;
	ctrlbuf[3] = (address>>16)&0xFF;
	*(WORD *)&ctrlbuf[4] = data;

//	return  Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,3,(char*)ctrlbuf,6);
	return  Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,4,(char*)ctrlbuf,6);

}
WORD CEzFlashBase::ReadDevice(HANDLE &hDev,UINT address)
{
	ctrlbuf[0] = Read_Operation;
	ctrlbuf[1] = (address)&0xFF;
	ctrlbuf[2] = (address>>8)&0xFF;
	ctrlbuf[3] = (address>>16)&0xFF;
	//Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,3,(char*)ctrlbuf,4);
	Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,4,(char*)ctrlbuf,4);
	//Bulk(hDev,IOCTL_EZUSB_BULK_READ,10,(char*)ctrlbuf,64);
	Bulk(hDev,IOCTL_EZUSB_BULK_READ,4,(char*)ctrlbuf,64);
	return(*(WORD *)ctrlbuf);

}

//---------------------------------------------------------------------------
void CEzFlashBase::CartOpenFlashOP(HANDLE &hDev)
{
/*	ctrlbuf[0] = Open_Flash_Op;
	Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,3,ctrlbuf,1);
*/
	WriteDevice(hDev,0xff0000,0xd2FF);
	WriteDevice(hDev,0x000000,0x15FF);
	WriteDevice(hDev,0x010000,0xd2FF);
	WriteDevice(hDev,0x020000,0x15FF);
	WriteDevice(hDev,0xe20000,0x15FF);
	WriteDevice(hDev,0xfe0000,0x15FF);

}
//---------------------------------------------------------------------------
void CEzFlashBase::CartCloseFlashOP(HANDLE &hDev)
{
/*	ctrlbuf[0] = Close_Flash_Op;
	Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,3,ctrlbuf,1);
*/
	WriteDevice(hDev,0xff0000,0xd2FF);
	WriteDevice(hDev,0x000000,0x15FF);
	WriteDevice(hDev,0x010000,0xd2FF);
	WriteDevice(hDev,0x020000,0x15FF);
	WriteDevice(hDev,0xe20000,0xd2FF);
	WriteDevice(hDev,0xfe0000,0x15FF);
}
//---------------------------------------------------------------------------
void CEzFlashBase::CartOpenReadOP(HANDLE &hDev)
{
/*	ctrlbuf[0] = Open_Flash_Op;
	Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,3,ctrlbuf,1);
*/
  ctrlbuf [ 0 ] = Open_Read_Op;
  Bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, ( LPVOID ) ctrlbuf, 1 );
}
//---------------------------------------------------------------------------
void CEzFlashBase::CartCloseReadOP(HANDLE &hDev)
{
  ctrlbuf [ 0 ] = Close_Read_Op;
  Bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, ( LPVOID ) ctrlbuf, 1 );
}
//---------------------------------------------------------------------------
void CEzFlashBase::CartSetRAMPage(HANDLE &hDev,DWORD Offset)
{
/*	ctrlbuf[0] = Set_SRAM_Page;
	ctrlbuf[1] = (BYTE)Offset&0xFF;
	ctrlbuf[2] = (BYTE)(Offset>>8)&0xFF;
	Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,3,ctrlbuf,3);
	*/
	WriteDevice(hDev,0xff0000,0xd2FF);
	WriteDevice(hDev,0x000000,0x15FF);
	WriteDevice(hDev,0x010000,0xd2FF);
	WriteDevice(hDev,0x020000,0x15FF);
	WriteDevice(hDev,0xe00000,(WORD)Offset);
	WriteDevice(hDev,0xfe0000,0x15FF);
}
//---------------------------------------------------------------------------
void CEzFlashBase::CartSetROMPage(HANDLE &hDev,DWORD Offset)
{
/*	ctrlbuf[0] = Set_Flash_Offset;
	ctrlbuf[1] = (BYTE)Offset&0xFF;
	ctrlbuf[2] = (BYTE)(Offset>>8)&0xFF;
	Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,3,ctrlbuf,3);
	*/
	WriteDevice(hDev,0xff0000,0xd2FF);
	WriteDevice(hDev,0x000000,0x15FF);
	WriteDevice(hDev,0x010000,0xd2FF);
	WriteDevice(hDev,0x020000,0x15FF);
	WriteDevice(hDev,0xc40000,(WORD)Offset);
	WriteDevice(hDev,0xfe0000,0x15FF);

}

bool CEzFlashBase::OpenCartDevice ( HANDLE &hDev )
{
  usb_init ();

#ifdef DEBUG
  usb_set_debug ( 255 );
#endif

  int r = usb_find_busses ();

  if ( r == -2 )
    return false;
 
  r = usb_find_devices ();

  struct usb_device* device;
  struct usb_bus* bus;

  // usb_busses is an extern of libusb!
  if ( usb_busses == NULL )
    return false;

  bool found = false;
  
  // search for our man, first through busses ...
  for ( bus = usb_busses; bus != NULL; bus = bus->next )
  {
    // ... and then through it's devices
    for ( device = bus->devices; device != NULL; device = device->next )
    {
      if ( device->descriptor.idVendor == EZ_VENDOR &&
           device->descriptor.idProduct == EZ_PRODID )
      {
        found = true;
        break;
      }
    }
  }

  if ( ! found )
    return false;

  hDev = usb_open ( device );

  if ( hDev == NULL )
    return false;

  if ( usb_claim_interface ( hDev, 0 ) < 0 )
    return false;

  usb_set_configuration ( hDev, 1 );

  return true;
}

bool CEzFlashBase::CloseCartDevice ( HANDLE &hDev )
{
  usb_release_interface ( hDev, 0 );

  return usb_close ( hDev ) == 0;
}

void CEzFlashBase::CartOpenPort ( HANDLE &hDev )
{
  ctrlbuf [ 0 ] = Open_Port;
  
  Bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, ( char* ) ctrlbuf, 1 );
}

void CEzFlashBase::CartClosePort ( HANDLE &hDev )
{
  ctrlbuf [ 0 ] = Close_Port;
  
  Bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, ( char* ) ctrlbuf, 1 );
}
