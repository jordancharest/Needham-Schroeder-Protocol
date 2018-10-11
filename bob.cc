#include <cstdlib>
#include <fstream>
#include <iostream>

#include "des_cipher.h"
#include "udp_server.h"

// ----------------------------------------------------------------------------
void read_key(char** argv, uint16_t* key) {
  // read my private key
  std::ifstream in(argv[1]);
  if (!in.good()) {
    std::cerr << "ERROR: failed to open key file\n";
    std::exit(EXIT_FAILURE);
  }

  in >> std::hex >> *key;
  std::cout << std::hex << "My (Bob) Key: " << *key << std::endl;
}

// ----------------------------------------------------------------------------
inline void validate_input(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Invalid Argument(s).\n";
    std::cerr << "USAGE: " << argv[0] << " <keys-file>\n";
    std::exit(EXIT_FAILURE);
  }
}

// ============================================================================
int main(int argc, char** argv) {
  validate_input(argc, argv);

  // read my private key and build cipher object
  uint16_t key;
  read_key(argv, &key);
  DES::Cipher cipher(key);

  // start server
  int port = 5002;
  std::string host = "127.0.0.1";
  UDP::Server server(host, port);



    




  return EXIT_SUCCESS;
}