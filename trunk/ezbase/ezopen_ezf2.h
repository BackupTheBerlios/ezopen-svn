/*
  Convenience wrapper for the EZ-Writer delivered with
  the EZ-Flash II Powerstar GBA-cart.

  <reborn@users.berlios.de>
  <bagu@users.berlios.de> 
*/
#include "ezopen_io.h"


class ezopen_ezf2
{
 private:
  ezopen_io ezio;
 public:
  bool cartOpen ( ez_handle_t &hDev );
  void cartErase (ez_handle_t &hDev, ez_dword_t BlockNum);
  bool cartClose ( ez_handle_t &hDev );

  bool prepRAMOp ( ez_handle_t &hDev, u_int32_t page );
  bool closeRAMOp ( ez_handle_t &hDev );
  bool readRAM ( ez_handle_t &hDev, u_int32_t offset, ez_byte_t* buf, u_int32_t bs );
  bool writeRAM ( ez_handle_t &hDev, u_int32_t offset, ez_byte_t* buf, u_int32_t bs );

  bool prepROMOp ( ez_handle_t &hDev, u_int32_t page );
  bool closeROMOp ( ez_handle_t &hDev );
  bool readROM ( ez_handle_t &hDev, u_int32_t offset, u_int32_t length, ez_byte_t* buf );
  bool writeROM ( ez_handle_t &hDev, u_int32_t offset, ez_byte_t* buf, u_int32_t bs );
};

