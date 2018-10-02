#ifndef UDP_SERVER_H
#define UDP_SERVER_H


#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

namespace UDP {
  extern const int kMaxBuffer;

class Server {
 public:

  // Constructor
  Server(const std::string& host, int port);
  ~Server() = default;

  // copying and moving not allowed
  Server(const Server& rhs) = delete;
  Server(Server&& rhs) = delete;
  Server& operator=(const Server& rhs) = delete;
  Server& operator=(Server&& rhs) = delete;

  // member functions
  void receive(std::string& return_buffer);
  void send(const std::string& server_ip, int server_port, const std::string& buffer);




 private:
  int sd_;
  struct sockaddr_in sock_;
  int port_;
  std::string host_;





};

} // namespace UDP



#endif // UDP_SERVER_H