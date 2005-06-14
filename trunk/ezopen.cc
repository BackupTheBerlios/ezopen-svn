#include <iostream>
#include <fstream>
#include <iomanip>

#include <sys/types.h>

#include "CEzFlashFujistu.h"

#define BLOCK_COUNT   2
#define BLOCK_SIZE    0x8000
#define ERASE_BLOCK_SIZE  0x10000
#define RAM_BLOCK_SIZE  64
#define BLOCK_OFFSET  0

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
readROMOpen ( CEzFlashBase &t, HANDLE &h )
{
  t.CartSetROMPage ( h, 0 );
  t.CartOpenPort ( h );
  t.CartOpenReadOP ( h );
  t.SetReadArray ( h );
}

// read data from the ROM
void
readROM ( CEzFlashBase &t, HANDLE &h, u_int32_t offset, u_int32_t length,
    BYTE *buf )
{
  t.CartReadEx ( h, offset, buf, length );
}

// finish reading of ROM
void
readROMClose ( CEzFlashBase &t, HANDLE &h )
{
  t.CartCloseReadOP ( h );
  t.CartClosePort ( h );
}

void
readRAMOpen ( CEzFlashBase &t, HANDLE &h, u_int32_t page )
{
  t.CartOpenFlashOP ( h );
  t.CartSetRAMPage ( h, page );
  t.SetReadArray ( h );
}

void
readRAM ( CEzFlashBase &t, HANDLE &h, u_int32_t offset, BYTE* buf, u_int32_t bs )
{
  //t.CartRAMReadEx ( h, offset, buf, bs );
  t.CartRAMRead ( h, offset, buf, bs );
}

void readRAMClose ( CEzFlashBase &t, HANDLE &h )
{
  t.CartCloseFlashOP ( h );
}

void
writeRAMOpen ( CEzFlashBase &t, HANDLE &h, u_int32_t page )
{
  //t.CartOpenPort ( h );
  t.CartOpenFlashOP ( h );
  t.CartSetRAMPage ( h, page );
  t.SetReadArray ( h );
}

void
writeRAM ( CEzFlashBase &t, HANDLE &h, u_int32_t offset, BYTE* buf, u_int32_t bs )
{
  //t.CartRAMWriteEx ( h, offset, buf, bs );
  t.CartRAMWrite ( h, offset, buf, bs );
}

void
writeRAMClose ( CEzFlashBase &t, HANDLE &h )
{
  //t.CartClosePort ( h );
  t.CartCloseFlashOP ( h );
}

// initialise writing/erasing of ROM
void
writeROMOpen ( CEzFlashBase &t, HANDLE &h )
{
  t.CartOpenFlashOP ( h );
  t.SetReadArray ( h );
}

void
writeROM ( CEzFlashBase &t, HANDLE &h, u_int32_t offset, BYTE* buf, u_int32_t bs )
{
  t.CartWriteEx ( h, offset, buf, bs );
}

// close ROM writing
void
writeROMClose ( CEzFlashBase &t, HANDLE &h )
{
  t.CartCloseFlashOP ( h );
}

// open ROM erasing
void
eraseROMOpen ( CEzFlashBase &t, HANDLE &h )
{
  t.CartOpenFlashOP ( h );
}

void
eraseROM ( CEzFlashBase &t, HANDLE &h, u_int32_t block )
{
  t.CartErase ( h, block );
}

// close ROM erasing
void
eraseROMClose ( CEzFlashBase &t, HANDLE &h )
{
  t.CartCloseFlashOP ( h );
}

// get cart info
void
romGetInfo ( CEzFlashBase &t, HANDLE &h, int offset, struct gba_header* gbah )
{
  // start by reading the first 192 bytes, which should contain the first
  // ROM's header
  readROMOpen ( t, h );
  
  //readROM ( t, h, offset, sizeof ( gbah ), ( BYTE* ) gbah );
  readROM ( t, h, offset, 192, ( BYTE* ) gbah );

  readROMClose ( t, h );
}

