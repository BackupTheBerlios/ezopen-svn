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
