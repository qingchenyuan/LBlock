
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "platform.h"
#include "xcodec.h"
#include "xcodec_hw.h"
#include "xparameters.h"
#include "xtime_l.h"
#include "xil_printf.h"
#include "xstatus.h"
#include "xuartps.h"
#include "xuartps_hw.h"
#include "xbram.h"
#include "xsdps.h"
#include "ff.h"
#include "xil_cache.h"
#include "sleep.h"

XUartPs UartPs;
XUartPs_Config *UartPs_Config;
XCodec Codec;
XBram Bram;
XBram_Config *Bram_Config;
FATFS *fs;
FIL file1;
FIL file2;
FRESULT result;
TCHAR *Path = "0:/";


void init_LBlock();
void file_mangement(u32 mod);
void read_from_SD(u32 mod);
void encrypt_decrypt(uint16_t a,uint64_t b,uint32_t rounds,u32 mod);

//uint64_t mem_arr[1024];
u64 mem_arr[1024];// long long unsigned int, 64 bit,8 byte,1024行64bit，因为明文是64bit
// point to the BRAM Controllers(which controls the block rams)
u32 *BRAM1_mem = (u32 *)0x40000000;// 存储text的地方，0x后面8位，即8x4=32bit
u32 *BRAM2_mem = (u32 *)0x42000000;// 给了地址启示位，存储ans的地方
u64 mem_buff[10000];

// 给sd用的
char buff1[20];
char buff2[20];
unsigned int count1;//used for f_read
unsigned int count2;//used for f_write
int mem_count;
u64 text_in;
u64 text_out =0;
int ind;

int main()
{
	init_platform();
	uint16_t a = 0xf;//随便给一些master key的值做测试
	uint64_t b = 0xf;
	uint32_t rounds;
	u32 mod;
	xil_printf("choose mod (1-encryption,0-decryption): ");
	scanf("%ld",&mod);
	xil_printf("\n");
	xil_printf("choose the number of rounds: ");
	scanf("%ld",&rounds);
	xil_printf("\n");
	init_LBlock();
	file_mangement(mod);
	read_from_SD(mod);
	encrypt_decrypt(a,b,rounds,mod);
	f_close(&file1);
	f_close(&file2);
	cleanup_platform();
	return 0;
}

