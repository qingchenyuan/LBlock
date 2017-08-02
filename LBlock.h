#ifndef _lblock_H_
#define _lblock_H_

#include <iostream> //include a standard library
#include <stdio.h> //include a standard library
#include <stdlib.h> //include a standard library
#include <stdint.h>

using namespace std; //used for the cin/cout/endl etc. commands

const unsigned short S[10][16]={
							   {14,9,15,0,13,4,10,11,1,2,8,3,7,6,12,5},
							   {4,11,14,9,15,13,0,10,7,12,5,6,2,8,1,3},
							   {1,14,7,12,15,13,0,6,11,5,9,3,2,4,8,10},
							   {7,6,8,11,0,15,3,14,9,10,12,13,5,2,4,1},
							   {14,5,15,0,7,2,12,13,1,8,4,9,11,10,6,3},
							   {2,13,11,12,15,14,0,9,7,10,6,3,1,8,4,5},
							   {11,9,4,14,0,15,10,13,6,12,5,7,3,8,1,2},
							   {13,10,15,0,14,4,9,11,2,1,8,3,7,5,12,6},
							   {8,7,14,5,15,13,0,6,11,12,9,10,2,4,1,3}, // The S-box values as presented in the report
							   {11,5,15,0,7,2,9,13,4,8,1,12,14,10,3,6}};
const unsigned short P[32] = { 0, 8, 16, 24, 4, 12, 20, 28, 1, 9, 17, 25, 5, 13, 21, 29, 2, 10, 18, 26, 6, 14, 22, 30, 3, 11, 19, 27, 7, 15, 23, 31 };//new bit permutation
const unsigned int rounds=32; //number of rounds

void roundkey(uint16_t a, uint64_t b,unsigned int rk[rounds]);
unsigned int S_Layer(unsigned int x);
unsigned int P_Layer(unsigned int x);
unsigned int F(unsigned int x, unsigned int k);
void swap(unsigned int *left, unsigned int *right);
void codec(uint64_t text[1024], uint16_t a, uint64_t b, bool mod, uint64_t ans[1024]);

#endif
