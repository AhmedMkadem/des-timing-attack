/*
 * ahmed mkadem: mkadem@eurecom.fr
 *
 * Copyright (C) Telecom ParisTech
 * 
 * This file must be used under the terms of the CeCILL. This source
 * file is licensed as described in the file COPYING, which you should
 * have received as part of this distribution. The terms are also
 * available at:
 * http://www.cecill.info/licences/Licence_CeCILL_V1.1-US.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include <utils.h>
#include <des.h>
#include <km.h>

uint64_t *ct; /* Array of cipher texts. */
double *t; /* Array of timing measurements. */

/* Allocate arrays <ct> and <t> to store <n> cipher texts and timing
 * measurements. Open datafile <name> and store its content in global variables
 * <ct> and <t>. */
void
read_datafile(char *name, int n);

int
main(int argc, char **argv) {
  int n; /* Required number of experiments. */
  uint64_t r16l16; /* Output of last round, before final permutation. */
  uint64_t l16; /* Right half of r16l16. */
  uint64_t sbo; /* Output of SBoxes during last round. */
  double sum; /* Sum of timing measurements. */
  int i; /* Loop index. */
  uint64_t rk; /* Round key */
  double T4, T0, d; /*Time sum according to hamming weight and their difference*/
  double diff[8]; /*Difference in timing for each subkey*/
  uint64_t subkey[8]; /*subkeys*/
  int N4,N0; /*A counter for sbox hamming weights*/
  uint64_t K=0; /*Final round key*/
  /************************************************************************/
  /* Before doing anything else, check the correctness of the DES library */
  /************************************************************************/
  if(!des_check()) {
    ERROR(0, -1, "DES functional test failed");
  }

  /*************************************/
  /* Check arguments and read datafile */
  /*************************************/
  /* If invalid number of arguments (including program name), exit with error
   * message. */
  if(argc != 3) {
    ERROR(0, -1, "usage: ta <datafile> <nexp>\n");
  }
  /* Number of experiments to use is argument #2, convert it to integer and
   * store the result in variable n. */
  n = atoi(argv[2]);
  if(n < 1) { /* If invalid number of experiments. */
    ERROR(0, -1, "number of experiments to use (<nexp>) shall be greater than 1 (%d)", n);
  }
  /* Read data. Name of data file is argument #1. Number of experiments to use is n. */
  read_datafile(argv[1], n);

  /*****************************************************************************
   * Compute the Hamming weight of output of first (leftmost) SBox during last *
   * round, under the assumption that the last round key is all zeros.         *
   *****************************************************************************/
  rk = UINT64_C(0); /* All zeroes last round key. */
  /* Undoes the final permutation on cipher text of n-th experiment. */
  r16l16 = des_ip(ct[n - 1]);
  /* Extract right half (strange naming as in the DES standard). */
  l16 = des_right_half(r16l16);
  /* Compute output of SBoxes during last round of first experiment, assuming
   * the last round key is all zeros. */
  sbo = des_sboxes(des_e(l16) ^ rk); /* R15 = L16, K16 = rk */
  /* Compute and print Hamming weight of output of first SBox (mask the others). */
 // fprintf(stderr, "Hamming weight: %d\n", hamming_weight(sbo & UINT64_C(0xf0000000)));

  /************************************
   * Compute and print average timing *
   ************************************/
  sum = 0.0; /* Initializes the accumulator for the sum of timing measurements. */
  for(i = 0; i < n; i++) { /* For all n experiments. */
    sum = sum + t[i]; /* Accumulate timing measurements. */
  }
  /* Compute and print average timing measurements. */
 // fprintf(stderr, "Average timing: %f\n", sum / (double)(n));
  
  uint64_t mask=15;
  for(int box=0;box<8;box++){ 
  diff[box]=0.0;
  for(int key=0;key<64;key++){ /*All possible keys: 6 input bits of the entry of Sandbox box*/
  rk= (uint64_t)( key);
  rk=rk << 6*box; /*Placing the 6 bits at the right position of Sandbox box*/
  T0=T4=0.0;
  N0=N4=0;
  for(i=0;i<n;i++){ /*For all the ciphertexts*/
	r16l16 = des_ip(ct[i]);
        l16 = des_right_half(r16l16);
        sbo = des_sboxes(des_e(l16) ^ rk);
        if(hamming_weight(sbo & mask)==0){ T0+=t[i]; N0++; } /*permutation should be extremely fast*/
	if(hamming_weight(sbo & mask)==4){ T4+=t[i]; N4++; } /*permutation should be extremely slow*/
        
        

  }
 d=T4/(double)N4-T0/(double)N0;
 if(d>diff[box]){diff[box]=d; subkey[box]=rk;} /*The higher the difference, the more we are confident about the subkey*/
}
K+=subkey[box]; /*The final round key is the sum of all the subkeys*/
mask=mask<<4; /*update the mask for the next sandbox*/
}
  /************************
   * Print last round key *
   ************************/
  fprintf(stderr, "Last round key (hex):\n");
  printf("0x%012" PRIx64 "\n", K);

free(ct); /* Deallocate cipher texts */
free(t); /* Deallocate timings */
  return 0; /* Exits with "everything went fine" status. */
}

void
read_datafile(char *name, int n) {
  FILE *fp; /* File descriptor for the data file. */
  int i; /* Loop index */

  /* Open data file for reading, store file descriptor in variable fp. */
  fp = XFOPEN(name, "r");

  /* Allocates memory to store the cipher texts and timing measurements. Exit
   * with error message if memory allocation fails. */
  ct = XCALLOC(n, sizeof(uint64_t));
  t = XCALLOC(n, sizeof(double));

  /* Read the n experiments (cipher text and timing measurement). Store them in
   * the ct and t arrays. Exit with error message if read fails. */
  for(i = 0; i < n; i++) {
    if(fscanf(fp, "%" PRIx64 " %lf", &(ct[i]), &(t[i])) != 2) {
      ERROR(, -1, "cannot read cipher text and/or timing measurement");
    }
  }
}
