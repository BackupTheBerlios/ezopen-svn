/*
  Basic USB IO for the EZ-Writer delivered with
  the EZ-Flash II Powerstar GBA-cart.

  <reborn@users.berlios.de>
  <bagu@users.berlios.de> 
*/
#include "EzopenBaseIO.h"

dword_t EzopenBaseIO::bulk ( handle_t &hDev, unsigned long ControlCode, unsigned long pipeNum,
		 lpvoid_t buf, unsigned long ByteCount )
{
  int r = -1;

  if ( ControlCode == IOCTL_EZUSB_BULK_READ )
    r = usb_bulk_read ( hDev, pipeNum, buf, ByteCount, EZ_TIMEOUT );
  else if ( ControlCode == IOCTL_EZUSB_BULK_WRITE )
    r += usb_bulk_write ( hDev, pipeNum, buf, ByteCount, EZ_TIMEOUT );

  return r;
}

dword_t EzopenBaseIO::writeDevice ( handle_t &hDev, uint_t address, word_t data )
{
  ctrlbuf[0] = Write_Operation;
  ctrlbuf[1] = (address)&0xFF;
  ctrlbuf[2] = (address>>8)&0xFF;
  ctrlbuf[3] = (address>>16)&0xFF;
  *(word_t *)&ctrlbuf[4] = data;
  
  return bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, (char*)ctrlbuf, 6 );
}

bool EzopenBaseIO::prepFlashOp ( handle_t &hDev )
{
  int r = -1;

  r = writeDevice ( hDev, 0xff0000, 0xd2FF );
  r += writeDevice ( hDev, 0x000000, 0x15FF );
  r += writeDevice ( hDev, 0x010000, 0xd2FF );
  r += writeDevice ( hDev, 0x020000, 0x15FF );
  r += writeDevice ( hDev, 0xe20000, 0x15FF );
  r += writeDevice ( hDev, 0xfe0000, 0x15FF );

  if ( r < 0 )
    return false;

  return true;
}

bool EzopenBaseIO::closeFlashOp ( handle_t &hDev )
{
  int r = -1;

  r = writeDevice(hDev,0xff0000,0xd2FF);
  r += writeDevice(hDev,0x000000,0x15FF);
  r += writeDevice(hDev,0x010000,0xd2FF);
  r += writeDevice(hDev,0x020000,0x15FF);
  r += writeDevice(hDev,0xe20000,0xd2FF);
  r += writeDevice(hDev,0xfe0000,0x15FF);

  if ( r < 0 )
    return false;

  return true;
}


bool EzopenBaseIO::cartOpen ( handle_t &hDev )
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

bool EzopenBaseIO::cartClose ( handle_t &hDev )
{
  if ( usb_release_interface ( hDev, 0 ) < 0 )
    return false;

  if ( usb_close ( hDev ) < 0 )
    return false;

  return true;
}

// RAM ops
bool EzopenBaseIO::prepRAMOp ( handle_t &hDev, u_int32_t page )
{

  if ( ! prepFlashOp ( hDev ) )
    return false;

  int r = -1;

  // set RAM page
  r = writeDevice(hDev,0xff0000,0xd2FF);
  r += writeDevice(hDev,0x000000,0x15FF);
  r += writeDevice(hDev,0x010000,0xd2FF);
  r += writeDevice(hDev,0x020000,0x15FF);
  r += writeDevice(hDev,0xe00000,(word_t) page);
  r += writeDevice(hDev,0xfe0000,0x15FF);

  if ( r < 0 )
    return false;


  // set read array (from the fujitsu-class)
  r += writeDevice ( hDev, 0x555 * 2, 0xAA );
  r += writeDevice ( hDev, 0x2AA * 2, 0x55 );
  r += writeDevice ( hDev, 0x555 * 2, 0xF0 );
  r += writeDevice ( hDev, 0x555 * 2 + 1, 0xAA );
  r += writeDevice ( hDev, 0x2AA * 2 + 1, 0x55 );
  r += writeDevice ( hDev, 0x555 * 2 + 1, 0xF0 );

  if ( r < 0 )
    return false;

  return true;
}

bool EzopenBaseIO::readRAM ( handle_t &hDev, u_int32_t offset, byte_t* buf, u_int32_t bytecount )
{
  int r = -1;

  ctrlbuf[0] = RAM_Read;
  *(word_t *)&ctrlbuf[1] = offset;
  r = bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, (char*) ctrlbuf, 3 );
  r += bulk ( hDev, IOCTL_EZUSB_BULK_READ, 2, (char*) buf, bytecount);

  if ( r < 0 )
    return false;

  return true;
}

