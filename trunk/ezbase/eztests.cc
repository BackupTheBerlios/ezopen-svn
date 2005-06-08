#include <iostream>
#include "ezopen_ezf2.h"
using namespace std;

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

  dev.cartOpen(h) ? cout << "  * Device opened\n" : cout << "  ! Couldn't open device.\n";

  dev.cartClose(h) ? cout << "  * Device closed\n" : cout << "  ! Couldn't close device.\n";
  cout << "-=--=--=--=--=--=--=--=--=--=--=-" << endl;
  

  return 0;
}
