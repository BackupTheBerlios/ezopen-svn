#include <iostream>
#include <stdio.h>
#include <iomanip>

#include <sys/types.h>

#include "CEzFlashFujistu.h"

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
  char reserved1 [ 3 ];
  u_int16_t rom_size;       /* size of rom * 32k */
  u_int8_t saver_size;      /* size of rom's saver * 32k */
  char reserved2 [ 2 ];     /* reserved */
  u_int8_t complement;      /* complement check */
  u_int8_t menu_tag;        /* 0xCE for EZLoader use */
  char reserved3;           /* reserved */
};

BYTE* read_cart ( CEzFlashBase t, HANDLE h, int offset, int length )
{
  BYTE* buf = ( BYTE* ) calloc ( 1, 192 );

  t.CartSetROMPage ( h, 0 );
  
  t.CartOpenPort ( h );
  t.CartOpenFlashOP ( h );

  t.SetReadArray ( h );

  t.CartRead ( h, offset, buf, 192 );

  t.CartCloseFlashOP ( h );
  t.CartClosePort ( h );

  return buf;
}

int main ( int argc, char *argv [] )
{
  HANDLE h;
  
  CEzFlashFujistu t;

  if ( t.OpenCartDevice ( h ) == false )
  {
    cout << "Ack!" << endl;
    exit ( -1 );
  }
  
  unsigned int size = t.CartReadID ( h );

  cout << "Cart ID = " << size << endl;

  if ( argc > 2 )
  {
    if ( ! strcmp ( argv [ 1 ], "write" ) )
    {
      // did they specify a start offset?
      int offset = 0;
      
      if ( argc > 3 )
      {
        offset = atoi ( argv [ 3 ] );
        cout << "Setting write offset to " << offset << endl;
      }
      
      FILE *f = fopen ( argv [ 2 ], "r" );

      // make sure we can open the file ...
      if ( f != NULL )
      {
        // find the full length of the file
        fseek ( f, 0, SEEK_END );
        unsigned long length = ftell ( f );

        // back we go ...
        rewind ( f );

        // blocks are 32k!
        int bsize = 0x8000;

        // number of blocks to use
        int bcount = ( length / bsize ) + ( length % bsize > 0 ? 1 : 0 );

        cout << "Writing \"" << argv [ 2 ] << "\" to cart ... "
             << " (cart is " << length << " bytes) "
             << endl << "Erasing    " << flush;

        t.CartOpenFlashOP ( h );

        for ( int l = ( offset / 2 ); l < ( bcount / 2 ); ++l )
        {
          cout << "\b\b\b" << setw ( 3 ) << l << flush;
          t.CartErase ( h, l );
        }

        cout << "\b\bis complete!" << endl;

        cout << "SetReadArray ... " << flush;

        t.SetReadArray ( h );

        cout << "Done." << endl;

        // quarter megabit chunks
        cout << "Writing " << bcount
             << " blocks (in " << bsize << " byte chunks) ... --%" << flush;

        BYTE buf [ bsize ];
        unsigned long p = 0;
        float percent = 0;
        
        offset *= bsize;

        while ( p < length )
        {
          percent = ( ( float ) p / ( float ) length ) * 100.0f;

          cout << "\b\b\b" << setw ( 2 ) << ( int ) percent << "%" << flush;

          fread ( buf, bsize, 1, f );

          t.CartWriteEx ( h, offset + p, buf, bsize );

          p += bsize;
        }

        t.CartCloseFlashOP ( h );

        cout << "\b\b\bDone!" << endl;
        fclose ( f );
      }
      else
      {
        cout << "Couldn't open \"" << argv [ 2 ] << "\"" << endl;
      }
    }
    else if ( ! strcmp ( argv [ 1 ], "writeram" ) )
    {
      // did they specify a start offset?
      int offset = 0;
      
      if ( argc > 3 )
      {
        offset = atoi ( argv [ 3 ] );
        cout << "Setting write offset to " << offset << endl;
      }
      
      FILE *f = fopen ( argv [ 2 ], "r" );

      // make sure we can open the file ...
      if ( f != NULL )
      {
        // find the full length of the file
        fseek ( f, 0, SEEK_END );
        unsigned long length = ftell ( f );

        // back we go ...
        rewind ( f );

        // blocks are 32k!
        int bsize = 0x8000;

        // number of blocks to use
        int bcount = ( length / bsize ) + ( length % bsize > 0 ? 1 : 0 );

        cout << "Writing \"" << argv [ 2 ] << "\" to cart." << endl << flush;

        //t.CartOpenFlashOP ( h );
        t.CartOpenPort ( h );

        cout << "SetReadArray ... " << flush;

        t.SetReadArray ( h );

        cout << "Done." << endl;

        // quarter megabit chunks
        cout << "Writing " << bcount
             << " blocks (in " << bsize << " byte chunks) ... --%" << flush;

        BYTE buf [ bsize ];
        unsigned long p = 0;
        float percent = 0;
        
        offset *= bsize;

        while ( p < length )
        {
          percent = ( ( float ) p / ( float ) length ) * 100.0f;

          cout << "\b\b\b" << setw ( 2 ) << ( int ) percent << "%" << flush;

          fread ( buf, bsize, 1, f );

          t.CartRAMWriteEx ( h, offset + p, buf, bsize );

          p += bsize;
        }

        //t.CartCloseFlashOP ( h );
        t.CartClosePort ( h );

        cout << "\b\b\bDone!" << endl;
        fclose ( f );
      }
      else
      {
        cout << "Couldn't open \"" << argv [ 2 ] << "\"" << endl;
      }
    }
    else if ( ! strcmp ( argv [ 1 ], "readram" ) )
    {
      // did they specify a start offset?
      int offset = 0;
      int length = 0x8000;
      
      if ( argc > 3 )
      {
        length = atoi ( argv [ 3 ] );
        cout << "Reading " << length << " bytes ..." << endl;

        if ( argc > 4 )
        {
          offset = atoi ( argv [ 4 ] );
          cout << "Setting read offset to " << offset << endl;
        }
      }
      
      cout << "Reading " << length << " bytes (@ offset " << offset
           << ") into file " << argv [ 2 ] << endl;
      FILE *f = fopen ( argv [ 2 ], "w" );

      // make sure we can open the file ...
      if ( f != NULL )
      {
        //t.CartOpenFlashOP ( h );
        t.CartOpenPort ( h );

        cout << "SetReadArray ... " << flush;

        t.SetReadArray ( h );

        cout << "Done." << endl;

        BYTE* buf = ( BYTE* ) calloc ( 1, length );
        t.CartRAMRead ( h, offset, buf, length );

        fwrite ( buf, 1, length, f );

        //t.CartCloseFlashOP ( h );
        t.CartClosePort ( h );

        cout << "\b\b\bDone!" << endl;
        fclose ( f );
      }
      else
      {
        cout << "Couldn't open \"" << argv [ 2 ] << "\"" << endl;
      }
    }
    else if ( ! strcmp ( argv [ 1 ], "read" ) )
    {
      FILE *f = fopen ( argv [ 2 ], "w" );

      if ( f != NULL )
      {
        // default to 1mbit
        int read = 0x8000 * 4;

        if ( argc > 3 )
          read = atoi ( argv [ 3 ] );

        cout << "Reading " << read << " bytes ... " << flush;
        
        BYTE* buf = read_cart ( t, h, 0, read );
        
        fwrite ( buf, read, 1, f );
        
        free ( buf );
        
        fclose ( f );
      }
      else
      {
        cout << "Couldn't open \"" << argv [ 2 ] << "\"" << endl;
      }
    }
  }
  else if ( argc > 1 )
  {
    // some options don't require the extra parameter

    // dump the cart's info
    if ( ! strcmp ( "info", argv [ 1 ] ) )
    {
      // start by reading the first 196 bytes, which should contain the first
      // ROM's header
      struct gba_header *gbah = ( struct gba_header* ) read_cart ( t, h, 0, 192 );

      int rom_number = 1;
      
      cout << "ROM info, number " << dec << rom_number << ";" << endl
           << "Entry address: 0x" << hex << gbah->start_address << endl
           << "Title: \"" << gbah->title << "\"" << endl
           << "Game code: " << gbah->code << endl
           << "ROM size: " << dec << ( u_int32_t ) gbah->rom_size << " * 32K" << endl
           << "Saver size: " << dec << ( u_int32_t ) gbah->saver_size << " * 32K" << endl
           << "Compliment byte: 0x" << hex << ( u_int32_t ) gbah->complement << endl
           << "Menu tag: 0x" << hex << ( u_int32_t ) gbah->menu_tag << endl;

      free ( gbah );
    }
  }
  else
  {
    cout << "Usage: " << argv [ 0 ]
         << " [ write | read | info ] [ ... ]" << endl;
  }
  
  t.CartUnlock ( h );
  
  t.CloseCartDevice ( h );
}

// patch_rom re-writes the rom's header so that it can be used with EZLoader,
// rom -> the rom data
// length -> the number of bytes for the rom
void patch_rom ( char* rom, int length )
{
  // the number of 32k blocks this'll take up
  int blocks = ( length / 0x8000 ) + ( length % 0x8000 > 0 ? 1 : 0 );

  // patch rom length
  memset ( rom + 0xB8, ( blocks & 0xff00 ) >> 8, 1 );
  memset ( rom + 0xB8, ( blocks & 0xff ), 1 );
  
  // multiload bytes
  memset ( rom + 0xBA, 0x00, 1 );
  memset ( rom + 0xBE, 0xCE, 1 );

  // fix up the checksum
  char b [ 29 ];
  memcpy ( b, rom + 0xA0, 29 );
  int sum = 18;
  
  for ( int i = 0; i <= 29; ++i )
    sum += b [ i ];

  char t = ( char ) ( ( sum ^ 0xFF ) & 0xFF );
  memset ( rom + 0xBD, t, 1 );
}
