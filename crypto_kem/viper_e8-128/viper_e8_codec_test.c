#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "viper_e8.h"

int main(void) {
  unsigned char message[VIPER_MSGBYTES];
  unsigned char decoded[VIPER_MSGBYTES];
  vpoly encoded;

  const unsigned bytes_per_block = VIPER_E8_BITS_PER_BLOCK / 8u;
  const unsigned labels = 1u << VIPER_E8_BITS_PER_BLOCK;
  const unsigned label_step = VIPER_E8_BITS_PER_BLOCK == 8u ? 1u : 257u;

  if ((VIPER_E8_BITS_PER_BLOCK != 8u &&
       VIPER_E8_BITS_PER_BLOCK != 16u) ||
      VIPER_E8_ACTIVE_BLOCKS * bytes_per_block != VIPER_MSGBYTES) {
    puts("unsupported-test-parameters");
    return 2;
  }

  for (unsigned block = 0; block < VIPER_E8_ACTIVE_BLOCKS; block++) {
    for (unsigned label = 0; label < labels; label += label_step) {
      memset(message, 0, sizeof(message));
      for (unsigned i = 0; i < bytes_per_block; i++) {
        message[block * bytes_per_block + i] =
            (unsigned char)(label >> (8u * i));
      }
      viper_e8_encode(encoded, message);

      for (unsigned j = 0; j < 8; j++) {
        uint16_t expected = viper_e8_label_to_coeff(label, j);
        if (encoded[8 * block + j] != expected) {
          printf("encode mismatch: block=%u label=%u coord=%u\n",
                 block, label, j);
          return 1;
        }
      }

      viper_e8_decode(decoded, encoded);
      if (memcmp(decoded, message, sizeof(message)) != 0) {
        printf("roundtrip mismatch: block=%u label=%u\n", block, label);
        return 1;
      }
    }
  }

  puts("viper-e8-codec: ok");
  return 0;
}
