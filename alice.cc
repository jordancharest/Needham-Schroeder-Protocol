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

// ----------------------------------------------------------------------------
void secure_messaging(UDP::Server& server, DES::Cipher& session_cipher,
                      int port) {
  std::string msg;
  std::string buffer;
  fd_set read_fd_set;
  int sd = server.getSocketDescriptor();

  while (true) {
    struct timeval timeout;
    timeout.tv_sec = 60;
    timeout.tv_usec = 500;  // 60 AND 500 microseconds

    // reset the fd set for UDP socket and stdin
    FD_ZERO(&read_fd_set);
    FD_SET(sd, &read_fd_set);
    FD_SET(STDIN_FILENO, &read_fd_set);

    // check if any file descriptors are ready to be read from
    int ready = select(FD_SETSIZE, &read_fd_set, NULL, NULL, &timeout);
    if (ready == 0) {
        std::cout << "No activity\n";
        continue;

    // receive a message, decrypt and print it to terminal
    } else if (FD_ISSET(sd, &read_fd_set)) {
      server.receive(buffer);
      std::cout << "Received encrypted message. Decrypting...\n";
      session_cipher.decrypt(buffer, msg);
      std::cout << msg << std::endl;

    // read stdin, encrypt and send it to user 
    } else if (FD_ISSET(STDIN_FILENO, &read_fd_set)) {
      std::getline(std::cin, buffer);
      session_cipher.encrypt(buffer, msg);
      server.send("127.0.0.1", port, msg);
    }
  }
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
  std::string decrypted;
  cipher_server.decrypt(buffer, decrypted);
  std::cout << decrypted << std::endl;

  // enter the private key you want to use and send to server
  std::string str_private_key;
  std::cin >> str_private_key;
  std::string encrypted;
  cipher_server.encrypt(str_private_key, encrypted);
  server.send("127.0.0.1", server_port, encrypted);

  uint16_t private_key = std::stoi(str_private_key, nullptr, 16);

  // wait for the server to respond with the session key
  DES::Cipher private_cipher(private_key);
  server.receive(buffer);
  private_cipher.decrypt(buffer, decrypted);
  
  uint16_t session_key_bob = std::stoi(decrypted);
  std::cout << "Session key with Bob: " << session_key_bob << std::endl;
  DES::Cipher cipher_session_bob(session_key_bob);

  // receive the session key again but encrypted with Bob's private key,
  // forward it to Bob
  server.receive(buffer);
  server.send("127.0.0.1", port_bob, buffer);

  // receive the timestamp and forward it to Bob
  server.receive(buffer);
  server.send("127.0.0.1", port_bob, buffer);

  // run the secure messaging server
  secure_messaging(server, cipher_session_bob, port_bob);



  
  return EXIT_SUCCESS;
}