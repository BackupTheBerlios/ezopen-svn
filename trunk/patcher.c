#include <stdio.h>
#include <string.h>

/* ROM offset 0xBE is patched with 0xCF (if it's not that) */

/* For some reason the 'official' software uses 0xCF, but I can only make it
 * work when using 0xCE ... */
 
char patch_be [] = { 0xCF };

int main ( int argc, char* argv [] )
{
  FILE *inf;
  int length = 0;
  unsigned short blocks = 0;

  if ( argc < 2 )
  {
    printf ( "Usage: %s rom.gba\n", argv [ 0 ] );
    return 1;
  }

  char patch_saver = 0;

  if ( argc > 2 )
  {
    patch_saver = ( char ) atoi ( argv [ 2 ] );
    printf ( "Saver size = %d (%d bytes)\n", ( int ) patch_saver,
             ( int ) ( patch_saver * 32768 ) );

    if ( argc > 3 )
    {
      blocks = ( unsigned short ) atoi ( argv [ 3 ] );
    }
  }
  
  inf = fopen ( argv [ 1 ], "r+" );

  // patch 0xB8 with a short for the number of 32K blocks this rom takes up
  if ( ! blocks )
  {
    // find the length
    fseek ( inf, 0, SEEK_END );
    length = ftell ( inf );
    rewind ( inf );
  
    blocks = ( length / 0x8000 ) + ( length % 0x8000 > 0 ? 1 : 0 );
  }

  // patch the number of 32k blocks that this ROM takes up
  fseek ( inf, 0xB8, SEEK_SET );
  fwrite ( &blocks, 2, 1, inf );

  // patch 0xBA (the number of 32k blocks the saver is)
  fseek ( inf, 0xBA, SEEK_SET );
  fwrite ( &patch_saver, 1, 1, inf );

  // patch 0cBC with ... err ... dunno, 2 = EEPROM saver?
  fseek ( inf, 0xBC, SEEK_SET );
  char saver_type = 2;  // EEPROM?
  fwrite ( &saver_type, 1, 1, inf );
  
  // patch 0xBE with 0xCF (game list flag)
  fseek ( inf, 0xBE, SEEK_SET );
  fwrite ( patch_be, 1, 1, inf );

  // finally, recalculate the complement byte (0xBD) a0h  and bch
  // FIXME: this doesn't work properly, not sure why.
  fseek ( inf, 0xA0, SEEK_SET );
  char b [ 29 ];
  fread ( &b, 29, 1, inf );

  int sum = 0x18;
  int i = 0;
  
  for ( i = 0; i < 29; ++i )
    sum += b [ i ];

  sum ^= 0xff;
  
  char c = sum & 0xff;

  // bleh.
  printf ( "Complement check bytes: 0x%02X\n", c );

  fwrite ( &c, 1, 1, inf );

  fclose ( inf );

  return 0;
}