void init_LBlock()
{

	int Status = 0;
	UartPs_Config = XUartPs_LookupConfig(XPAR_XUARTPS_0_DEVICE_ID);
	if (UartPs_Config == NULL)
	{
		return XST_FAILURE;
	}
	Status = XUartPs_CfgInitialize(&UartPs, UartPs_Config,UartPs_Config->BaseAddress);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	Status = XUartPs_SelfTest(&UartPs);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	Status = XCodec_Initialize(&Codec,XPAR_CODEC_0_DEVICE_ID);
	if(Status != XST_SUCCESS)
	{
		xil_printf("Failed to initialized\n");
		return XST_FAILURE;
	}

	Bram_Config = XBram_LookupConfig(XPAR_AXI_BRAM_CTRL_0_DEVICE_ID);
	Status = XBram_CfgInitialize(&Bram, Bram_Config,Bram_Config->CtrlBaseAddress);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	Status = XBram_SelfTest(&Bram, 0);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	fs = malloc(sizeof(FATFS));
	result = f_mount(fs,Path,0);
	if (result != 0)
	{
		xil_printf("ERROR: f_mount failed %d\r\n", result);
		return XST_FAILURE;
	}
}
void file_mangement(u32 mod)
{
	//open and create file in SD card
	if(mod == 1)
	{
		result = f_open(&file1,"Text.txt",FA_READ);
			if(result != 0)
			{
				xil_printf("ERROR: f_open Text.txt failed, return %d\n",result);
				return XST_FAILURE;
			}
			result = f_open(&file2,"Cipher.txt",FA_CREATE_ALWAYS | FA_WRITE);
			if(result != 0)
			{
				xil_printf("ERROR: f_open Cipher.txt failed, return %d\n",result);
				return XST_FAILURE;
			}
	}
	if(mod == 0)
	{
		result = f_open(&file2,"Cipher.txt",FA_READ);
		if(result != 0)
		{
			xil_printf("ERROR: f_open Cipher.txt failed, return %d\n",result);
			return XST_FAILURE;
		}
		result = f_open(&file1,"Text.txt",FA_CREATE_ALWAYS | FA_WRITE);
		if(result != 0)
		{
			xil_printf("ERROR: f_open Text.txt failed, return %d\n",result);
			return XST_FAILURE;
		}
	}
}
void read_from_SD(u32 mod)
{
	//开始从SD卡里读取数据
	count1 =8;
	while(count1 == 8)
	{
		if(mod == 1)
		{
			result = f_read(&file1,buff1,8,&count1);
			if(result != 0)
			{
				xil_printf("ERROR: f_read Text.txt failed, return %d\n",result);
				return XST_FAILURE;
			}
		    text_in = 0;
		    if(count1 == 0) break;
		    xil_printf("the plaintext is ");
		    for(int i =0; i < count1;i++)
		    {
		    	xil_printf("%c",buff1[i]);
		    	text_in ^= (unsigned int) buff1[i];
		    	if(i != count1-1) text_in = text_in <<8;
		    }

		    if(count1 != 8) text_in = (text_in <<((8 - count1)*8));
		    mem_buff[ind] = text_in;
		    ind++;
		}
		if(mod == 0)
		{
			result = f_read(&file2,buff1,8,&count1);
			if(result != 0)
			{
				xil_printf("ERROR: f_read Cipher.txt failed, return %d\n",result);
				return XST_FAILURE;
			}
		    text_in = 0;
		    if(count1 == 0) break;
		    xil_printf("the ciphertext is ");
		    for(int i =0; i < count1;i++)
		    {
		    	xil_printf("%c",buff1[i]);
		    	text_in ^= (unsigned int) buff1[i];
		    	if(i != count1-1) text_in = text_in <<8;
		    }

		    if(count1 != 8) text_in = (text_in <<((8 - count1)*8));
		    mem_buff[ind] = text_in;
		    ind++;
		}
		xil_printf("\n");
	}
	xil_printf("\n");
}
void encrypt_decrypt(uint16_t a,uint64_t b,uint32_t rounds,u32 mod)
{
	//开始吧数据存到BRAM里
	ind=0;
	int ind_count = 0;
	while (mem_buff[ind] != 0)
	{
		if(mod == 1)
		{
			BRAM1_mem[ind_count] = ((mem_buff[ind]) & 0xffffffff);
			ind_count++;
			BRAM1_mem[ind_count] = (((mem_buff[ind]) >> 32) & 0xffffffff);
			ind_count++;
			xil_printf("the plaintext stored in BRAM is %x%x\n",BRAM1_mem[ind_count -1],BRAM1_mem[ind_count -2]);
			ind++;
			//if BRAM is full or finished
			if(((ind -1) != 0 && (ind-1) % 1023 == 0) || mem_buff[ind]==0)
			{
				if(XCodec_IsIdle(&Codec))
				{
					XCodec_Set_a(&Codec, a);
					XCodec_Set_b(&Codec, b);
					XCodec_Set_rounds(&Codec,rounds);
					XCodec_Set_mod_r(&Codec, mod);
					XCodec_Start(&Codec);
				}
				mem_count =0;
				xil_printf("\n");
				while(1)
				{
					if(XCodec_IsDone(&Codec))
					{
						//usleep(36);
						for(mem_count = 0;mem_count < 2048;mem_count +=2)
						{
							if(BRAM2_mem[mem_count + 1] != 0 || BRAM2_mem[mem_count] != 0)
							{
								xil_printf("the ciphertext(HEX) are: %x%x\n",BRAM2_mem[mem_count + 1],BRAM2_mem[mem_count]);
								text_out = BRAM2_mem[mem_count + 1];
								text_out = text_out << 32;
								text_out ^= BRAM2_mem[mem_count];
								if(text_out != 0)
								{
									xil_printf("the ciphertext(characters) are: ");
									for(int i = 0; i<=7;i++)
									{
										buff2[i] = text_out >> (56-(8*i)) & 0xff;
										xil_printf("%c",buff2[i]);
									}
									xil_printf("\n");
									result = f_write(&file2,buff2,8,&count2);
									if(result != 0)
									{
										xil_printf("ERROR: f_write Cipher.txt failed,return %d\n",result);
									}
								}
							}
						}
						break;
					}
				}
				for(int i = 0; i < 2048; i++)
				{
					BRAM1_mem[i] = 0;
					BRAM2_mem[i] = 0;
				}
				ind_count = 0;
			}
		}
		if(mod == 0)
		{
			BRAM1_mem[ind_count] = ((mem_buff[ind]) & 0xffffffff);
			ind_count++;
			BRAM1_mem[ind_count] = (((mem_buff[ind]) >> 32) & 0xffffffff);
			ind_count++;
			xil_printf("the ciphertext stored in BRAM is %x%x\n",BRAM1_mem[ind_count -1],BRAM1_mem[ind_count -2]);
			ind++;
			//if BRAM is full or finished
			if(((ind -1) != 0 && (ind-1) % 1023 == 0) || mem_buff[ind]==0)
			{
				if(XCodec_IsIdle(&Codec))
				{
					XCodec_Set_a(&Codec, a);
					XCodec_Set_b(&Codec, b);
					XCodec_Set_rounds(&Codec, rounds);
					XCodec_Set_mod_r(&Codec, mod);
					XCodec_Start(&Codec);
				}
				mem_count =0;
				xil_printf("\n");
				while(1)
				{
					if(XCodec_IsDone(&Codec))
					{
						//usleep(36);
						for(mem_count = 0;mem_count < 2048;mem_count +=2)
						{
							if(BRAM2_mem[mem_count + 1] != 0 || BRAM2_mem[mem_count] != 0)
							{
								xil_printf("the plaintext(HEX) are: %x%x\n",BRAM2_mem[mem_count + 1],BRAM2_mem[mem_count]);
								text_out = BRAM2_mem[mem_count + 1];
								text_out = text_out << 32;
								text_out ^= BRAM2_mem[mem_count];
								if(text_out != 0)
								{
									xil_printf("the plaintext(characters) are: ");
									for(int i = 0; i<=7;i++)
									{
										buff2[i] = text_out >> (56-(8*i)) & 0xff;
										xil_printf("%c",buff2[i]);
									}
									xil_printf("\n");
									result = f_write(&file1,buff2,8,&count2);
									if(result != 0)
									{
										xil_printf("ERROR: f_write Text.txt failed,return %d\n",result);
									}
								}
							}
						}
						break;
					}
				}
				for(int i = 0; i < 2048; i++)
				{
					BRAM1_mem[i] = 0;
					BRAM2_mem[i] = 0;
				}
				ind_count = 0;
			}
		}

	}


}


