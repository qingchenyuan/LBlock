#include <time.h>
#include "LBlock.h"
#include <stdio.h>

using namespace std; //used for the cin/cout/endl etc. commands

void roundkey_less(uint16_t a, uint64_t b,uint32_t rounds,unsigned int rk[32] )
{ //Key Scheduling
	unsigned int i;
	uint16_t temp_a,temp1,temp2;
	unsigned long long temp_5; //variables used in the function
	temp1 = (b>>48)&0xFFFF; //take the 16 most significant bits of b
	rk[0]=(a<<16)^ temp1; //output first round subkey
	for(i=1;i<rounds;i++)
	{
		temp1 = (b>>51) & 0x1FFF; //take the 13 most significant bits of b
		temp2 = (b>>35) & 0xFFFF; //take the 16 bits after the 13 most significant bits of b
		temp_a=a;
		a = a << 29; //shift a by 29 positions to the left
		b = b << 29; //shift b by 29 positions to the left
		a = temp2; //implement cyclic shift
		b^=(temp_a << 13) ^ temp1; //implement cyclic shift
		temp1=(a>>12)&0xF;//take the 4 bits, corresponding to K79-K76 in the paper
		temp2=(a>>8)&0xF; //take the 4 bits, corresponding to K75-K72
		temp1=S[9][temp1];//take the corresponding value from the S-box
		temp2=S[8][temp2];//take the corresponding value from the S-box
		a=(temp1<<12)^(temp2<<8)^(a&0x00FF);//update the value of a with the corresponding vallues from the S-boxes at the front 8 bits and the original 8 bits at the back
		temp_5 = b>>46&0x1F; //take k50-k46 from the masterkey
		temp_5^=i; //XOR with I
		b=(b&0xFFF8000000000000)^(temp_5<<46)^(b&0x3FFFFFFFFFFF); //update the value of b
		temp1 = (b>>48)&0xFFFF;//take the 16 most significant bits of b
		rk[i]=(a<<16)^ temp1;//Output the leftmost 32 bits of the current master key as a round subkey
	}
}
void roundkey_more(uint64_t c, uint64_t d,uint32_t rounds,unsigned int rk[32] )
{ //Key Scheduling
	uint64_t temp_c,temp_d,temp1,temp2,temp3;
	unsigned int i;
	rk[0] = c>>32;
	for(i=1;i<rounds;i++)
	{
		//K<<<29
		temp_c = c>>35;
		temp_d = d>>29;
		c = c<<29;
		d = d>>35;
		c = c^d;
		d = (temp_d<<29)^temp_c;
		//SS替换
		temp1 = (c>>60)&0xF;
		temp2 = (c>>56)&0xF;
		temp1 = S[9][temp1];
		temp2 = S[8][temp2];
		c = (temp1<<60)^(temp2<<56)^(c&0xffffffffffffff);
		// XOR  round counter
		temp3 = d>>60;
		temp3 ^= i;
		d = (temp3<<60)^(d>>4);
		// left-most 32 bit
		rk[i] = c>>32;
	}
}

unsigned int S_Layer(unsigned int x)
{
	unsigned int temp=0x0;
	int i;
	for(i=0;i<=7;i++)
	{
		temp^=S[7-i][(x>>(28-4*i)&0xF)];//starting from the MSB concatenate the corresponding value from the S-Box at the last 4 bits of temp
		if (i!=7) temp<<=4; //add four zeroes at the end to support the XOR operation above unless i has reached 7, which signifies the end
	}
   // temp^=S[7-i][x&0xF]; //Alternatively use this instead of the if expression at the end of the loop
	return temp; //return the calculated value
}

