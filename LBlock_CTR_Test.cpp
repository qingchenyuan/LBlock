#include "LBlock.h"
int main()
{
	int retval;
	uint16_t a = 0xf;//随便给一些master key的值做测试
	uint64_t b = 0xf;
	uint64_t c = 0xf;
	uint64_t d = 0xf;
	uint32_t keymod = 0;
	uint64_t IV = 1;
	uint32_t rounds=32;//还是通过32round，先把IP核造出来
	uint64_t text2[1024], ans2[1024];//1024行明文，1024行密文
	bool mod2 = 1;//给mod一个初值
	FILE *fp;
	for(int j=0; j<1024; j++)
	{
		text2[j] = j+1;//组成了1024行明文，不断+1
		ans2[j] = 0;//初始化密文，都为0
	}
	codec(IV,text2,a,b,c,d,keymod,rounds, mod2, ans2);//返回得到1024行密文
	fp=fopen("result.dat","w");//创建result的文件
	for(int j=0; j<1024; j++)
	{
		fprintf(fp, "%llx\n", ans2[j]);//%llx表示有符号64位16进制整数，把加密好的存进去
	}
	fclose(fp);
	//Compare the results file with the golden results
	retval = system("diff --brief -w result.dat result.golden_encrypt.dat");//和原来存好的加密好的数据（即密文）进行比较，这个是vivado HLS特有的，能够比较两个数据不同的个数
	if (retval != 0)
	{
		printf("Test failed  !!!\n");
		retval=1;//retval是return value的意思
	}
	else
	{
		printf("Test passed !\n");
	}


	mod2 = 0;
	for (int j=0; j<1024; j++)
		{
			text2[j] = *(ans2+j);//这里的目的是把密文放进text里，*（ans2）是指向数组的指针,数组名ans2是一个指向&balance[0]的指针
			ans2[j] = 0;//初始化，清空ans2数组
		}
	codec(IV,text2,a,b,c,d,keymod,rounds, mod2, ans2);
	fp=fopen("result.dat","w");
	for (int j=0; j<1024; j++)
	{
		fprintf(fp, "%llx\n", ans2[j]);
	}
	fclose(fp);
	//Compare the results file with the golden results
	retval = system("diff --brief -w result.dat result.golden_decrypt.dat");//和原来存好的解密好的数据（即明文）进行比较
	if (retval != 0)
	{
		printf("Test failed  !!!\n");
		retval=1;
	}
	else
	{
		printf("Test passed !\n");
	}

	// Return 0 if the test passes
	return retval;
}
