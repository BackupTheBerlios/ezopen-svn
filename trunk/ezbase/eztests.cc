#include <iostream>
#include "ezopen_ezf2.h"
using namespace std;

struct gba_header
{
  u_int32_t start_address;  /* 32-bit int */
  char logo [ 156 ];        /* nintendo logo */
  char title [ 12 ];        /* cartridge title */
  char code [ 4 ];          /* game code */
  char maker [ 2 ];         /* maker's code */
  u_int8_t reserved;        /* reserved */
  u_int8_t unit_code;       /* main unit code, 00h for GBA */
  u_int8_t device_type;     /* device type */
  char reserved1 [ 3 ];     /* reserved */
  u_int16_t rom_size;       /* size of rom * 32k */
  u_int8_t saver_size;      /* size of rom's saver * 32k */
  char reserved2 [ 2 ];     /* reserved */
  u_int8_t complement;      /* complement check */
  u_int8_t menu_tag;        /* 0xCE for EZLoader use */
  char reserved3;           /* reserved */
};

int main( int argc, char *argv [] )
{
  ez_handle_t h = NULL;
  ezopen_ezf2 dev;

  // open, claim, close
  cout << "-=--=--=--=--=--=--=--=--=--=--=-" << endl;
  cout << " open - claim - close\n";

  dev.cartOpen(h) ? cout << "  * Device opened\n" : cout << "  ! Couldn't open device.\n";
  dev.cartClose(h) ? cout << "  * Device closed\n" : cout << "  ! Couldn't close device.\n";

  // open, claim, read, close
  cout << "-=--=--=--=--=--=--=--=--=--=--=-" << endl;
  cout << " open - claim - read from rom - close\n";

  struct gba_header* gbah = (struct gba_header*) calloc(1, sizeof(struct gba_header));
  dev.cartOpen(h) ? cout << "  * Device opened\n" : cout << "  ! Couldn't open device.\n";
  dev.readROM(h, 0, sizeof(struct gba_header), (ez_byte_t*) gbah);
  cout << "  * First ROM is: " << gbah->title << endl;
  dev.cartClose(h) ? cout << "  * Device closed\n" : cout << "  ! Couldn't close device.\n";
  cout << "-=--=--=--=--=--=--=--=--=--=--=-" << endl;
  

  return 0;
}
