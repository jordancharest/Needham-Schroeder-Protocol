#ifndef DES_CIPHER_H
#define DES_CIPHER_H

#include <stdint.h>

#include <vector>

namespace DES {

class Cipher {
 public:
  Cipher();
  ~Cipher() = default;

  // moving and copying is forbidden
  Cipher(const Cipher&) = delete;
  Cipher(Cipher&&) = delete;
  Cipher& operator=(const Cipher&) = delete;
  Cipher& operator=(Cipher&&) = delete;

  // Encrypt and decrypt one byte at a time
  uint8_t encrypt(uint8_t byte);
  uint8_t decrypt(uint8_t byte);


 private:
  // methods ------------------------------------
  void generateSubkeys();
  uint8_t feistel(uint8_t data, uint16_t key);

  // members ------------------------------------
  const uint16_t key = 0x2D7; // 0b1011010111
  uint16_t key1, key2;
  const std::vector<std::vector<int>> S0 = {{1,0,3,2}, 
                                            {3,2,1,0},
                                            {0,2,1,3},
                                            {3,1,3,2}};

  const std::vector<std::vector<int>> S1 = {{0,1,2,3},
                                            {2,0,1,3},
                                            {3,0,1,0},
                                            {2,1,0,3}};

};

}





#endif // DES_CIPHER_H