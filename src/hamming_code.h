#ifndef HAMMING_CODE_H
#define HAMMING_CODE

#define VERSION    2
#define SUBVERSION 0

#define DATA_LEN    16
#define MATRIX_SIZE 4

#include <stdint.h>

typedef struct
{
  uint16_t bits;
} binary_data_t;

#endif /* HAMMING_CODE */