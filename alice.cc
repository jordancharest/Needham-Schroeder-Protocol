#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "des_cipher.h"
#include "udp_server.h"

long long dh_private_key = 3;
std::string name = "Alice";

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
  long long generated_key = (long long)pow(G, dh_private_key) % P;
  server.send("127.0.0.1", port, std::to_string(generated_key));

  // wait to receive a message from user
  std::string buffer;
  server.receive(buffer);
  long long received_key = (long long)stoi(buffer);

  // compute session key
  long long session_key = (long long)pow(received_key, dh_private_key) % P;

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
  int port = 5001;
  int port_bob = 5002;
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
  std::string str_private_key;
  std::cin >> str_private_key;
  std::string encrypted = "";
  for (char c : str_private_key)
    encrypted += cipher_server.encrypt(c);

  server.send("127.0.0.1", server_port, encrypted);

  uint16_t private_key = std::stoi(str_private_key, nullptr, 16);

  // wait for the server to respond with the session key
  DES::Cipher private_cipher(private_key);
  server.receive(buffer);
  decrypted = "";
  for (char c : buffer)
    decrypted += private_cipher.decrypt(c);

  std::cout << "Received " << decrypted << std::endl;
  uint16_t session_key_bob = std::stoi(decrypted, nullptr, 16);

  // receive the session key again but encrypted with Bob's private key,
  // forward it to Bob
  server.receive(buffer);
  server.send("127.0.0.1", port_bob, buffer);



  
  return EXIT_SUCCESS;
}