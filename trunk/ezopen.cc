#include <iostream>
#include <fstream>
#include <iomanip>

#include <sys/types.h>

#include "CEzFlashFujistu.h"

#define BLOCK_COUNT_DEFAULT   4
#define BLOCK_SIZE_DEFAULT    0x8000
#define BLOCK_OFFSET_DEFAULT  0

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

// initialise reading of ROM
void
readROMOpen ( CEzFlashBase &t, HANDLE h )
{
  t.CartSetROMPage ( h, 0 );
  t.CartOpenPort ( h );
  t.CartOpenFlashOP ( h );
  t.SetReadArray ( h );
}

// read data from the ROM
BYTE*
readROM ( CEzFlashBase &t, HANDLE h, u_int32_t offset, u_int32_t length )
{
  BYTE* buf = ( BYTE* ) calloc ( 1, length );
  
  t.CartRead ( h, offset, buf, length );

  return buf;
}

// finish reading of ROM
void
readROMClose ( CEzFlashBase &t, HANDLE h )
{
  t.CartCloseFlashOP ( h );
  t.CartClosePort ( h );
}

// initialise writing/erasing of ROM
void
writeROMOpen ( CEzFlashBase &t, HANDLE h )
{
  t.CartOpenFlashOP ( h );
  t.SetReadArray ( h );
}

void
writeROM ( CEzFlashBase &t, HANDLE h, u_int32_t offset, BYTE* buf, u_int32_t bs )
{
  t.CartWriteEx ( h, offset, buf, bs );
}

// close ROM writing
void
writeROMClose ( CEzFlashBase &t, HANDLE h )
{
  t.CartCloseFlashOP ( h );
}

// open ROM erasing
void
eraseROMOpen ( CEzFlashBase &t, HANDLE h )
{
  t.CartOpenFlashOP ( h );
}

void
eraseROM ( CEzFlashBase &t, HANDLE h, u_int32_t block )
{
  t.CartErase ( h, block );
}

// close ROM erasing
void
eraseROMClose ( CEzFlashBase &t, HANDLE h )
{
  t.CartCloseFlashOP ( h );
}

// get cart info
struct gba_header*
romGetInfo ( CEzFlashBase &t, HANDLE h, int offset )
{
  // start by reading the first 192 bytes, which should contain the first
  // ROM's header
  readROMOpen ( t, h );
  
  struct gba_header *gbah =
    ( struct gba_header* ) readROM ( t, h, offset, sizeof ( gba_header ) );

  readROMClose ( t, h );

  return gbah;
}

void
romDisplayInfo ( struct gba_header *gbah )
{
  cout << "Entry address: 0x" << hex << gbah->start_address << endl
       << "Title: \"" << gbah->title << "\"" << endl
       << "Game code: " << gbah->code << endl
       << "Maker: " << gbah->maker << endl
       << "Reserved (1): 0x" << hex << ( u_int32_t ) gbah->reserved << endl
       << "Unit code: 0x" << hex << ( u_int32_t ) gbah->unit_code << endl
       << "Device type: 0x" << hex << ( u_int32_t ) gbah->device_type << endl
       << "Reserved (2): 0x" << hex << ( u_int32_t ) gbah->reserved1 [ 0 ]
                    << " 0x" << hex << ( u_int32_t ) gbah->reserved1 [ 1 ]
                    << " 0x" << hex << ( u_int32_t ) gbah->reserved1 [ 2 ]
                    << endl
       << "ROM size: " << dec << ( u_int32_t ) gbah->rom_size
          << " * 32K" << endl
       << "Saver size: " << dec << ( u_int32_t ) gbah->saver_size
          << " * 32K" << endl
       << "Reserved (3): 0x" << hex << ( u_int32_t ) gbah->reserved2 [ 0 ]
                    << " 0x" << hex << ( u_int32_t ) gbah->reserved2 [ 1 ]
                    << endl
       << "Compliment byte: 0x" << hex
          << ( u_int32_t ) gbah->complement << endl
       << "Menu tag: 0x" << hex << ( u_int32_t ) gbah->menu_tag << endl
       << "Reserved (4): 0x" << hex << ( u_int32_t ) gbah->reserved3 << endl;
}

