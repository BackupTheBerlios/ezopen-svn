#include <iostream>
#include "ezopen_ezf2.h"
using namespace std;

int main( int argc, char *argv [] )
{
  ez_handle_t h = NULL;
  ezopen_ezf2 dev;

  // open, claim, close
  cout << "cartOpen\n";
  dev.cartOpen(h) ? cout << " * Device opened\n" : cout << " ! Couldn't open device.\n";


  cout << "cartClose\n";
  dev.cartClose(h) ? cout << " * Device closed\n" : cout << " ! Couldn't close device.\n";

  // open, claim, read, close
  

  return 0;
}
