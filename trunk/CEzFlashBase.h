#ifndef _CEzFlashBase_H_
#define _CEzFlashBase_H_

#define ROM_Read          1
#define ROM_Write         2
#define RAM_Read          3
#define RAM_Write         4
#define Open_Port         5
#define Close_Port        6
#define Open_Flash_Op     7
#define Close_Flash_Op    8
#define Set_SRAM_Page     9
#define Set_Flash_Offset  10
#define Write_Operation   25
#define Read_Operation    26
#define Open_Read_Op      27
#define Close_Read_Op     28
#define ROM_Write_fujistu  102

#define EZ_TIMEOUT        500
#define EZ_VENDOR         0x0548
#define EZ_PRODID         0x1005
#define EZ_BLOCKSIZE      0x8000 /* 32k! */

#define FILE_DEVICE_UNKNOWN             0x00000022
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define FILE_ANY_ACCESS                 0

#define CTL_CODE( DeviceType, Function, Method, Access ) ( \
    (DWORD)((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | \
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

#include <usb.h>

/* this just makes things a little easier for me */
#define LPVOID  char*
#define DWORD   unsigned long
#define WORD    unsigned short
#define BYTE    unsigned char
#define HANDLE  usb_dev_handle*
#define UINT    unsigned int

//typedef void*           LPVOID;
//typedef unsigned long   DWORD;
//typedef unsigned short  WORD;
//typedef unsigned char   BYTE;
//typedef usb_dev_handle* HANDLE;
//typedef unsigned int    UINT;

class CEzFlashBase
{
protected:
  BYTE  ctrlbuf [ 64 ];
  BYTE  StateBuf [ 64 ];

public:
  DWORD Bulk ( HANDLE hDev, unsigned long ControlCode, unsigned long pipeNum,
               LPVOID buf, unsigned long bytecount );
  void CartRead ( HANDLE hDev, DWORD StartAddr, BYTE* pbuf,
                  unsigned long ByteCount );
  void CartReadEx ( HANDLE hDev, DWORD StartAddr, BYTE* pbuf,
                    unsigned long ByteCount );
  void CartRAMRead ( HANDLE hDev, WORD StartAddr, BYTE* pbuf,
                     unsigned long ByteCount );
  void CartRAMWrite ( HANDLE hDev, WORD StartAddr, BYTE* pbuf,
                      unsigned long ByteCount );
  void CartRAMWriteEx ( HANDLE hDev, WORD StartAddr, BYTE* pbuf,
                      unsigned long ByteCount );
  DWORD WriteDevice ( HANDLE &hDev, UINT address, WORD data );
  WORD ReadDevice ( HANDLE &hDev, UINT address );
  void CartOpenFlashOP ( HANDLE hDev );
  void CartCloseFlashOP ( HANDLE hDev );
  void CartSetRAMPage ( HANDLE hDev, DWORD Offset );
  void CartSetROMPage ( HANDLE hDev, DWORD Offset );
  bool OpenCartDevice ( HANDLE &hDev );
  bool CloseCartDevice ( HANDLE &hDev );
  void CartOpenPort ( HANDLE hDev );
  void CartClosePort ( HANDLE hDev );
  void CartOpenReadOP ( HANDLE hDev );
  void CartCloseReadOP ( HANDLE hDev );

  void SetReadArray ( HANDLE hDev ) { };
  void CartWrite ( HANDLE hDev, DWORD StartAddr, BYTE* pbuf,
                   unsigned long ByteCount) { };
  void CartWriteEx ( HANDLE hDev, DWORD StartAddr, BYTE* pbuf,
                     unsigned long ByteCount) { };
  WORD CartReadStatus ( HANDLE hDev ) { };
  void CartErase ( HANDLE hDev, DWORD BlockNum ) { };
  void CartEraseEx ( HANDLE hDev, DWORD Address ) { };
  DWORD CartReadID ( HANDLE hDev ) { };
  void CartLock ( HANDLE hDev ) { };
  void CartUnlock ( HANDLE hDev ) { };
  void CartClearStatus ( HANDLE hDev ) { };
};

#endif
