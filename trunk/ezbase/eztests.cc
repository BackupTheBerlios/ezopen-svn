#include <iostream>
#include "EzopenBaseIO.h"
using namespace std;

int main( int argc, char *argv [] )
{
  handle_t h = NULL;
  EzopenBaseIO dev;

  cout << "cartOpen\n";
  dev.cartOpen(h) ? cout << " * Device opened\n" : cout << " ! Couldn't open device.\n";


  cout << "cartClose\n";
  dev.cartClose(h) ? cout << " * Device closed\n" : cout << " ! Couldn't close device.\n";

  return 0;
}
