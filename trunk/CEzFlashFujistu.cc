#include "CEzFlashFujistu.h"

#include <usb.h>

void CEzFlashFujistu::SetReadArray ( HANDLE &hDev )
{
  // there are two choices - Aladdin provided the first block, while reverse
  // engineering the Windows USB calls suggests the second block ... either
  // seem to work.
/*  
	WriteDevice ( hDev, 0x555 * 2, 0xAA );
	WriteDevice ( hDev, 0x555 * 2 + 1, 0xAA );
	WriteDevice ( hDev, 0x2AA * 2, 0x55 );
	WriteDevice ( hDev, 0x2AA * 2 + 1, 0x55 );
	WriteDevice ( hDev, 0x555 * 2, 0xF0 );
	WriteDevice ( hDev, 0x555 * 2 + 1, 0xF0 );
*/

	WriteDevice ( hDev, 0x555 * 2, 0xAA );
	WriteDevice ( hDev, 0x2AA * 2, 0x55 );
	WriteDevice ( hDev, 0x555 * 2, 0xF0 );
	WriteDevice ( hDev, 0x555 * 2 + 1, 0xAA );
	WriteDevice ( hDev, 0x2AA * 2 + 1, 0x55 );
	WriteDevice ( hDev, 0x555 * 2 + 1, 0xF0 );

}

void CEzFlashFujistu::CartWrite ( HANDLE &hDev, DWORD StartAddr, BYTE* pbuf,
                                  unsigned long ByteCount )
{
	while ( CartReadStatus ( hDev ) != 0x8080 );

	ctrlbuf [ 0 ] = ROM_Write;
	*(DWORD *)&ctrlbuf [ 1 ] = StartAddr;
	ctrlbuf [ 4 ] = ROM_Write_fujistu;
	//Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,3,(char*)ctrlbuf,5);
	Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,4,(char*)ctrlbuf,5);
  
  //Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,4,(char*)pbuf,ByteCount);
	Bulk(hDev,IOCTL_EZUSB_BULK_WRITE,2,(char*)pbuf,ByteCount);
}

void CEzFlashFujistu::CartWriteEx(HANDLE &hDev, DWORD StartAddr, BYTE* pbuf, unsigned long ByteCount)
{
	DWORD realaddress = 0;
	realaddress = (StartAddr>>1)&0xFFFFFF;
	CartWrite(hDev,realaddress,pbuf,ByteCount);
}


WORD CEzFlashFujistu::CartReadStatus ( HANDLE &hDev )
{
	return 0x8080;
}

void CEzFlashFujistu::CartErase(HANDLE &hDev, DWORD BlockNum)
{
	WORD read1=1,read2=2;
	BlockNum = BlockNum&0xFF ;
	for(int i=0;i<2;i++)
	{
		WriteDevice(hDev,0x555*2,0xAA) ;
		WriteDevice(hDev,0x2AA*2,0x55) ;
		WriteDevice(hDev,0x555*2,0x80) ;
		WriteDevice(hDev,0x555*2,0xAA) ;
		WriteDevice(hDev,0x2AA*2,0x55) ;
		WriteDevice(hDev,((BlockNum+i)<<16),0x30) ;

		WriteDevice(hDev,0x555*2+1,0xAA) ;
		WriteDevice(hDev,0x2AA*2+1,0x55) ;
		WriteDevice(hDev,0x555*2+1,0x80) ;
		WriteDevice(hDev,0x555*2+1,0xAA) ;
		WriteDevice(hDev,0x2AA*2+1,0x55) ;
		WriteDevice(hDev,((BlockNum+i)<<16)+1,0x30) ;

		read1=read2+1;
		while(read1!=read2)
		{
			read1 = ReadDevice(hDev,((BlockNum+i)<<16));
			read2 = ReadDevice(hDev,((BlockNum+i)<<16));
		}
		read1=read2+1;
		while(read1!=read2)
		{
			read1 = ReadDevice(hDev,((BlockNum+i)<<16)+1);
			read2 = ReadDevice(hDev,((BlockNum+i)<<16)+1);
		}
	}
}

void CEzFlashFujistu::CartEraseEx(HANDLE &hDev, DWORD Address)
{
	DWORD  block = 0 ;
	block = (((Address>>1)&0xFFFFFF)>>17)<<1 ;
	CartErase(hDev,block);
}

DWORD CEzFlashFujistu::CartReadID(HANDLE &hDev)
{
	BYTE id1,id2,id3,id4;
	WriteDevice(hDev,0x555*2,0xAA) ;
	WriteDevice(hDev,0x2AA*2,0x55) ;
	WriteDevice(hDev,0x555*2,0x90) ;
	id1 = (BYTE)ReadDevice(hDev,0x1C);
	id3 = (BYTE)ReadDevice(hDev,0x1E);

	WriteDevice(hDev,0x555*2+1,0xAA) ;
	WriteDevice(hDev,0x2AA*2+1,0x55) ;
	WriteDevice(hDev,0x555*2+1,0x90) ;
	id2 = (BYTE)ReadDevice(hDev,0x1C + 1);
	id4 = (BYTE)ReadDevice(hDev,0x1E + 1);

	return (id4<<24)|(id3<<16)|(id2<<8)|(id1) ;
}

void CEzFlashFujistu::CartLock(HANDLE &hDev)
{
	return ;
}

void CEzFlashFujistu::CartUnlock(HANDLE &hDev)
{
	return ;
}

void CEzFlashFujistu::CartClearStatus(HANDLE &hDev)
{
	return ;
}
