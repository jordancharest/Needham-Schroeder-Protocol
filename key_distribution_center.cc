#include <cstdlib>
#include <iostream>

#include "des_cipher.h"
#include "udp_server.h"


int main(int argc, char** argv) {
  std::string host = "127.0.0.1";
  int port = 5000;

  DES::Cipher cipher;
  UDP::Server server(host, port);

    

 


    return EXIT_SUCCESS;
}