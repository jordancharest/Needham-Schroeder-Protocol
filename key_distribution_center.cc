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
void read_public_info(char** argv, long long* P_alice, long long* G_alice, 
                      long long* P_bob, long long* G_bob) {
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
  std::cout << "\nBob's public info: \n"
            << "P: " << *P_bob << "\n"
            << "G: " << *G_bob << std::endl;
}

// ----------------------------------------------------------------------------
uint16_t diffie_hellman(UDP::Server& server, int port, long long P, long long G) {
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


// ----------------------------------------------------------------------------
uint16_t secure_connection(UDP::Server& server, std::string& name, 
                            int client_port, long long P, long long G) {
  std::cout << "\nWaiting to receive a connection request from " << name << "\n";
  uint16_t session_key = diffie_hellman(server, client_port, P, G);
  std::cout << "KDC: The session key with " << name << " is " << session_key << std::endl;

  return session_key;
}


// ----------------------------------------------------------------------------
uint16_t prompt_user_and_receive_key(UDP::Server& server, DES::Cipher& cipher, 
                                      std::string& msg, int client_port, 
                                      std::string& name) {

  std::string encrypted = "";
  for (char c : msg)
    encrypted += cipher.encrypt(c);
  server.send("127.0.0.1", client_port, encrypted);

  // receive user's (encrypted) private key
  std::string buffer;
  server.receive(buffer);
  std::string decrypted = "";
  for (char c : buffer)
    decrypted += cipher.decrypt(c);

  std::cout << "Received private key from " << name << std::endl;
  uint16_t private_key = std::stoi(decrypted, nullptr, 16);
  return private_key;
}



// ============================================================================
int main(int argc, char** argv) {
  validate_input(argc, argv);

  // read alice and bob's public info
  long long P_alice, G_alice, P_bob, G_bob;
  read_public_info(argv, &P_alice, &G_alice, &P_bob, &G_bob);

  // start server
  int port = 5000;
  int port_alice = 5001;
  int port_bob = 5002;
  std::string host = "127.0.0.1";
  UDP::Server server(host, port);

  // establish a secure connection with Alice
  std::string name = "Alice";
  uint16_t session_key_alice = secure_connection(server, name, port_alice, P_alice, G_alice);
  
  // prompt Alice to send the key for her communication with Bob
  DES::Cipher cipher_alice(session_key_alice);
  std::string msg = "Hello Alice, please input the 10-bit key you wish to use in your"
                    " communication with Bob (3-digit hex):";
  uint16_t alice_private_key = prompt_user_and_receive_key(server, cipher_alice, msg, port_alice, name);

  // establish a secure connection with Bob
  name = "Bob";
  uint16_t session_key_bob = secure_connection(server, name, port_bob, P_bob, G_bob);

  // Prompt Bob to send a private key to talk to Alice
  DES::Cipher cipher_bob(session_key_bob);
  msg = "Hello Bob, Alice wishes to communicate. Please input the 10-bit key"
        " you wish to use (3-digit hex):";
  uint16_t bob_private_key = prompt_user_and_receive_key(server, cipher_bob, msg, port_bob, name);

  // --------- DIFFIE-HELLMAN COMPLETE; BEGIN NEEDHAM SCHROEDER --------- //


  return EXIT_SUCCESS;
}