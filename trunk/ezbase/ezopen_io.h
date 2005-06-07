/*
  Basic USB IO for the EZ-Writer delivered with
  the EZ-Flash II Powerstar GBA-cart.

  <reborn@users.berlios.de>
  <bagu@users.berlios.de> 
*/
#include <usb.h>


#define EZ_TIMEOUT        500
#define EZ_BLOCKSIZE      0x8000 /* 32k! */

#define FILE_DEVICE_UNKNOWN             0x00000022
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define FILE_ANY_ACCESS                 0

#define CTL_CODE( DeviceType, Function, Method, Access ) ( \
    (unsigned long)((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | \
    (Method) \
    )

#define Ezusb_IOCTL_INDEX 0x0800

#define IOCTL_EZUSB_BULK_READ CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                       Ezusb_IOCTL_INDEX+19, \
                                       METHOD_OUT_DIRECT, \
                                       FILE_ANY_ACCESS)

#define IOCTL_EZUSB_BULK_WRITE  CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                         Ezusb_IOCTL_INDEX+20, \
                                         METHOD_IN_DIRECT, \
                                         FILE_ANY_ACCESS)


typedef char*           ez_lpvoid_t;
typedef unsigned long   ez_dword_t;
typedef unsigned short  ez_word_t;
typedef unsigned char   ez_byte_t;
typedef usb_dev_handle* ez_handle_t;
typedef unsigned int    ez_uint_t;

const int Ez_vendor = 0x0548;
const int Ez_prodid = 0x1005;

// hardware modes?
const unsigned char ROM_Read          = 1;
const unsigned char ROM_Write         = 2;
const unsigned char RAM_Read          = 3;
const unsigned char RAM_Write         = 4;
const unsigned char Open_Port         = 5;
const unsigned char Close_Port        = 6;
const unsigned char Open_Flash_Op     = 7;
const unsigned char Close_Flash_Op    = 8;
const unsigned char Set_SRAM_Page     = 9;
const unsigned char Set_Flash_Offset  = 10;
const unsigned char Write_Operation   = 25;
const unsigned char Read_Operation    = 26;
const unsigned char Open_Read_Op      = 27;
const unsigned char Close_Read_Op     = 28;
const unsigned char ROM_Write_fujistu = 102;

class ezopen_io
{
protected:
public:
  ez_byte_t  ctrlbuf [ 64 ];
  ez_byte_t  statebuf [ 64 ];

  ez_dword_t bulk ( ez_handle_t &hDev, unsigned long ControlCode, unsigned long pipeNum,
		 ez_lpvoid_t buf, unsigned long ByteCount );
  bool prepFlashOp ( ez_handle_t &hDev );
  bool closeFlashOp ( ez_handle_t &hDev );
  bool prepRAMOp ( ez_handle_t &hDev, u_int32_t page );
  bool closeRAMOp ( ez_handle_t &hDev );
  bool prepROMOp ( ez_handle_t &hDev, u_int32_t page );
  bool closeROMOp ( ez_handle_t &hDev );
  ez_dword_t writeDevice ( ez_handle_t &hDev, ez_uint_t address, ez_word_t data );
  bool cartRead ( ez_handle_t &hDev, ez_dword_t offset, ez_byte_t* buf, unsigned long bs );
  bool cartWrite ( ez_handle_t &hDev, ez_dword_t offset, ez_byte_t* buf, unsigned long bs );
};

