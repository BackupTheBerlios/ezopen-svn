/*
  Convenience wrapper for the EZ-Writer delivered with
  the EZ-Flash II Powerstar GBA-cart.

  <reborn@users.berlios.de>
  <bagu@users.berlios.de> 
*/
#include "ezopen_ezf2.h"

bool ezopen_ezf2::cartOpen ( ez_handle_t &hDev )
{
  usb_init();
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
      if ( device->descriptor.idVendor == Ez_vendor &&
           device->descriptor.idProduct == Ez_prodid )
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
  
  if ( usb_set_configuration ( hDev, 1 ) < 0 )
    return false;

  return true;

  return found;
}

bool ezopen_ezf2::cartClose ( ez_handle_t &hDev )
{
  if ( usb_release_interface ( hDev, 0 ) < 0 )
    return false;

  if ( usb_close ( hDev ) < 0 )
    return false;

  return true;
}

bool ezopen_ezf2::readRAM ( ez_handle_t &hDev, u_int32_t offset, ez_byte_t* buf, u_int32_t bytecount )
{
  int r = -1;

  ezio.ctrlbuf[0] = RAM_Read;
  *(ez_word_t *)&ezio.ctrlbuf[1] = offset;
  r = ezio.bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, (char*) ezio.ctrlbuf, 3 );
  r += ezio.bulk ( hDev, IOCTL_EZUSB_BULK_READ, 2, (char*) buf, bytecount);

  if ( r < 0 )
    return false;

  return true;
}

bool ezopen_ezf2::writeRAM ( ez_handle_t &hDev, u_int32_t offset, ez_byte_t* buf, u_int32_t bs )
{
  int r = -1;

  ezio.ctrlbuf[0] = RAM_Write;
  *(ez_word_t *) &ezio.ctrlbuf[1] = offset;
  r = ezio.bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, (char*) ezio.ctrlbuf, 3 );
  r += ezio.bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 2, (char*) buf, bs );

  if ( r < 0 )
    return false;

  return true;

}

bool ezopen_ezf2::readROM ( ez_handle_t &hDev, u_int32_t offset, u_int32_t length, ez_byte_t* buf ) 
{
  int r = -1;
  // "(offset >> 1) & 0xFFFFFF" becomes the address where to start
  r = ezio.cartRead ( hDev, (offset >> 1) & 0xFFFFFF, buf, length );
  
  if ( r < 0 )
    return true;
  
  return false;
}

bool ezopen_ezf2::writeROM ( ez_handle_t &hDev, u_int32_t offset, ez_byte_t* buf, u_int32_t bs )
{
  int r = -1;

  r = ezio.cartWrite ( hDev, (offset >> 1) & 0xFFFFFF, buf, bs );

  if ( r < 0 )
    return true;
  
  return false;
}

bool ezopen_ezf2::prepRAMOp ( ez_handle_t &hDev, u_int32_t page )
{

  if ( ! ezio.prepFlashOp ( hDev ) )
    return false;

  int r = -1;

  // set RAM page
  r = ezio.writeDevice ( hDev, 0xff0000, 0xd2FF );
  r += ezio.writeDevice ( hDev, 0x000000, 0x15FF );
  r += ezio.writeDevice ( hDev, 0x010000, 0xd2FF );
  r += ezio.writeDevice ( hDev, 0x020000, 0x15FF );
  r += ezio.writeDevice ( hDev, 0xe00000, (ez_word_t) page );
  r += ezio.writeDevice ( hDev, 0xfe0000, 0x15FF );

  if ( r < 0 )
    return false;


  // set read array (from the fujitsu-class)
  r += ezio.writeDevice ( hDev, 0x555 * 2, 0xAA );
  r += ezio.writeDevice ( hDev, 0x2AA * 2, 0x55 );
  r += ezio.writeDevice ( hDev, 0x555 * 2, 0xF0 );
  r += ezio.writeDevice ( hDev, 0x555 * 2 + 1, 0xAA );
  r += ezio.writeDevice ( hDev, 0x2AA * 2 + 1, 0x55 );
  r += ezio.writeDevice ( hDev, 0x555 * 2 + 1, 0xF0 );

  if ( r < 0 )
    return false;

  return true;
}