void
romDisplayInfo ( struct gba_header *gbah )
{
  cout << "Entry address: 0x" << hex << gbah->start_address << endl
       << "Title: \"" << gbah->title << "\"" << endl
       << "Game code: " << gbah->code << endl
       << "Maker: " << gbah->maker << endl
       << "ROM size: " << dec << ( u_int32_t ) gbah->rom_size
          << " * 32K" << endl
       << "Saver size: " << dec << ( u_int32_t ) gbah->saver_size
          << " * 32K" << endl
       << "Reserved (1): 0x"
          << hex << ( ( ( u_int32_t ) gbah->reserved ) & 0xff ) << endl
       << "Unit code: 0x"
          << hex << ( ( ( u_int32_t ) gbah->unit_code ) & 0xff ) << endl
       << "Device type: 0x"
          << hex << ( ( ( u_int32_t ) gbah->device_type ) & 0xff ) << endl
       << "Reserved (2): 0x"
          << hex << ( ( ( u_int32_t ) gbah->reserved1 [ 0 ] ) & 0xff )
          << " 0x" << hex << ( ( ( u_int32_t ) gbah->reserved1 [ 1 ] ) & 0xff )
          << " 0x" << hex << ( ( ( u_int32_t ) gbah->reserved1 [ 2 ] ) & 0xff )
          << endl
       << "Reserved (3): 0x"
          << hex << ( ( ( u_int32_t ) gbah->reserved2 [ 0 ] ) & 0xff )
          << " 0x" << hex << ( ( ( u_int32_t ) gbah->reserved2 [ 1 ] ) & 0xff )
          << endl
       << "Compliment byte: 0x"
          << hex << ( ( ( u_int32_t ) gbah->complement ) & 0xff ) << endl
       << "Menu tag: 0x"
          << hex << ( ( ( u_int32_t ) gbah->menu_tag ) & 0xff ) << endl
       << "Reserved (4): 0x"
          << hex << ( ( ( u_int32_t ) gbah->reserved3 ) & 0xff ) << endl;
}

