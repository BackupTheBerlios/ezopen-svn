#ifndef _CEzFlashFujistu_H_
#define _CEzFlashFujistu_H_

#include "CEzFlashBase.h"

class CEzFlashFujistu : public CEzFlashBase
{
public:
  void SetReadArray ( HANDLE hDev );
  void CartWrite ( HANDLE hDev, DWORD StartAddr, BYTE* pbuf,
                   unsigned long ByteCount);
  void CartWriteEx ( HANDLE hDev, DWORD StartAddr, BYTE* pbuf,
                     unsigned long ByteCount);
  WORD CartReadStatus ( HANDLE hDev );
  void CartErase ( HANDLE hDev, DWORD BlockNum );
  void CartEraseEx ( HANDLE hDev, DWORD Address );
  DWORD CartReadID ( HANDLE hDev );
  void CartLock ( HANDLE hDev );
  void CartUnlock ( HANDLE hDev );
  void CartClearStatus ( HANDLE hDev );
};

#endif
