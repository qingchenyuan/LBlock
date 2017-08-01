#include "LBlock.h"
int main()
{
	int retval;
	uint16_t a = 0x0;
	uint64_t b = 0x0;
	uint64_t text2[512], ans2[512];
	bool mod2 = 1;
	FILE *fp;
	for(int j=0; j<512; j++)
	{
		text2[j] = j+1;
		ans2[j] = 0;
	}
	codec(text2, a, b, mod2, ans2);
	fp=fopen("result.dat","w");
	for(int j=0; j<512; j++)
	{
	fprintf(fp, "%llx\n", ans2[j]);
	}
	fclose(fp);
	//Compare the results file with the golden results
	retval = system("diff --brief -w result.dat result.golden_encrypt.dat");
		if (retval != 0) {
			printf("Test failed  !!!\n");
			retval=1;
		} else {
			printf("Test passed !\n");
	  }


	mod2 = 0;
	for (int j=0; j<512; j++)
		{
	text2[j] = *(ans2+j);
	ans2[j] = 0;
		}
	codec(text2, a, b, mod2, ans2);
	fp=fopen("result.dat","w");
	for (int j=0; j<512; j++)
	{
	fprintf(fp, "%llx\n", ans2[j]);
	}
	fclose(fp);
	//Compare the results file with the golden results
	retval = system("diff --brief -w result.dat result.golden_decrypt.dat");
		if (retval != 0) {
			printf("Test failed  !!!\n");
			retval=1;
		} else {
			printf("Test passed !\n");
	  }

	// Return 0 if the test passes
	return retval;
}
