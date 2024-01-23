#include "hamming_code.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <getopt.h>

#define NO_ERROR_DETECTED           -1
#define UNREACHABLE_ERRORS_DETECTED -2

bool verbose = false;

void
clear_controls (binary_data_t *data)
{
  binary_data_t mask;
  mask.bits = 1 + (1 << 1) + (1 << 2) + (1 << 4) + (1 << 8);
  data->bits &= ~mask.bits;
}

void
set_ctrl1 (binary_data_t *data)
{
  bool ctrl1 = 0;
  for (int pos = 1; pos <= 15; pos += 2)
    ctrl1 ^= (data->bits >> pos) % 2;
  data->bits += ctrl1 << 1;
}

void
set_ctrl2 (binary_data_t *data)
{
  bool ctrl2 = 0;
  for (int pos = 2; pos <= 14; pos += 4)
    ctrl2 ^= ((data->bits >> pos) % 2) ^ ((data->bits >> (pos + 1)) % 2);
  data->bits += ctrl2 << 2;
}

void
set_ctrl3 (binary_data_t *data)
{
  bool ctrl3 = 0;
  for (int pos = 4; pos <= 7; pos++)
    ctrl3 ^= (data->bits >> pos) % 2;
  for (int pos = 12; pos <= 15; pos++)
    ctrl3 ^= (data->bits >> pos) % 2;
  data->bits += ctrl3 << 4;
}

void
set_ctrl4 (binary_data_t *data)
{
  bool ctrl4 = 0;
  for (int pos = 8; pos <= 15; pos++)
    ctrl4 ^= (data->bits >> pos) % 2;
  data->bits += ctrl4 << 8;
}

bool
calc_sum (binary_data_t data)
{
  bool sum = 0;
  for (int pos = 0; pos <= 15; pos++)
    sum ^= (data.bits >> pos) % 2;
  return sum;
}

void
set_controls (binary_data_t *data)
{
  clear_controls (data);
  set_ctrl1 (data);
  set_ctrl2 (data);
  set_ctrl3 (data);
  set_ctrl4 (data);
  data->bits += calc_sum (*data);
}

void
add_an_error (binary_data_t *data)
{
  unsigned int pos = rand () % (DATA_LEN);
  binary_data_t mask;
  mask.bits = 1 << pos;
  data->bits ^= mask.bits;
  if (verbose)
    fprintf (stdout, "hamming_code: error popped in position %u\n", pos);
}

int
find_an_error (binary_data_t data)
{
  int err_pos = 0;
  bool sum = calc_sum (data);
  fprintf(stdout, "sum = %d\n", sum);
  fprintf(stdout, "%d\n", data.bits % 2);
  for (int pos = 0; pos < DATA_LEN; pos++)
    {
      if ((data.bits >> pos) % 2)
        err_pos ^= pos;
    }
  if (err_pos == 0 && sum == data.bits % 2)
    {
      if (verbose)
        fputs ("hamming_code: there is probably no error\n", stdout);
      return NO_ERROR_DETECTED;
    }
  if (sum != data.bits % 2)
    {
      if (verbose)
        fprintf (stdout,
                 "hamming_code: if there is 1 error, it's in position %u\n",
                 err_pos);
      return err_pos;
    }
  if (verbose)
    fputs ("hamming_code: there is more than 1 error\n", stdout);
  return UNREACHABLE_ERRORS_DETECTED;
}

void
correct_an_error (binary_data_t *data)
{
  int err_pos = find_an_error (*data);
  if (err_pos == NO_ERROR_DETECTED || err_pos == UNREACHABLE_ERRORS_DETECTED)
    return;
  binary_data_t mask;
  mask.bits = (1 << err_pos);
  data->bits ^= mask.bits;
}

binary_data_t *
get_random_data ()
{
  binary_data_t *data = malloc (sizeof (binary_data_t));
  data->bits = rand () % (1 << DATA_LEN);
  return data;
}

binary_data_t *
get_singleton (unsigned int pos)
{
  if (pos > DATA_LEN)
    perror ("hamming_code: error: invalid position for singleton creation");
  binary_data_t *data = malloc (sizeof (binary_data_t));
  data->bits = 1 << pos;
  return data;
}

void
free_data (binary_data_t *data)
{
  free (data);
}

void
show_data_as_bytes (binary_data_t data)
{
  binary_data_t mask;
  mask.bits = 1;
  for (int i = 1; i <= DATA_LEN; i++)
    {
      if (mask.bits & data.bits)
        fputs ("1", stdout);
      else
        fputs ("0", stdout);
      if (i % 8 == 0)
        fputs (" ", stdout);
      mask.bits = mask.bits << 1;
    }
  fputs ("\n", stdout);
}

void
show_data_as_matrix (binary_data_t data)
{
  binary_data_t mask;
  mask.bits = 1;
  for (int lin = 1; lin <= MATRIX_SIZE; lin++)
    {
      for (int col = 1; col <= MATRIX_SIZE; col++)
        {
          if (mask.bits & data.bits)
            fputs ("1 ", stdout);
          else
            fputs ("0 ", stdout);
          mask.bits = mask.bits << 1;
        }
      fputs ("\n", stdout);
    }
}

int
main (int argc, char *argv[])
{
  const char *PROGRAM_USAGE = "usage : hamming_code [-v|-V|-h]\n";

  const char *PROGRAM_DESCRIPTION =
      "Error correction with Hamming codes (15,11) + extra parity bit\n";

  const char *OPTIONS_DESCRIPTION =
      "   -v,--verbose           verbose output\n"
      "   -V,--version           display version and exit\n"
      "   -h,--help              display this help and exit\n";

  const char *OPTS = "vVh";

  const struct option LONG_OPTS[] = {{"verbose", no_argument, NULL, 'v'},
                                     {"version", no_argument, NULL, 'V'},
                                     {"help", no_argument, NULL, 'h'},
                                     {NULL, 0, NULL, 0}};

  int optc;
  while ((optc = getopt_long (argc, argv, OPTS, LONG_OPTS, NULL)) != EOF)
    {
      switch (optc)
        {
        case 'v':
          verbose = true;
          break;

        case 'V':
          fprintf (stdout, "hamming_code %d.%d\n", VERSION, SUBVERSION);
          fputs (PROGRAM_DESCRIPTION, stdout);
          return EXIT_SUCCESS;

        case 'h':
          fprintf (stdout, "%s%s%s", PROGRAM_DESCRIPTION, PROGRAM_USAGE,
                   OPTIONS_DESCRIPTION);
          return EXIT_SUCCESS;

        default: /* error message already generated */
          return EXIT_FAILURE;
        }
    }

  srand (time (NULL));

  binary_data_t *data = get_random_data ();
  set_controls (data);
  show_data_as_bytes (*data);

  add_an_error (data);
  // add_an_error (data); // could not be corrected in general
  show_data_as_bytes (*data);

  correct_an_error (data);
  show_data_as_bytes (*data);

  free_data (data);
  return EXIT_SUCCESS;
}