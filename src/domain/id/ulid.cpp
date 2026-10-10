#include "domain/id/ulid.h"

#include <chrono>
#include <cstdint>
#include <cstring>
#include <random>
#include <stdexcept>

void ULID::getTime() {
  auto now = std::chrono::system_clock::now();
  uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                           now.time_since_epoch())
                           .count();

  for (uint8_t chunk = 0; chunk < 6; ++chunk) {
    uint8_t bits = 40 - chunk * 8;
    data[chunk] = (timestamp >> bits) & 0xFF;
  }
}

void ULID::getRandom() {
  std::random_device rd;

  std::seed_seq seq{rd(), rd(), rd(), rd()};

  std::mt19937_64 generator(seq);

  uint64_t random64 = generator();

  for (uint8_t chunk = 0; chunk < 8; ++chunk) {
    uint8_t bits = 56 - chunk * 8;
    data[chunk + 6] = (random64 >> bits) & 0xFF;
  }

  uint16_t random16 = generator() & 0xFFFF;

  data[14] = (random16 >> 8) & 0xFF;
  data[15] = random16 & 0xFF;
}

std::string ULID::base64(uint8_t data[16]) {
  const size_t chunkSize = 5;

  std::string result(26, '0');
  std::string letters = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

  for (uint8_t chunk = 0; chunk < 26; ++chunk) {
    uint8_t value = 0;

    for (uint8_t bit = 0; bit < 5; ++bit) {
      uint8_t currentBit = chunk * chunkSize + bit;

      if (currentBit < 2) continue;

      currentBit -= 2;
      uint8_t byteIndex = currentBit / 8;
      uint8_t bitIndex = 7 - currentBit % 8;

      value <<= 1;
      value |= (data[byteIndex] >> bitIndex) & 1;
    }
    result[chunk] = letters[value];
  }

  return result;
}

ULID::ULID() {
  getTime();
  getRandom();

  str = base64(data);
}

ULID::ULID(std::vector<uint8_t> value) {
  if (value.size() < 16)
    throw std::runtime_error("ULID cannot initialize: value data to small");

  std::memcpy(data, value.data(), 16);
  str = base64(data);
}
