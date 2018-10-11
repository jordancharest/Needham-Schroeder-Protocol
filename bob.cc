#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "des_cipher.h"
#include "udp_server.h"

long long private_key = 3;
// long long private_key = 7;
std::string name = "Bob";

// ----------------------------------------------------------------------------
void read_public_info(char** argv, long long* P, long long* G) {
  // read my public info for Diffie Hellman
  std::ifstream in(argv[1]);
  if (!in.good()) {
    std::cerr << "ERROR: failed to open key file\n";
    std::exit(EXIT_FAILURE);
  }

  in >> *P >> *G;
  std::cout << "Generating a session key with my (" << name 
            << ") public info:\nP: " << *P << "\nG: " << *G 
            << std::endl;
}

// ----------------------------------------------------------------------------
inline void validate_input(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Invalid Argument(s).\n";
    std::cerr << "USAGE: " << argv[0] << " <keys-file>\n";
    std::exit(EXIT_FAILURE);
  }
}

// ----------------------------------------------------------------------------
uint16_t diffie_hellman(UDP::Server& server, int port, long long P, long long G) {
  // generate key and send to server
  long long generated_key = (long long)pow(G, private_key) % P;
  server.send("127.0.0.1", port, std::to_string(generated_key));

  // wait to receive a message from user
  std::string buffer;
  server.receive(buffer);
  long long received_key = (long long)stoi(buffer);

  // compute session key
  long long session_key = (long long)pow(received_key, private_key) % P;

  // strip off all but ten least significant bits
  session_key &= 0x3FF;
  session_key ^= 0x3FF;

  return static_cast<uint16_t>(session_key);
}

// ============================================================================
int main(int argc, char** argv) {
  validate_input(argc, argv);

  long long P, G;
  read_public_info(argv, &P, &G);

  // start the UDP client
  int port = 5002;
  std::string host = "127.0.0.1";
  UDP::Server server(host, port);

  // establish a secure connection with the server
  int server_port = 5000;
  uint16_t session_key_server = diffie_hellman(server, server_port, P, G);
  DES::Cipher cipher_server(session_key_server);

  std::cout << "\nThe session key with the server is " << session_key_server << std::endl;

  // wait for prompt from the server
  std::string buffer;
  server.receive(buffer);
  std::cout << "\nReceived encrypted message: " << buffer << "\nDecrypting...\n";

  std::string decrypted = "";
  for (char c : buffer)
    decrypted += cipher_server.decrypt(c);
  std::cout << decrypted << std::endl;

  // enter the private key you want to use and send to server
  std::string str_session_key_alice;
  std::cin >> str_session_key_alice;
  std::string encrypted = "";
  for (char c : str_session_key_alice)
    encrypted += cipher_server.encrypt(c);

  server.send("127.0.0.1", server_port, encrypted);

  uint16_t session_key_bob = std::stoi(str_session_key_alice, nullptr, 16);




    




  return EXIT_SUCCESS;
}