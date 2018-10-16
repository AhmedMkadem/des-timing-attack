#des-timing-attack
Timing attack against a weak software implementation of DES encryption alrogrithm

#Usage
After cloning the repository, compile the program:

`make all`

Generate a time measurement database:

`./ta_acquisition 100000`

This will generate a file `ta.dat` with 100000 time measurements

Run the attack program that uses the timing measurments and the number of acquisitions:

`./ta ta.dat 100000`


#Proof-of-concept

**ta.c** is my timing attack program. It extracts the final round key given the timing measurment file and the number of experiments. 

#Lab files
des Data Encryption Standard algorithm implementation library

km library, to manage the partial knowledge about a DES secret key

ta_acquisition generates a ta.dat file containing a pair of cipher/exectime and a ta.key contianing the secret key and the 16 rounds keys

ta.dat containing the 100000 ciphertexts and timing measurements.