bool EzopenBaseIO::closeRAMOp ( handle_t &hDev )
{
  return closeFlashOp ( hDev );
}

bool EzopenBaseIO::writeRAM ( handle_t &hDev, u_int32_t offset, byte_t* buf, u_int32_t bs )
{
  int r = -1;

  ctrlbuf[0] = RAM_Write;
  *(word_t *) &ctrlbuf[1] = offset;
  r = bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, (char*) ctrlbuf, 3 );
  r += bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 2, (char*) buf, bs );

  if ( r < 0 )
    return false;

  return true;

}


// ROM ops
bool EzopenBaseIO::prepROMOp ( handle_t &hDev, u_int32_t page )
{
  int r = -1;

  //   CartSetROMPage ( h, 0 );
  r = writeDevice ( hDev, 0xff0000, 0xd2FF );
  r += writeDevice ( hDev, 0x000000, 0x15FF );
  r += writeDevice ( hDev, 0x010000, 0xd2FF );
  r += writeDevice ( hDev, 0x020000, 0x15FF );
  r += writeDevice ( hDev, 0xc40000, ( word_t ) page );
  r += writeDevice ( hDev, 0xfe0000, 0x15FF );

  if ( r < 0 )
    return false;
	
  //   CartOpenPort ( h );
  ctrlbuf [ 0 ] = Open_Port;
  r += bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, ( char* ) ctrlbuf, 1 );

  if ( r < 0 )
    return false;

  //   CartOpenReadOP ( h );
  ctrlbuf [ 0 ] = Open_Read_Op;
  r += bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, ( lpvoid_t ) ctrlbuf, 1 );

  if ( r < 0 )
    return false;

  //   SetReadArray ( h ) (from the fujitsu-class)
  r += writeDevice ( hDev, 0x555 * 2, 0xAA );
  r += writeDevice ( hDev, 0x2AA * 2, 0x55 );
  r += writeDevice ( hDev, 0x555 * 2, 0xF0 );
  r += writeDevice ( hDev, 0x555 * 2 + 1, 0xAA );
  r += writeDevice ( hDev, 0x2AA * 2 + 1, 0x55 );
  r += writeDevice ( hDev, 0x555 * 2 + 1, 0xF0 );

  if ( r < 0 )
    return false;

  return true;
}

bool EzopenBaseIO::closeROMOp ( handle_t &hDev )
{
  // CartCloseReadOP ( h );
  ctrlbuf [ 0 ] = Close_Read_Op;
  bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, ( lpvoid_t ) ctrlbuf, 1 );

  // CartClosePort ( h );
  ctrlbuf [ 0 ] = Close_Port;
  bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, ( char* ) ctrlbuf, 1 );

  return true;
}

bool EzopenBaseIO::cartRead ( handle_t &hDev, dword_t offset, byte_t* buf, unsigned long bs )
{
  ctrlbuf[0] = ROM_Read;
  * ( dword_t * ) &ctrlbuf[1] = offset;
  bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, (char*) ctrlbuf, 4 );
  bulk ( hDev, IOCTL_EZUSB_BULK_READ, 2, (char*) buf, bs );

  return true;
}

bool EzopenBaseIO::cartWrite ( handle_t &hDev, dword_t offset, byte_t* buf, unsigned long bs )
{
  //while ( CartReadStatus ( hDev ) != 0x8080 );
  
  ctrlbuf [ 0 ] = ROM_Write;
  *(dword_t *)&ctrlbuf [ 1 ] = offset;
  ctrlbuf [ 4 ] = ROM_Write_fujistu;
  bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, (char*) ctrlbuf, 5 );
  bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 2, (char*) buf, bs );

  return true;
}

bool EzopenBaseIO::readROM ( handle_t &hDev, u_int32_t offset, u_int32_t length, byte_t* buf ) 
{
  // "(offset >> 1) & 0xFFFFFF" becomes the address where to start
  cartRead ( hDev, (offset >> 1) & 0xFFFFFF, buf, length );

  return true;
}

bool EzopenBaseIO::writeROM ( handle_t &hDev, u_int32_t offset, byte_t* buf, u_int32_t bs )
{
  cartWrite ( hDev, (offset >> 1) & 0xFFFFFF, buf, bs );
  
  return true;
}
