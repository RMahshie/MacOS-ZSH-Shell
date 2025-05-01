/**
 * @file bitmap.c
 * @author CS3650 staff
 *
 * Bitmap implementation.
 */
#include <stdint.h>
#include <stdio.h>

#include "bitmap.h"

#define nth_bit_mask(n) (1 << (n))
#define byte_index(n) ((n) / 8)
#define bit_index(n) ((n) % 8)

// Get the given bit from the bitmap.
int bitmap_get(void *bm, int i) {
  unsigned char *bs = (unsigned char *) bm;

  return (bs[i/8] & (1 << (i % 8))) != 0;
}

// Set the given bit in the bitmap to the given value.
void bitmap_put(void *bm, int i, int val) {
  unsigned char *bs = (unsigned char *) bm;

  if (val) {
    bs[i/8] |= (1 << (i % 8));
  } else {
    bs[i/8] &= ~(1 << (i % 8));
  }
}

// Pretty-print the bitmap (with the given no. of bits).
void bitmap_print(void *bm, int size) {

  for (int i = 0; i < size; i++) {
    putchar(bitmap_get(bm, i) ? '1' : '0');

    if ((i + 1) % 64 == 0) {
      putchar('\n');
    } else if ((i + 1) % 8 == 0) {
      putchar(' ');
    }
  }
}