bool
cartOpen ( CEzFlashBase &t, HANDLE &h )
{
  if ( t.OpenCartDevice ( h ) == false )
    return false;

  //t.CartLock ( h );

  return true;
}

void
cartClose ( CEzFlashBase &t, HANDLE &h )
{
  t.CartUnlock ( h );
  t.CloseCartDevice ( h );
}

int
main ( int argc, char *argv [] )
{
  HANDLE h = NULL;
  CEzFlashFujistu t;

  // default block size is 0x8000 = 32768 bytes = 0.25mbit
  u_int32_t block_size = BLOCK_SIZE_DEFAULT;

  // number of blocks to read/write
  u_int32_t block_count = BLOCK_COUNT_DEFAULT;
  
  // block offset
  u_int32_t block_offset = BLOCK_OFFSET_DEFAULT;
 
  // default operation
  string operation = "";
  
  // filename
  string filename = "";
  
  // banner
  cout << "EZOpen EZCart-II PS flasher" << endl
       << "---------------------------" << endl << endl;

  // parse the command line options
  if ( argc > 1 )
  {
    operation = string ( argv [ 1 ] );

    // where to start reading options
    int l = 2;
    
    // parse the other options
    for ( ; l < ( argc - 1 ); ++l )
    {      
      if ( ! strcmp ( argv [ l ], "-b" ) )  // block size
      {
        int t = atoi ( argv [ l + 1 ] );

        if ( t > 0 )
        {
          cout << "Block size set to " << t << endl;
          block_size = t;
          l++;
        }
        else
        {
          cout << "Refusing to set block size to " << t
               << ", leaving at " << block_size << endl;
        }
      }
      else if ( ! strcmp ( argv [ l ], "-o" ) ) // read/write block offset
      {
        int t = atoi ( argv [ l + 1 ] );

        if ( t >= 0 )
        {
          cout << "Block offset set to " << t << endl;
          block_offset = t;
          l++;
        }
        else
        {
          cout << "Refusing to set block offset to " << t
               << ", leaving at " << block_offset << endl;
        }
      }
      else if ( ! strcmp ( argv [ l ], "-c" ) ) // block count
      {
        int t = atoi ( argv [ l + 1 ] );

        if ( t >= 0 )
        {
          cout << "R/W block count set to " << t << endl;
          block_count = t;
          l++;
        }
        else
        {
          cout << "Refusing to set r/w block count to " << t
               << ", leaving at " << block_count << endl;
        }
      }
      else if ( ! strcmp ( argv [ l ], "-f" ) ) // filename
      {
        filename = argv [ ++l ];
      }
    }
  }

  // check the operation
  if ( operation == "read" )
  {
    // try to open the device
    if ( cartOpen ( t, h ) == false )
    {
      cout << "Cannot open device! (Permissions problem?)" << endl;
      return 1;
    }
  }
  else if ( operation == "write" )
  {
    // try to open the device
    if ( cartOpen ( t, h ) == false )
    {
      cout << "Cannot open device! (Permissions problem?)" << endl;
      return 1;
    }

    // open the filename
    if ( filename == "" )
    {
      cout << "No ROM file specified." << endl;
      return 1;
    }
    
    // open the ROM file
    ifstream rom ( filename.c_str () );
    
    // make sure we can open the file ...
    if ( ! rom )
    {
      // couldn't open file :(
      cout << "Could not open ROM file \"" << filename << "\"." << endl;
      return 1;
    }
    else
    {
      // find the full length of the file
      rom.seekg ( 0, ios::end );
      
      u_int32_t length = rom.tellg ();
      
      rom.seekg ( 0, ios::beg );
      
      // calculate the number of blocks to write
      block_count = ( length / block_size ) + ( length % block_size > 0 ? 1 : 0 );

      cout << "Writing \"" << filename << "\" to cart "
           << "(cart is " << block_count << " * " << block_size << " blocks) "
           << endl << "Erasing - --%" << flush;

      // start ROM erasing
      eraseROMOpen ( t, h );

      float percent = 0.0f;
      float pinc = 100.0f / ( float ) ( block_count / 2.0f );

      for ( u_int32_t l = block_offset;
            l < ( block_offset + block_count ); l += 2 )
      {
        cout << "\b\b\b" << setw ( 2 ) << ( u_int32_t ) percent << "%" << flush;
        
        eraseROM ( t, h, l );

        percent += pinc;
      }

      cout << "\b\b\bDone." << endl << flush;
      
      // finish ROM erasing
      eraseROMClose ( t, h );

      // start ROM writing
      writeROMOpen ( t, h );
      
      cout << "Flashing - --%" << flush;

      BYTE buf [ block_size ];
      percent = 0;
      u_int32_t bc = 0;
      
      pinc = 100.0f / ( float ) block_count;
      
      while ( bc < block_count )
      {
        cout << "\b\b\b" << setw ( 2 ) << ( u_int32_t ) percent << "%" << flush;

        rom.read ( ( char* ) buf, block_size );

        writeROM ( t, h, ( bc * block_size ), buf, block_size );

        percent += pinc;
        bc++;
      }

      // finish ROM writing
      writeROMClose ( t, h );

      cout << "\b\b\bDone." << endl;
      
      rom.close ();
    }
    
    cartClose ( t, h );
  }
  else if ( operation == "readram" )
  {
    // try to open the device
    if ( cartOpen ( t, h ) == false )
    {
      cout << "Cannot open device! (Permissions problem?)" << endl;
      return 1;
    }
  }
  else if ( operation == "writeram" )
  {
    // try to open the device
    if ( cartOpen ( t, h ) == false )
    {
      cout << "Cannot open device! (Permissions problem?)" << endl;
      return 1;
    }
  }
  else if ( operation == "info" )
  {
    // try to open the device
    if ( cartOpen ( t, h ) == false )
    {
      cout << "Cannot open device! (Permissions problem?)" << endl;
      return 1;
    }
  }
  else
  {
    // help!
    cout << argv [ 0 ] << " operation [ options ]" << endl
         << endl
         << "where;" << endl
         << "  operation  = [ read | write | readram | writeram | info ]"
            << endl
         << "options;" << endl
         << "  -b integer = block size (default: " << BLOCK_SIZE_DEFAULT
            << ")" << endl
         << "  -c integer = block count to read/write (default: "
            << BLOCK_COUNT_DEFAULT << ")" << endl
         << "  -o integer = read/write block offset (default: "
            << BLOCK_OFFSET_DEFAULT << ")" << endl
         << "  -f filename = file to read/write (for everything other than info)"
            << endl
         << endl;
  }
  
/*
    else if ( ! strcmp ( argv [ 1 ], "writeram" ) )
    {
      // did they specify a start offset?
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

        // number of blocks to use
        int bcount = ( length / block_size ) + ( length % block_size > 0 ? 1 : 0 );

        cout << "Writing \"" << argv [ 2 ] << "\" to cart." << endl << flush;

        //t.CartOpenFlashOP ( h );
        t.CartOpenPort ( h );

        cout << "SetReadArray ... " << flush;

        t.SetReadArray ( h );

        cout << "Done." << endl;

        // quarter megabit chunks
        cout << "Writing " << bcount
             << " blocks (in " << block_size << " byte chunks) ... --%" << flush;

        BYTE buf [ block_size ];
        unsigned long p = 0;
        float percent = 0;
        
        offset *= block_size;

        while ( p < length )
        {
          percent = ( ( float ) p / ( float ) length ) * 100.0f;

          cout << "\b\b\b" << setw ( 2 ) << ( int ) percent << "%" << flush;

          fread ( buf, block_size, 1, f );

          t.CartRAMWriteEx ( h, offset + p, buf, block_size );

          p += block_size;
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
*/
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
