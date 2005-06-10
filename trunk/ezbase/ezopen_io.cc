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

ez_dword_t ezopen_io::writeDevice ( ez_handle_t &hDev, ez_uint_t address, ez_word_t data )
{
  ctrlbuf[0] = Write_Operation;
  ctrlbuf[1] = (address)&0xFF;
  ctrlbuf[2] = (address>>8)&0xFF;
  ctrlbuf[3] = (address>>16)&0xFF;
  *(ez_word_t *)&ctrlbuf[4] = data;
  
  return bulk ( hDev, IOCTL_EZUSB_BULK_WRITE, 4, (char*)ctrlbuf, 6 );
}

ez_word_t ezopen_io::readDevice(ez_handle_t &hDev, ez_uint_t address)
{
  ctrlbuf[0] = Read_Operation;
  ctrlbuf[1] = (address)&0xFF;
  ctrlbuf[2] = (address>>8)&0xFF;
  ctrlbuf[3] = (address>>16)&0xFF;
  //Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,3,(char*)ctrlbuf,4);
  bulk(hDev,IOCTL_EZUSB_BULK_WRITE,4,(char*)ctrlbuf,4);
  //Bulk(hDev,IOCTL_EZUSB_BULK_READ,10,(char*)ctrlbuf,64);
  bulk(hDev,IOCTL_EZUSB_BULK_READ,4,(char*)ctrlbuf,64);
  return(*(ez_word_t *)ctrlbuf);
  
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
