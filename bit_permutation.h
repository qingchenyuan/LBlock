#include <iostream>
#include <bitset>
using namespace std;

int main()
{
	unsigned int x=0x00;
	unsigned short temp[8], i;
	unsigned int t = 0x00, a[32];
	unsigned short P[32] = { 1, 9, 17, 25, 5, 13, 21, 29, 2, 10, 18, 26, 6, 14, 22, 30, 3, 11, 19, 27, 7, 15, 23, 31, 4, 12, 20, 28, 8, 16, 24, 32 };
	//cout << "the size is " << sizeof(unsigned short) << endl; //得到结果是2byte，也就是2x8=16bit
	temp[0] = 0x0;//0000
	temp[1] = 0x1;//0001
	temp[2] = 0x2;//0010
	temp[3] = 0x3;//0011
	temp[4] = 0x4;//0100
	temp[5] = 0x5;//0101
	temp[6] = 0x6;//0110
	temp[7] = 0x7;//0111
	//cout << temp[7] << endl;
	for (i = 0; i < 8; i++)
	{
		t ^= temp[i];
		if (i != 7)
			t <<= 4;
	}
	for (i = 0; i < 32; i++)
	{
		a[i] = (t >> (31 - i)) & 0x1;
	}
	for (i = 0; i < 32; i++)
	{
		x ^= a[P[i]];
		if (i != 31) x <<= 1;
	}
	cout << bitset<32>(t)<<endl;
	cout << bitset<32>(x);
	//cout << "the value of t is "<<t << endl;
	/*cout << bitset<32>(t)<< endl;
	cout << a[0];//和matlab不同，数组有a[0]
	cout << a[1];
	cout << a[2];
	cout << a[3];
	cout << a[4];
	cout << a[5];
	cout << a[6];
	cout << a[7];
	cout << a[8];
	cout << a[9];
	cout << a[10];
	cout << a[11];
	cout << a[12];
	cout << a[13];
	cout << a[14];
	cout << a[15];
	cout << a[16];
	cout << a[17];
	cout << a[18];
	cout << a[19];
	cout << a[20];
	cout << a[21];
	cout << a[22];
	cout << a[23];*/
	return 0;
}