bool ezopen_ezf2::closeRAMOp ( ez_handle_t &hDev )
{
  return ezio.closeFlashOp ( hDev );
}

bool ezopen_ezf2::prepROMOp ( ez_handle_t &hDev, u_int32_t page )
{
  int r = -1;

  //   CartSetROMPage ( h, 0 );
  r = ezio.writeDevice ( hDev, 0xff0000, 0xd2FF );
  r += ezio.writeDevice ( hDev, 0x000000, 0x15FF );
  r += ezio.writeDevice ( hDev, 0x010000, 0xd2FF );
  r += ezio.writeDevice ( hDev, 0x020000, 0x15FF );
  r += ezio.writeDevice ( hDev, 0xc40000, ( ez_word_t ) page );
  r += ezio.writeDevice ( hDev, 0xfe0000, 0x15FF );

  if ( r < 0 )
    return false;
	
  //   CartOpenPort ( h );
  ezio.ctrlbuf [ 0 ] = Open_Port;
  r += ezio.bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, ( char* ) ezio.ctrlbuf, 1 );

  if ( r < 0 )
    return false;

  //   CartOpenReadOP ( h );
  ezio.ctrlbuf [ 0 ] = Open_Read_Op;
  r += ezio.bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, ( ez_lpvoid_t ) ezio.ctrlbuf, 1 );

  if ( r < 0 )
    return false;

  //   SetReadArray ( h ) (from the fujitsu-class)
  r += ezio.writeDevice ( hDev, 0x555 * 2, 0xAA );
  r += ezio.writeDevice ( hDev, 0x2AA * 2, 0x55 );
  r += ezio.writeDevice ( hDev, 0x555 * 2, 0xF0 );
  r += ezio.writeDevice ( hDev, 0x555 * 2 + 1, 0xAA );
  r += ezio.writeDevice ( hDev, 0x2AA * 2 + 1, 0x55 );
  r += ezio.writeDevice ( hDev, 0x555 * 2 + 1, 0xF0 );

  if ( r < 0 )
    return false;

  return true;
}

bool ezopen_ezf2::closeROMOp ( ez_handle_t &hDev )
{
  int r = -1;

  // CartCloseReadOP ( h );
  ezio.ctrlbuf [ 0 ] = Close_Read_Op;
  r = ezio.bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, ( ez_lpvoid_t ) ezio.ctrlbuf, 1 );

  // CartClosePort ( h );
  ezio.ctrlbuf [ 0 ] = Close_Port;
  r += ezio.bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, ( char* ) ezio.ctrlbuf, 1 );

  if ( r < 0 )
    return true;
  
  return false;
}

void ezopen_ezf2::cartErase (ez_handle_t &hDev, ez_dword_t BlockNum)
{
  ez_word_t read1=1,read2=2;
  BlockNum = BlockNum&0xFF ;
  for(int i=0;i<2;i++)
    {
      ezio.writeDevice(hDev,0x555*2,0xAA);
      ezio.writeDevice(hDev,0x2AA*2,0x55);
      ezio.writeDevice(hDev,0x555*2,0x80);
      ezio.writeDevice(hDev,0x555*2,0xAA);
      ezio.writeDevice(hDev,0x2AA*2,0x55);
      ezio.writeDevice(hDev,((BlockNum+i)<<16),0x30);

      ezio.writeDevice(hDev,0x555*2+1,0xAA);
      ezio.writeDevice(hDev,0x2AA*2+1,0x55);
      ezio.writeDevice(hDev,0x555*2+1,0x80);
      ezio.writeDevice(hDev,0x555*2+1,0xAA);
      ezio.writeDevice(hDev,0x2AA*2+1,0x55);
      ezio.writeDevice(hDev,((BlockNum+i)<<16)+1,0x30);

      read1=read2+1;
      while(read1!=read2)
	{
	  read1 = ezio.readDevice(hDev,((BlockNum+i)<<16));
	  read2 = ezio.readDevice(hDev,((BlockNum+i)<<16));
	}
      read1=read2+1;
      while(read1!=read2)
	{
	  read1 = ezio.readDevice(hDev,((BlockNum+i)<<16)+1);
	  read2 = ezio.readDevice(hDev,((BlockNum+i)<<16)+1);
	}
    }
}
