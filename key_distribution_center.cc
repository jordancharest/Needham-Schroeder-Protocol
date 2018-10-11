#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "des_cipher.h"
#include "udp_server.h"

long long private_key = 9;


// ----------------------------------------------------------------------------
inline void validate_input(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Invalid Argument(s).\n";
    std::cerr << "USAGE: " << argv[0] << " <keys-file>\n";
    std::exit(EXIT_FAILURE);
  } 
}

// ----------------------------------------------------------------------------
void read_public_info(char** argv, int* P_alice, int* G_alice, int* P_bob, int* G_bob) {
  // read Alice and Bob's keys
  std::ifstream in(argv[1]);
  if (!in.good()) {
    std::cerr << "ERROR: failed to open key file\n";
    std::exit(EXIT_FAILURE);
  }
  in >> *P_alice >> *G_alice;
  std::cout << "Alice's public info: \n"
            << "P: " << *P_alice << "\n"
            << "G: " << *G_alice << std::endl;

  in.close();
  in.open(argv[2]);
  if (!in.good()) {
    std::cerr << "ERROR: failed to open key file\n";
    std::exit(EXIT_FAILURE);
  }  

  in >> *P_bob >> *G_bob;
}

// ----------------------------------------------------------------------------
uint16_t diffie_hellman(UDP::Server& server, int port, int P, int G) {
  // wait to receive a message from user
  std::string buffer;
  server.receive(buffer);
  long long received_key = (long long)stoi(buffer);

  // send generated key to user
  long long generated_key = (long long)pow(G, private_key) % P;
  server.send("127.0.0.1", port, std::to_string(generated_key));

  // compute session key
  long long session_key = (long long)pow(received_key, private_key) % P;

  // strip off all but last ten bits
  session_key &= 0x3FF;
  session_key ^= 0x3FF;

  return static_cast<uint16_t>(session_key);
}

// ============================================================================
int main(int argc, char** argv) {
  validate_input(argc, argv);

  // read alice and bob's keys, build their respective cipher objects
  int P_alice, G_alice, P_bob, G_bob;
  read_public_info(argv, &P_alice, &G_alice, &P_bob, &G_bob);

  // start server
  int port = 5000;
  int port_alice = 5001;
  int port_bob = 5002;
  std::string host = "127.0.0.1";
  UDP::Server server(host, port);

  // establish a secure connection with Alice
  std::cout << "Waiting to receive a connection request from Alice\n";
  long long session_key_alice = diffie_hellman(server, port_alice, P_alice, G_alice);
  std::cout << "KDC: The session key with Alice is " << session_key_alice << std::endl;
  
  // prompt Alice to send the key for her communication with Bob
  DES::Cipher cipher_alice(session_key_alice);
  std::string msg = "Hello Alice, please input the 10-bit key you wish to use in your"
                  " communication with Bob";
  std::string encrypted = "";
  for (char c : msg)
    encrypted += cipher_alice.encrypt(c);
  server.send("127.0.0.1", port_alice, encrypted);

  // receive Alice's private key
  std::string buffer;
  server.receive(buffer);
  std::cout << buffer << std::endl;
  std::string decrypted = "";
  for (char c : buffer)
    decrypted += cipher_alice.decrypt(c);

  std::cout << "Alice's private key: " << decrypted << std::endl;
  uint16_t alice_private_key = std::stoi(decrypted, nullptr, 16);




  // establish a secure connection with Bob
  // long long session_key_bob = diffie_hellman(server, port_bob, P_bob, G_bob);

  return EXIT_SUCCESS;
}