//new bit permutation
unsigned int P_Layer(unsigned int x)
{
	unsigned short temp[32],i;
	unsigned int t=0x0;
	for(i=0;i<32;i++)
	temp[i]=(x>>(31-i))&0x1;
	for(i=0;i<32;i++)
	{
		t^=temp[P[i]];
		if(i!=31) t<<=1;
	}
	return t;
}
unsigned int F(unsigned int x, unsigned int k)//Rounding function F
{
	x^=k; //X XOR K
	x=S_Layer(x); //Confusion function with arguement X^K calculated above
	x=P_Layer(x); //Diffusion function with arguement Confusion function from X^K calculated above
	return x; //return the calculated value
}
void swap(unsigned int *left, unsigned int *right)//A function used to swap the values of the two parts of the plaintext when encrypting; use pointers to reflect the change in the main program
{
	unsigned int temp;// temp is used not to lose the value of left
	temp=(*left); //get the value of left
	(*left)=(*right); //pass the value of right to left
	(*right)=temp; //pass the value of temp (left) to right
}

void codec(uint64_t text[1024], uint16_t a, uint64_t b,uint64_t c,uint64_t d,uint32_t keymod,uint32_t rounds, bool mod, uint64_t ans[1024])
{
	#pragma HLS interface bram port=ans //a command, which allow the AXI4-Lite (s_axilite) interface to be introduced
	#pragma HLS RESOURCE variable=ans core=RAM_1P_BRAM
	#pragma HLS interface bram port=text //a command, which allow the AXI4-Lite (s_axilite) interface to be introduced
	#pragma HLS RESOURCE variable=text core=RAM_1P_BRAM
	#pragma HLS interface s_axilite port=return bundle=s_axi //a command, which allow the AXI4-Lite (s_axilite) interface to be introduced
	#pragma HLS interface s_axilite port=a bundle=s_axi //a command, which allow the AXI4-Lite (s_axilite) interface to be introduced
	#pragma HLS interface s_axilite port=b bundle=s_axi //a command, which allow the AXI4-Lite (s_axilite) interface to be introduced
	#pragma HLS interface s_axilite port=mod bundle=s_axi //a command, which allow the AXI4-Lite (s_axilite) interface to be introduced
 	#pragma HLS interface s_axilite port=rounds bundle=s_axi
	#pragma HLS interface s_axilite port=c bundle=s_axi
	#pragma HLS interface s_axilite port=d bundle=s_axi
	#pragma HLS interface s_axilite port=keymod bundle=s_axi

	unsigned int i; //a simple variable used for control of the loop below
	unsigned int left, right; //2x32-bit variables used to store the split into two 32-bit parts plain text
	unsigned int rk1[32]; // an array used to store the roundkey

	for(int j=0; j<1024; j++)
	{
		if(text[j] !=0 )//提取存在BRAM的1024行64bit的明文，1024x64,64是因为明文数量，每次取一行64bit，一共1024行
		{
			left = (text[j]>>32) & 0xFFFFFFFF;
			right = text[j] & 0xFFFFFFFF;
			if(keymod == 0)
			{
				roundkey_less(a,b,rounds,rk1); //calculating the roundkey
			}
			if(keymod == 1)
			{
				roundkey_more(c,d,rounds,rk1);
			}



			if (mod == 1)
			{
				for(i=0;i<rounds;i++)
				{
					right=(right<<20)^(right>>12); //left rotation 20 bit
					right^=F(left,rk1[i]); //calculate the round function F
					if (i!=rounds-1) swap(&left,&right); //swap the two parts of the text as in the picture in the paper; pass the addresses of left and right as arguments
				}
				ans[j]=left; //store the value of the right part of the ciphertext
				ans[j]<<=32; //shift it by 32 bits to the left
				ans[j]^=right; //store the value of the left part at the end of the ciphertext to get the correct answer since the loop above performed one for swap than needed
			}
			else
			{
				for(int i=0;i<rounds;i++)
				{
					right^=F(left,rk1[rounds-1-i]); //calculate the round function F
					right=(right>>20)^(right<<12);//right rotation 20bit
					swap(&left,&right); //swap the two parts of the text as in the picture in the paper; pass the addresses of left and right as arguments
				}
				ans[j]=right; //store the value of the right part of the ciphertext
				ans[j]<<=32; //shift it by 32 bits to the left
				ans[j]^=left; //store the value of the left part at the end of the ciphertext to get the correct answer since the loop above performed one for swap than needed
			}
		}
	}
}

