/*
  Basic USB IO for the EZ-Writer delivered with
  the EZ-Flash II Powerstar GBA-cart.

  <reborn@users.berlios.de>
  <bagu@users.berlios.de> 
*/
#include "ezopen_io.h"

ez_dword_t ezopen_io::bulk ( ez_handle_t &hDev, unsigned long ControlCode, unsigned long pipeNum,
		 ez_lpvoid_t buf, unsigned long ByteCount )
{
  int r = -1;

  if ( ControlCode == IOCTL_EZUSB_BULK_READ )
    r = usb_bulk_read ( hDev, pipeNum, buf, ByteCount, EZ_TIMEOUT );
  else if ( ControlCode == IOCTL_EZUSB_BULK_WRITE )
    r += usb_bulk_write ( hDev, pipeNum, buf, ByteCount, EZ_TIMEOUT );

  return r;
}

bool ezopen_io::prepFlashOp ( ez_handle_t &hDev )
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

bool ezopen_io::closeFlashOp ( ez_handle_t &hDev )
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


// RAM ops
bool ezopen_io::prepRAMOp ( ez_handle_t &hDev, u_int32_t page )
{

  if ( ! prepFlashOp ( hDev ) )
    return false;

  int r = -1;

  // set RAM page
  r = writeDevice ( hDev, 0xff0000, 0xd2FF );
  r += writeDevice ( hDev, 0x000000, 0x15FF );
  r += writeDevice ( hDev, 0x010000, 0xd2FF );
  r += writeDevice ( hDev, 0x020000, 0x15FF );
  r += writeDevice ( hDev, 0xe00000, (ez_word_t) page );
  r += writeDevice ( hDev, 0xfe0000, 0x15FF );

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

bool ezopen_io::closeRAMOp ( ez_handle_t &hDev )
{
  return closeFlashOp ( hDev );
}

// ROM ops
bool ezopen_io::prepROMOp ( ez_handle_t &hDev, u_int32_t page )
{
  int r = -1;

  //   CartSetROMPage ( h, 0 );
  r = writeDevice ( hDev, 0xff0000, 0xd2FF );
  r += writeDevice ( hDev, 0x000000, 0x15FF );
  r += writeDevice ( hDev, 0x010000, 0xd2FF );
  r += writeDevice ( hDev, 0x020000, 0x15FF );
  r += writeDevice ( hDev, 0xc40000, ( ez_word_t ) page );
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
  r += bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, ( ez_lpvoid_t ) ctrlbuf, 1 );

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

bool ezopen_io::closeROMOp ( ez_handle_t &hDev )
{
  int r = -1;

  // CartCloseReadOP ( h );
  ctrlbuf [ 0 ] = Close_Read_Op;
  r = bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, ( ez_lpvoid_t ) ctrlbuf, 1 );

  // CartClosePort ( h );
  ctrlbuf [ 0 ] = Close_Port;
  r += bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, ( char* ) ctrlbuf, 1 );

  if ( r < 0 )
    return true;
  
  return false;
}

bool ezopen_io::cartRead ( ez_handle_t &hDev, ez_dword_t offset, ez_byte_t* buf, unsigned long bs )
{
  int r = -1;

  ctrlbuf[0] = ROM_Read;
  * ( ez_dword_t * ) &ctrlbuf[1] = offset;
  r = bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, (char*) ctrlbuf, 4 );
  r += bulk ( hDev, IOCTL_EZUSB_BULK_READ, 2, (char*) buf, bs );

  if ( r < 0)
    return true;
  
  return false;
}

bool ezopen_io::cartWrite ( ez_handle_t &hDev, ez_dword_t offset, ez_byte_t* buf, unsigned long bs )
{
  int r = -1;
  //while ( CartReadStatus ( hDev ) != 0x8080 );
  
  ctrlbuf [ 0 ] = ROM_Write;
  *(ez_dword_t *) &ctrlbuf [ 1 ] = offset;
  ctrlbuf [ 4 ] = ROM_Write_fujistu;
  r = bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, (char*) ctrlbuf, 5 );
  r += bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 2, (char*) buf, bs );

  if ( r < 0 )
    return true;
  
  return false;
}