bool
cartOpen ( CEzFlashBase &t, HANDLE &h )
{
  if ( t.OpenCartDevice ( h ) == false )
    return false;

  t.CartLock ( h );

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

  // number of blocks to read/write
  u_int32_t block_count = BLOCK_COUNT;
  
  // block offset
  u_int32_t block_offset = BLOCK_OFFSET;
 
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
      if ( ! strcmp ( argv [ l ], "-o" ) ) // read/write block offset
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

    // open the filename
    if ( filename == "" )
    {
      cout << "No ROM file specified." << endl;
      return 1;
    }
    
    // open the ROM file
    ofstream rom ( filename.c_str () );
    
    // make sure we can open the file ...
    if ( ! rom )
    {
      // couldn't open file :(
      cout << "Could not open output file \"" << filename << "\"." << endl;
      return 1;
    }
    else
    {
      cout << "Reading " << block_count << " blocks (1 block = " << BLOCK_SIZE
           << " bytes) from cart and writing to \"" << filename << "\"."
           << endl << "Reading - --%" << flush;

      // when erasing the block size is always 65536 (0x10000)
      BYTE buf [ BLOCK_SIZE ];
      float percent = 0.0f;
      float pinc = 100.0f / ( float ) block_count;

      // start ROM erasing
      readROMOpen ( t, h );

      for ( u_int32_t l = block_offset; l < ( block_offset + block_count ); ++l )
      {
        cout << "\b\b\b" << setw ( 2 ) << ( u_int32_t ) percent << "%" << flush;
        
        readROM ( t, h, ( l * BLOCK_SIZE ), BLOCK_SIZE, buf );

        rom.write ( ( char* ) buf, BLOCK_SIZE );
        
        percent += pinc;
      }

      // finish ROM writing
      readROMClose ( t, h );

      cout << "\b\b\bDone." << endl;
      
      rom.close ();
    }
    
    cartClose ( t, h );
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
      block_count = ( length / BLOCK_SIZE ) + ( length % BLOCK_SIZE > 0 ? 1 : 0 );

      cout << "Writing \"" << filename << "\" to cart "
           << "(cart is " << block_count << " * " << BLOCK_SIZE << " blocks) "
           << endl << "Erasing - --%" << flush;

      // start ROM erasing
      eraseROMOpen ( t, h );

      // when erasing the block size is always 65536 (0x10000)
      float percent = 0.0f;
      u_int32_t ebc = ( length / ERASE_BLOCK_SIZE ) + ( length % ERASE_BLOCK_SIZE > 0 ? 1 : 0 );
      float pinc = 100.0f / ( float ) ebc;
      
      for ( u_int32_t l = block_offset; l < ( block_offset + ebc ); ++l )
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

      BYTE buf [ BLOCK_SIZE ];
      percent = 0.0f;
      pinc = 100.0f / ( float ) block_count;
      
      for ( u_int32_t l = block_offset; l < ( block_offset + block_count ); ++l )
      {
        cout << "\b\b\b" << setw ( 2 ) << ( u_int32_t ) percent << "%" << flush;

        rom.read ( ( char* ) buf, BLOCK_SIZE );

        writeROM ( t, h, ( l * BLOCK_SIZE ), buf, BLOCK_SIZE );

        percent += pinc;
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

    // open the filename
    if ( filename == "" )
    {
      cout << "No RAM file specified." << endl;
      return 1;
    }
    
    // open the ram file
    ofstream ram ( filename.c_str () );
    
    // make sure we can open the file ...
    if ( ! ram )
    {
      // couldn't open file :(
      cout << "Could not open output file \"" << filename << "\"." << endl;
      return 1;
    }
    else
    {
      cout << "Reading " << block_count << " blocks (1 block = " << RAM_BLOCK_SIZE
           << " bytes) fram cart and writing to \"" << filename << "\"."
           << endl << "Reading - --%" << flush;

      // start ram erasing
      readRAMOpen ( t, h, 0 );

      // when erasing the block size is always 65536 (0x10000)
      BYTE buf [ RAM_BLOCK_SIZE ];
      float percent = 0.0f;
      float pinc = 100.0f / ( float ) block_count;

      for ( u_int32_t l = block_offset; l < ( block_offset + block_count ); ++l )
      {
        cout << "\b\b\b" << setw ( 2 ) << ( u_int32_t ) percent << "%" << flush;
        
        readRAM ( t, h, ( l * RAM_BLOCK_SIZE ), buf, RAM_BLOCK_SIZE );

        ram.write ( ( char* ) buf, RAM_BLOCK_SIZE );
        
        percent += pinc;
      }

      // finish ram writing
      readRAMClose ( t, h );

      cout << "\b\b\bDone." << endl;
      
      ram.close ();
    }
    
    cartClose ( t, h );
  }
  else if ( operation == "writeram" )
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
      cout << "No RAM file specified." << endl;
      return 1;
    }
    
    // open the ROM file
    ifstream ram ( filename.c_str () );
    
    // make sure we can open the file ...
    if ( ! ram )
    {
      // couldn't open file :(
      cout << "Could not open RAM file \"" << filename << "\"." << endl;
      return 1;
    }
    else
    {
      // find the full length of the file
      ram.seekg ( 0, ios::end );
      
      u_int32_t length = ram.tellg ();
      
      ram.seekg ( 0, ios::beg );
      
      // calculate the number of blocks to write
      block_count = ( length / RAM_BLOCK_SIZE ) + ( length % RAM_BLOCK_SIZE > 0 ? 1 : 0 );

      // start ROM writing
      writeRAMOpen ( t, h, 0 );

      cout << "Writing \"" << filename << "\" to cart "
           << "(cart is " << block_count << " * " << RAM_BLOCK_SIZE << " blocks) "
           << "Flashing - --%" << flush;

      // when erasing the block size is always 65536 (0x10000)
      float percent = 0.0f;
      float pinc = 100.0f / ( float ) block_count;

      BYTE buf [ RAM_BLOCK_SIZE ];
      percent = 0;
      
      for ( u_int32_t l = block_offset; l < ( block_offset + block_count ); ++l )
      {
        cout << "\b\b\b" << setw ( 2 ) << ( u_int32_t ) percent << "%" << flush;

        ram.read ( ( char* ) buf, RAM_BLOCK_SIZE );

        writeRAM ( t, h, ( l * RAM_BLOCK_SIZE ), buf, RAM_BLOCK_SIZE );

        percent += pinc;
      }

      // finish ROM writing
      writeRAMClose ( t, h );

      cout << "\b\b\bDone." << endl;
      
      ram.close ();
    }
    
    cartClose ( t, h );
  }
  else if ( operation == "info" )
  {
    struct gba_header* gbah = ( struct gba_header* ) calloc ( 1, sizeof ( struct gba_header ) );

    // should we read from a file?
    if ( filename == "" )
    {
      // try to open the device
      if ( cartOpen ( t, h ) == false )
      {
        cout << "Cannot open device! (Permissions problem?)" << endl;
        return 1;
      }

      romGetInfo ( t, h, 0, gbah );

      cartClose ( t, h );
    }
    else
    {
      ifstream rom ( filename.c_str () );

      if ( ! rom )
      {
        cout << "Cannot open ROM file \"" << filename << "\"." << endl;
        return 1;
      }

      rom.read ( ( char* ) gbah, 192 );

      rom.close ();
    }

    romDisplayInfo ( gbah );
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
         << "  -c integer = block count to read/write (default: "
            << BLOCK_COUNT << ")" << endl
         << "  -o integer = read/write block offset (default: "
            << BLOCK_OFFSET << ")" << endl
         << "  -f filename = file to read/write (for everything other than info)"
            << endl
         << endl;
  }
  
/*
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
