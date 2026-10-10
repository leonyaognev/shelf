#pragma once

#include <cstdint>
#include <string>
#include <vector>

// -------------------------------------------------------------
// ULID: 128-bit identifier.
// First 6 bytes store the current time in milliseconds,
// the remaining 10 bytes are random. The result is encoded
// as a 26-character Crockford base32 string exposed in `id`.
// -------------------------------------------------------------
class ULID {
  void getTime();
  void getRandom();
  std::string base64(uint8_t data[16]);

 public:
  std::string str;
  uint8_t data[16];

  ULID();
  ULID(std::vector<uint8_t> value);
};
