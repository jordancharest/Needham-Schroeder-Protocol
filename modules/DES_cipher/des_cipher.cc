#include "des_cipher.h"

#include <iostream>

namespace DES {
Cipher::Cipher() {
  this->generateSubkeys();
}

// ----------------------------------------------------------------------------
void Cipher::generateSubkeys() {
  // initial permutation: 1 2 3 4 5 6 7 8 9 10  -->  3 5 2 7 4 10 1 9 8 6
  // Original: 1011010111 (2D7)
  // Permuted: 1000111111 (23F)

  // Credit: http://programming.sirrida.de/calcperm.php for this permutation
  uint16_t permuted = ((this->key & 0x00000002) << 1)
                    | ((this->key & 0x00000080) << 2)
                    | ((this->key & 0x00000028) << 3)
                    | ((this->key & 0x00000001) << 4)
                    | ((this->key & 0x00000200) >> 6)
                    | ((this->key & 0x00000010) >> 4)
                    | ((this->key & 0x00000144) >> 1);

  // mask the permuted key to get the left and right halves
  uint16_t left = (permuted & (0x1F << 5)) >> 5;
  uint16_t right = permuted & 0x1F;

  // left shift left half
  uint16_t carry = left & 0x10;
  left <<= 1;
  left |= (carry >> 4);
  left &= 0x1F;

  // left shift right half
  carry = right & 0x10;
  right <<= 1;
  right |= (carry >> 4);
  right &= 0x1F;

  // concatenate left and right
  this->key1 = (left << 5) | right;
  
  // permute again to get key1
  this->key1  = (this->key1 & 0x00000300)
              | ((this->key1 & 0x00000005) << 1)
              | ((this->key1 & 0x00000008) << 2)
              | ((this->key1 & 0x00000010) << 3)
              | ((this->key1 & 0x00000020) >> 3)
              | ((this->key1 & 0x00000040) >> 2)
              | ((this->key1 & 0x00000082) >> 1);
  
  // strip off the two most significant bits to truncate 10 bits to 8
  this->key1 &= 0x0FF;

  // left shift the left and right halves again
  carry = left & 0x10;
  left <<= 1;
  left |= (carry >> 4);
  left &= 0x1F;

  carry = right & 0x10;
  right <<= 1;
  right |= (carry >> 4);
  right &= 0x1F;

  // concatenate and permute again to get key 2
  this->key2 = (left << 5) | right;
  this->key2  = (this->key2 & 0x00000300)
              | ((this->key2 & 0x00000005) << 1)
              | ((this->key2 & 0x00000008) << 2)
              | ((this->key2 & 0x00000010) << 3)
              | ((this->key2 & 0x00000020) >> 3)
              | ((this->key2 & 0x00000040) >> 2)
              | ((this->key2 & 0x00000082) >> 1);

  // strip off the two most significant bits to truncate 10 bits to 8
  this->key2 &= 0x0FF;
}

// ----------------------------------------------------------------------------
uint8_t Cipher::feistel(uint8_t data, uint16_t key) {
  // expansion and permutation
  uint8_t carry = data & 0x01;
  uint8_t left = (data >> 1) | (carry << 3);

  carry = (data & 0x08) >> 3;
  uint8_t right = (data << 1) | carry;
  right &= 0x0F;

  // concatenate left and right
  uint16_t permuted = (left << 4) | right;

  // incorporate the key
  permuted = permuted ^ key;

  // calculate bits coming from S-box 0 (left)
  // column is from bits 5 and 6 (0 indexed)
  uint8_t col = (permuted >> 5) & 0x03;

  // row is from bits 4 and 7
  carry = (permuted & 0x10) >> 4;
  uint8_t row = ((permuted >> 6) | carry) & 0x03;

  uint8_t two_bit_left = this->S0[row][col] & 0x03;

  // calculate bits coming from S-box 1 (right)
  // column is from bits 1 and 2
  col = (permuted >> 1) & 0x03;

  // row is from bits 0 and 3
  carry = permuted & 0x01;
  row = ((permuted >> 2) | carry) & 0x03;

  uint8_t two_bit_right = this->S1[row][col] & 0x03;

  // concatenate and permute the bits again
  return ((two_bit_left & 0x01) << 3) | ((two_bit_right & 0x01) << 2)
          | (two_bit_right & 0x02) | ((two_bit_left & 0x02) >> 1);
}

// ----------------------------------------------------------------------------
uint8_t Cipher::encrypt(uint8_t plain) {
  // std::cout << "\nEncrypting...\n";

  // permute bit positions: 1 2 3 4 5 6 7 8  -->  2 6 3 1 4 8 5 7
  uint8_t cipher  = (plain & 0x00000020)
                  | ((plain & 0x00000040) << 1)
                  | ((plain & 0x00000001) << 2)
                  | ((plain & 0x00000004) << 4)
                  | ((plain & 0x00000080) >> 3)
                  | ((plain & 0x00000008) >> 2)
                  | ((plain & 0x00000012) >> 1);

  // separate into left and right
  uint8_t left = (cipher & (0xF << 4)) >> 4;
  uint8_t right = cipher & 0xF;

  // first round
  uint8_t temp = right;
  right = left ^ this->feistel(right, this->key1);
  left = temp;

  // second round
  left = left ^ this->feistel(right, this->key2);

  // concatenate left and right
  cipher = (left << 4) | right;

  // reverse permutation
  cipher  = (cipher & 0x00000020)
          | ((cipher & 0x00000009) << 1)
          | ((cipher & 0x00000002) << 2)
          | ((cipher & 0x00000010) << 3)
          | ((cipher & 0x00000040) >> 4)
          | ((cipher & 0x00000004) >> 2)
          | ((cipher & 0x00000080) >> 1);

  return cipher;
}

// ----------------------------------------------------------------------------
uint8_t Cipher::decrypt(uint8_t cipher) {
  // std::cout << "\nDecrypting...\n";

  // permute bit positions: 1 2 3 4 5 6 7 8  -->  2 6 3 1 4 8 5 7
  uint8_t plain   = (cipher & 0x00000020)
                  | ((cipher & 0x00000040) << 1)
                  | ((cipher & 0x00000001) << 2)
                  | ((cipher & 0x00000004) << 4)
                  | ((cipher & 0x00000080) >> 3)
                  | ((cipher & 0x00000008) >> 2)
                  | ((cipher & 0x00000012) >> 1);

  // separate into left and right
  uint8_t left = (plain & (0xF << 4)) >> 4;
  uint8_t right = plain & 0xF;

  // first round
  uint8_t temp = right;
  right = left ^ this->feistel(right, this->key2);
  left = temp;

  // second round
  left = left ^ this->feistel(right, this->key1);

  // concatenate left and right
  plain = (left << 4) | right;

  // reverse permutation
  plain = (plain & 0x00000020)
        | ((plain & 0x00000009) << 1)
        | ((plain & 0x00000002) << 2)
        | ((plain & 0x00000010) << 3)
        | ((plain & 0x00000040) >> 4)
        | ((plain & 0x00000004) >> 2)
        | ((plain & 0x00000080) >> 1);

  return plain;
}

}