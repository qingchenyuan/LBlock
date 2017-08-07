
/******************************************************************************
 *
 * Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/
/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "xtime_l.h"
#include "platform.h"
#include "xil_printf.h"
#include "xcodec.h"
#include "xcodec_hw.h"
#include "xparameters.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xuartps.h"
#include "xuartps_hw.h"
#include "xil_exception.h"
#include "sleep.h"
#include "xsdps.h"
#include "ff.h"
#include "xil_cache.h"
#include "xbram.h"
XUartPs UartPs;
XUartPs_Config *UartPs_Config;
XCodec Codec;
FATFS FS_Instance;
FIL file1;//Text.txt
FIL file2;//Decrypt.txt
FIL file3;//Cipher.txt
FIL file4;//KeyEnc.txt
FIL file5;//KeyDec.txt
FRESULT result;
TCHAR *Path = "0:/";
XTime tStart, tEnd;
XBram_Config *Bram_Config;
XBram Bram;
char buff[20];
char buff2[20];
char buff3[20];
char encrypt_buff[20];
char decrypt_buff[20];
unsigned int count = 8;
unsigned int count2;
unsigned int count3;
unsigned int count4;
unsigned int count_sym;
int i;
int num_key;
unsigned int key_sel;
long exponent, base;
u64 temp;
u32 b_front, b_back, b_front_temp;
unsigned int text_out_front, text_out_back, text_out_front_temp;
unsigned int text_in_front, text_in_back, text_in_front_temp;
int num;
int error, error1, temp_c;
int c;
long long p, q, n, z, flag, public_key[100], private_key[100], mem[100], j,m[100], encrypt[100], decrypt[100];
unsigned int character[100];
char message[100];
long long pub_key_n, pub_key_e;
uint64_t mem_arr[1024];
u32* BRAM_mem = (u32 *) 0x40000000;
u32* BRAM2_mem = (u32 *) 0x42000000;
int mem_count = 0;
int ind;
int loop_var2;
u64 mem_buff[10000];
int loop_var;
int Is_Prime(long long number) 
{
       for (i = 2; i <= number - 1; i++) 
       {
              if (number % i == 0)
                     return 0;
       }
       return 1;
}
void Calc_Pub_Key();
long Calc_Priv_Key(long long x);
void RSA_Encrypt();
void RSA_Decrypt();
u64 text_in = 0;
u64 text_out = 0;
u16 a = 0;
u64 b = 0;
u32 mod = 0;
int main() 
{
       init_platform();
       int Status;
       XTime_GetTime(&tStart);
       UartPs_Config = XUartPs_LookupConfig(XPAR_XUARTPS_0_DEVICE_ID);
       if (NULL == UartPs_Config) 
       {
              return XST_FAILURE;
       }
       Status = XUartPs_CfgInitialize(&UartPs, UartPs_Config,
                     UartPs_Config->BaseAddress);
       if (Status != XST_SUCCESS) 
       {
              return XST_FAILURE;
       }
       Status = XUartPs_SelfTest(&UartPs);
       if (Status != XST_SUCCESS)
       {
              return XST_FAILURE;
       }
       Status = XCodec_Initialize(&Codec, XPAR_CODEC_0_DEVICE_ID);
       if (Status != XST_SUCCESS) 
       {
              return XST_FAILURE;
       }
       Bram_Config = XBram_LookupConfig(XPAR_AXI_BRAM_CTRL_0_DEVICE_ID);
       Status = XBram_CfgInitialize(&Bram, Bram_Config,
                     Bram_Config->CtrlBaseAddress);
       if (Status != XST_SUCCESS) 
       {
              return XST_FAILURE;
       }
       Status = XBram_SelfTest(&Bram, 0);
       if (Status != XST_SUCCESS) 
       {
              return XST_FAILURE;
       }
       result = f_mount(&FS_Instance, Path, 0);
       if (result != 0) 
       {
              xil_printf("ERROR: f_mount returned %d\r\n", result);
              return XST_FAILURE;
       }
       XTime_GetTime(&tEnd);
       while (1) 
       {
              xil_printf("\nPLEASE SELECT A MODE OF OPERATION (1 - ENCRYPTION, 0 - DECRYPTION)AND PRESS ENTER: ");
              c = 0;
              p = 0;
              q = 0;
              n = 0;
              z = 0;
              j = 0;
              while (1) 
              {
                     c = XUartPs_RecvByte(UartPs_Config->BaseAddress);
                     if (c == 13)
                           break;
                     if (c == 48)
                           mod = 0;
                     else if (c == 49)
                           mod = 1;
              }
			  //不同mode下不同的SD卡里文件打开和创建的的方式
              if (mod == 1) 
              {
                     result = f_open(&file1, "Text.txt", FA_READ);
                     if (result != 0) 
                     {
                           xil_printf("ERROR: f_read returned %d\r\n", result);
                           return XST_FAILURE;
                     }
                     result = f_open(&file3, "Cipher.txt",FA_CREATE_ALWAYS | FA_WRITE);
                     if (result != 0) 
                     {
                           xil_printf("ERROR: f_read returned %d\r\n", result);
                           return XST_FAILURE;
                     }
                     result = f_open(&file4, "KeyEnc.txt",FA_CREATE_ALWAYS | FA_WRITE);
                     if (result != 0) 
                     {
                           xil_printf("ERROR: f_read returned %d\r\n", result);
                           return XST_FAILURE;
                     }
              }
              if (mod == 0) 
              {
                     result = f_open(&file3, "Cipher.txt",FA_READ);
                     if (result != 0) 
                     {
                           xil_printf("ERROR: f_read returned %d\r\n", result);
                           return XST_FAILURE;
                     }
                     result = f_open(&file4, "KeyEnc.txt",
                     FA_READ);
                     if (result != 0)
                     {
                           xil_printf("ERROR: f_read returned %d\r\n", result);
                           return XST_FAILURE;
                     }
                     result = f_open(&file5, "KeyDec.txt",
                     FA_CREATE_ALWAYS | FA_WRITE);
                     if (result != 0) 
                     {
                           xil_printf("ERROR: f_read returned %d\r\n", result);
                           return XST_FAILURE;
                     }
                     result = f_open(&file2, "Decrypt.txt",
                     FA_CREATE_ALWAYS | FA_WRITE);
                     if (result != 0)
                     {
                           xil_printf("ERROR: f_read returned %d\r\n", result);
                           return XST_FAILURE;
                     }
              }
              //为了下面master key, RSA加密，解密key初始化一些变量
			  int init;
              for (init = 0; init < 100; init++) 
              {
                     public_key[init] = 0;
                     private_key[init] = 0;
                     mem[init] = 0;
                     m[init] = 0;
                     encrypt[init] = 0;
                     decrypt[init] = 0;
              }
			  //加密模式下，创建public key并用RSA来加密master key,创建master key分随机和手动输入
              if (mod == 1) 
              {
                     xil_printf(
                                  "\nWOULD YOU LIKE YO USE THE RANDOM KEY GENERATOR TO GENERATE A KEY [Y/N]? ");
                     c = 0;
                     while (1) 
                     {
                           c = XUartPs_RecvByte(UartPs_Config->BaseAddress);
                           if (c == 13)
                                  break;
                           else
                                  temp_c = c;
                     }
					 //随机输入master key
                     if (temp_c == 89 || temp_c == 121) 
                     {
                           srand(tEnd - tStart);
                           a = rand() % 65535;
                           xil_printf("\nTHE GENERATED KEY IS: %x", (unsigned int) a);
                           b ^= rand() % 4294967295;
                           b = b << 32;
                           b ^= rand() % 4294967295;
                           b_front = ((b & 0xFFFFFFFF00000000) >> 32);
                           b_back = b & 0x00000000FFFFFFFF;
                           b_front_temp = 0;
                           num = 0;
                           for (num = 1; num <= 8; num++)
                           {
                                  b_front_temp = b_front >> (32 - (4 * num));
                                  xil_printf("%x", b_front_temp & 0xF);
                           }
                           for (num = 1; num <= 8; num++)
                           {
                                  b_back = b >> (32 - (4 * num));
                                  xil_printf("%x", b_back & 0xF);
                           }
                           xil_printf("\n");
                     }
                     //手动输入master key
                     else 
                     {
                         xil_printf("\nPLEASE ENTER THE 80-BIT MASTERKEY IN HEXADECIMAL FORM (20 CHARACTERS TOTAL) AND PRESS ENTER: ");
                         a = 0;
                         b = 0;
                         c = 0;
                         count_sym = 0;
                         while (1) 
                         {
                                  c = XUartPs_RecvByte(UartPs_Config->BaseAddress);
                                  temp_c = c;
                                  if (c == 13)
                                         break;
                                  else if (c != 10)
                                         count_sym++;
                                  if (count_sym != 0 && count_sym <= 4)
                                         a = a << 4;
                                  else if (count_sym > 4 && count_sym <= 20)
                                         b = b << 4;
                                  else if (count_sym > 20) {
                                         xil_printf(
                                                       "\nERROR: YOU'VE ENTERED A MASTERKEY LARGER THAN 80 BITS!\n");
                                         break;
                                  }
                                  if (c == 48)
                                         c = 0;
                                  else if (c == 49)
                                         c = 1;
                                  else if (c == 50)
                                         c = 2;
                                  else if (c == 51)
                                         c = 3;
                                  else if (c == 52)
                                         c = 4;
                                  else if (c == 53)
                                         c = 5;
                                  else if (c == 54)
                                         c = 6;
                                  else if (c == 55)
                                         c = 7;
                                  else if (c == 56)
                                         c = 8;
                                  else if (c == 57)
                                         c = 9;
                                  else if (c == 65 || c == 97)
                                         c = 10;
                                  else if (c == 66 || c == 98)
                                         c = 11;
                                  else if (c == 67 || c == 99)
                                         c = 12;
                                  else if (c == 68 || c == 100)
                                         c = 13;
                                  else if (c == 69 || c == 101)
                                         c = 14;
                                  else if (c == 70 || c == 102)
                                         c = 15;
                                  else if (temp_c != 10) {
                                         xil_printf(
                                                       "\nYOU'VE ENTERED AN INVALID CHARACTER!\n PLEASE MAKE SURE YOU ENTER ONLY VALID HEXADECIMAL SYMBOLS!\n");
                                         break;
                                  }
                                  if (temp_c != 10 && count_sym <= 4) {
                                         a ^= c; //temp_c ignores line feed
                                  } else if (temp_c != 10 && count_sym > 4
                                                && count_sym <= 20) {
                                         b ^= c;
                                  }
                           }
                     }
					 //为了创建public key需要选择两个素数
                     unsigned long long key;
                     xil_printf("\nENTER FIRST PRIME NUMBER: ");
                     c = 0;
                     p = 0;
                     count_sym = 0;
                     while (1) {
                           c = XUartPs_RecvByte(UartPs_Config->BaseAddress);
                           if (c == 13)
                                  break;
                           else if (c != 10) {
                                  character[count_sym] = c - 48;
                                  count_sym++;
                           }
                     }
                     for (i = 0; i < count_sym; i++) {
                           exponent = count_sym - i - 1;
                           base = 1;
                           while (exponent != 0) {
                                  base = base * 10;
                                  exponent--;
                           }
                           p += base * character[i];
                     }
                     flag = Is_Prime(p);
                     if (flag == 0) {
                           xil_printf("\nWRONG INPUT\n");
                           exit(1);
                     }
                     xil_printf("\nENTER ANOTHER PRIME NUMBER: ");
                     c = 0;
                     q = 0;
                     count_sym = 0;
                     while (1) {
                           c = XUartPs_RecvByte(UartPs_Config->BaseAddress);
                           if (c == 13)
                                  break;
                           else if (c != 10) {
                                  character[count_sym] = c - 48;
                                  count_sym++;
                           }
                     }
                     for (i = 0; i < count_sym; i++) {
                           exponent = count_sym - i - 1;
                           base = 1;
                           while (exponent != 0) {
                                  base = base * 10;
                                  exponent--;
                           }
                           q += base * character[i];
                     }
                     flag = Is_Prime(q);
                     if (flag == 0 || p == q) {
                           xil_printf("\nWRONG INPUT\n");
                           exit(1);
                     }
                     int cnt = 0;
                     for (i = 0; i <= 19; i++) {
                           if (i <= 3) {
                                  key = a;
                                  m[cnt] = (key >> (12 - (i * 4))) & 0xF;
                                  cnt++;
                           } else {
                                  key = b;
                                  m[cnt] = (key >> (60 - ((i - 4) * 4))) & 0xF;
                                  cnt++;
                           }
                     }
                     n = p * q;
                     z = (p - 1) * (q - 1);
                     Calc_Pub_Key();
                     xil_printf("\nPOSSIBLE PUBLIC KEY VALUES:\n\n");
                     for (i = 0; i <= num_key - 1; i++) {
                           xil_printf("%u) N = %u ", i + 1, n);
                           if (((public_key[i] & 0xFFFFFFFF00000000) >> 32) != 0) {
                                  xil_printf("E = %u",
                                                (unsigned int) ((public_key[i] & 0xFFFFFFFF00000000)
                                                              >> 32));
                                  xil_printf("%u ",
                                                (unsigned int) (public_key[i] & 0x00000000FFFFFFFF));
                           } else {
                                  xil_printf("E = %u ",
                                                (unsigned int) (public_key[i] & 0x00000000FFFFFFFF));
                           }
                           if (((z & 0xFFFFFFFF00000000) >> 32) != 0) {
                                  xil_printf("Z = %u",
                                                (unsigned int) ((z & 0xFFFFFFFF00000000) >> 32));
                                  xil_printf("%u\n", (unsigned int) (z & 0x00000000FFFFFFFF));
                           } else {
                                  xil_printf("Z = %u\n",
                                                (unsigned int) (z & 0x00000000FFFFFFFF));
                           }
                     }
                     xil_printf("\nPLEASE SELECT A PUBLIC KEY: ");
                     c = 0;
                     count_sym = 0;
                     key_sel = 0;
                     while (1) {
                           c = XUartPs_RecvByte(UartPs_Config->BaseAddress);
                           if (c == 13)
                                  break;
                           else if (c != 10) {
                                  character[count_sym] = c - 48;
                                  count_sym++;
                           }
                     }
                     for (i = 0; i < count_sym; i++) {
                           exponent = count_sym - i - 1;
                           base = 1;
                           while (exponent != 0) {
                                  base = base * 10;
                                  exponent--;
                           }
                           key_sel += base * character[i];
                     }
                     RSA_Encrypt();
                     xil_printf("\nTHE ENCRYPTED KEY IS:\n");
                     for (i = 0; i < 20; i++) {
                           if (((encrypt[i] & 0xFFFFFFFF00000000) >> 32) != 0) {
                                  xil_printf("%u",
                                                (unsigned int) ((encrypt[i] & 0xFFFFFFFF00000000)
                                                              >> 32));
                                  xil_printf("%u",
                                                (unsigned int) (encrypt[i] & 0x00000000FFFFFFFF));
                           } else {
                                  xil_printf("%u",
                                                (unsigned int) (encrypt[i] & 0x00000000FFFFFFFF));
                           }
                     }
                     xil_printf("\n");
                     result = f_write(&file4, encrypt, 160, &count4);//把用RSA加密过的master key放到SD卡里
                     if (result != 0) {
                           xil_printf("\nERROR: f_write returned %d\r\n", result);
                     }
              }
			  //解密模式下，把加密过的master key,用RSA还原
              if (mod == 0) 
			 {
                     xil_printf("\nPLEASE ENTER THE PUBLIC KEY USED:\n");
                     xil_printf("\nN = ");
                     c = 0;
                     count_sym = 0;
                     pub_key_n = 0;
                     while (1) {
                           c = XUartPs_RecvByte(UartPs_Config->BaseAddress);
                           if (c == 13)
                                  break;
                           else if (c != 10) {
                                  character[count_sym] = c - 48;
                                  count_sym++;
                           }
                     }
                     for (i = 0; i < count_sym; i++) {
                           exponent = count_sym - i - 1;
                           base = 1;
                           while (exponent != 0) {
                                  base = base * 10;
                                  exponent--;
                           }
                           pub_key_n += base * character[i];
                     }
                     n = pub_key_n;
                     xil_printf("\nE = ");
                     c = 0;
                     count_sym = 0;
                     pub_key_e = 0;
                     while (1) {
                           c = XUartPs_RecvByte(UartPs_Config->BaseAddress);
                           if (c == 13)
                                  break;
                           else if (c != 10) {
                                  character[count_sym] = c - 48;
                                  count_sym++;
                           }
                     }
                     for (i = 0; i < count_sym; i++) {
                           exponent = count_sym - i - 1;
                           base = 1;
                           while (exponent != 0) {
                                  base = base * 10;
                                  exponent--;
                           }
                           pub_key_e += base * character[i];
                     }
                     xil_printf("\nZ = ");
                     c = 0;
                     count_sym = 0;
                     z = 0;
                     while (1) {
                           c = XUartPs_RecvByte(UartPs_Config->BaseAddress);
                           if (c == 13)
                                  break;
                           else if (c != 10) {
                                  character[count_sym] = c - 48;
                                  count_sym++;
                           }
                     }
                     for (i = 0; i < count_sym; i++) {
                           exponent = count_sym - i - 1;
                           base = 1;
                           while (exponent != 0) {
                                  base = base * 10;
                                  exponent--;
                           }
                           z += base * character[i];
                     }
                     result = f_read(&file4, encrypt, 160, &count4);
                     if (result != 0) {
                           xil_printf("ERROR: f_write returned %d\r\n", result);
                     }
                     encrypt[20] = -1;
                     RSA_Decrypt();
                     for (i = 0; i < 20; i++) {
                           if (((unsigned int) decrypt[i]) < 10) {
                                  decrypt_buff[i] = ((unsigned int) decrypt[i]) + 48;
                           } else {
                                  decrypt_buff[i] = ((unsigned int) decrypt[i]) + 87;
                           }
                     }
                     result = f_write(&file5, decrypt_buff, 20, &count4);
                     if (result != 0) {
                           xil_printf("ERROR: f_write returned %d\r\n", result);
                     }
                     a = 0;
                     b = 0;
                     for (i = 0; i <= 3; i++) {
                           a = a ^ decrypt[i];
                           if (i != 3)
                                  a = a << 4;
                     }
                     xil_printf("\nTHE DECRYPTED KEY IS: %x", (unsigned int) a);
                     for (i = 3; i < 20; i++) {
                           b = b ^ decrypt[i];
                           if (i != 19)
                                  b = b << 4;
                     }
                     b_front = (b >> 32) & 0xFFFFFFFF;
                     b_back = b & 0x00000000FFFFFFFF;
                     b_front_temp = 0;
                     num = 0;
                     for (num = 1; num <= 8; num++) {
                           b_front_temp = b_front >> (32 - (4 * num));
                           xil_printf("%x", b_front_temp & 0xF);
                     }
                     for (num = 1; num <= 8; num++) {
                           b_back = b >> (32 - (4 * num));
                           xil_printf("%x", b_back & 0xF);
                     }
                     xil_printf("\n");
              }
              //之前都是key方面的现在下面根据mod不同，对明文或密文进行预处理,放到mem_buff里
			  count = 8;
              ind = 0;
              while (count == 8) 
			 {
                     if (mod == 1) 
					 {
                           result = f_read(&file1, buff, 8, &count);
                           if (result != 0) 
						   {
                                  xil_printf("ERROR: f_read returned %d\r\n", result);
                                  return XST_FAILURE;
                           }
                           text_in = 0;
                           xil_printf("\nTHE READ TEXT IS: ");
                           for (i = 0; i <= count - 1; i++) {
                                  xil_printf("%c", buff[i]);
                                  text_in ^= (unsigned int) buff[i];
                                  if (i != count - 1)
                                         text_in = text_in << 8;
                           }
                           xil_printf("\n");
                           if (count != 8)
                                  text_in = (text_in << ((8 - count) * 8));
                           mem_buff[ind] = text_in;
                           ind++;
                           //xil_printf("THE READ TEXT IN HEXADECIMAL FORM IS: ");
                           //xil_printf("%x",(unsigned int)((text_in & 0xFFFFFFFF00000000) >> 32));
                           //xil_printf("%x\n",(unsigned int)(text_in & 0x00000000FFFFFFFF));
                           /*BRAM_mem[mem_count] = (text_in & 0xFFFFFFFF);
                            mem_count+=1;
                            BRAM_mem[mem_count] = ((text_in>>32) & 0xFFFFFFFF);
                            mem_count+=1;
                            xil_printf("THE READ TEXT IN HEXADECIMAL FORM IS: ");
                            xil_printf("%x%x\n", BRAM_mem[mem_count-1], BRAM_mem[mem_count-2]);
                            */
                     }
                     if (mod == 0) {
                           result = f_read(&file3, buff, 8, &count);
                           if (count == 0)
                                  break;
                           if (result != 0) {
                                  xil_printf("ERROR1: f_read returned %d\r\n", result);
                                  return XST_FAILURE;
                           }
                           text_in = 0;
                           xil_printf("\nTHE TEXT READ FROM CIPHER.TXT IS: ");
                           for (i = 0; i <= 7; i++) {
                                  xil_printf("%c", buff[i]);
                                  text_in ^= (unsigned int) buff[i];
                                  if (i != 7)
                                         text_in = text_in << 8;
                           }
                           xil_printf("\n");
                           mem_buff[ind] = text_in;
                           ind++;
                           /*
                            BRAM_mem[mem_count] = (mem_buff[ind] & 0xFFFFFFFF);
                            mem_count+=1;
                            BRAM_mem[mem_count] = ((mem_buff[ind]>>32) & 0xFFFFFFFF);
                            mem_count+=1;
                            if(BRAM_mem[mem_count-1] !=0 || BRAM_mem[mem_count-2] !=0)
                            {
                            xil_printf("THE TEXT READ FROM CIPHER.TXT IN HEXADECIMAL FORM IS: ");
                            text_in_front = (text_in >> 32) & 0xFFFFFFFF;
                            text_in_back = text_in & 0x00000000FFFFFFFF;
                            text_in_front_temp = 0;
                            num = 0;
                            for (num = 1; num<=8; num++)
                            {
                            text_in_front_temp = text_in_front>>(32-(4*num));
                            xil_printf("%x",text_in_front_temp & 0xF);
                            }
                            for (num = 1; num<=8; num++)
                            {
                            text_in_back = text_in>>(32-(4*num));
                            xil_printf("%x",text_in_back & 0xF);
                            }
                            }
                            xil_printf("\n");
                            */
                           /*    if(XCodec_IsIdle(&Codec))
                            {
                            XCodec_Set_a(&Codec, a);
                            XCodec_Set_b(&Codec, b);
                            XCodec_Set_mod_r(&Codec, mod);
                            XCodec_Start(&Codec);
                            }
                            if(XCodec_IsDone(&Codec));
                            {
                            }
                            xil_printf("THE DECRYPTED TEXT IN HEXADECIMAL FORM IS: ");
                            text_out_front = (text_out >> 32) & 0xFFFFFFFF;
                            text_out_back = text_out & 0x00000000FFFFFFFF;
                            text_out_front_temp = 0;
                            num = 0;
                            for (num = 1; num<=8; num++)
                            {
                            text_out_front_temp = text_out_front>>(32-(4*num));
                            xil_printf("%x",text_out_front_temp & 0xF);
                            }
                            for (num = 1; num<=8; num++)
                            {
                            text_out_back = text_out>>(32-(4*num));
                            xil_printf("%x",text_out_back & 0xF);
                            }
                            xil_printf("\n");
                            xil_printf("THE DECRYPTED TEXT IS: ");
                            for (i=0;i<=7;i++)
                            {
                            if((text_out>>(56-(8*i)) & 0xFF) == 0)
                            {
                            temp = i;
                            break;
                            }
                            else
                            {
                            temp = 8;
                            buff2[i] = text_out>>(56-(8*i)) & 0xFF;
                            xil_printf("%c", buff2[i]);
                            }
                            }
                            xil_printf("\n");
                            result = f_write(&file2,buff2,temp,&count2);
                            if (result !=0)
                            {
                            xil_printf("ERROR: f_write returned %d\r\n", result);
                            }
                            */
                     }
              }
              xil_printf("\n");
              ind = 0;
              int ind_count = 0;
			  //对mem_buff进行处理，根据不同的mode
              while (mem_buff[ind] != 0) {
                     	//加密模式下
						if (mod == 1) 
						{
                           //loop_var = 1;
                           //for(mem_count=0; mem_count<2048; mem_count++)
                           //{
                           BRAM_mem[ind_count] = ((mem_buff[ind]) & 0xFFFFFFFF);
                           ind_count++;
                           BRAM_mem[ind_count] = (((mem_buff[ind]) >> 32) & 0xFFFFFFFF);
                           ind_count++;
                           xil_printf(
                                         "THE TEXT WRITTEN TO MEMORY IN HEXADECIMAL FORM IS: ");
                           xil_printf("%x%x\n\n", BRAM_mem[ind_count - 1],
                                         BRAM_mem[ind_count - 2]);
                           ind++;
                           if ((ind - 1) != 0 && (ind - 1) % 1023 == 0) {
                                  //}
                                  //ind+=2047;
                                  //loop_var++;
                                  if (XCodec_IsIdle(&Codec)) {
                                         XCodec_Set_a(&Codec, a);
                                         XCodec_Set_b(&Codec, b);
                                         XCodec_Set_mod_r(&Codec, mod);
                                         XCodec_Start(&Codec);
                                  }
                                  mem_count = 0;
                                  while (1) {
                                         if (XCodec_IsDone(&Codec))
                                                ;
                                         {
                                                usleep(36);
                                                for (mem_count = 0; mem_count < 2048; mem_count +=
                                                              2) {
                                                       //if(BRAM2_mem[mem_count+1] !=0 || BRAM2_mem[mem_count]!=0)
                                                       //{
                                                       if (BRAM2_mem[mem_count + 1] != 0
                                                                     || BRAM2_mem[mem_count] != 0) {
                                                              xil_printf("THE ENCRYPTED TEXT IS: %x%x\n",
                                                                            BRAM2_mem[mem_count + 1],
                                                                            BRAM2_mem[mem_count]);
                                                              text_out = BRAM2_mem[mem_count + 1];
                                                              text_out = text_out << 32;
                                                              text_out ^= BRAM2_mem[mem_count];
                                                              //xil_printf("T = %x", (text_out>>32) & 0xFFFFFFFF);
                                                              //xil_printf("%x\n", text_out & 0xFFFFFFFF);
                                                              if (text_out != 0) {
                                                                     xil_printf(
                                                                                  "THE CHARACTERS WRITTEN INTO THE CIPHER.TXT FILE ARE: ");
                                                                     for (i = 0; i <= 7; i++) {
                                                                            buff3[i] = text_out
                                                                                         >> (56 - (8 * i)) & 0xFF;
                                                                            xil_printf("%c", buff3[i]);
                                                                     }
                                                                     xil_printf("\n\n");
                                                                     result = f_write(&file3, buff3, 8,
                                                                                  &count3);
                                                                     if (result != 0) {
                                                                            xil_printf(
                                                                                         "ERROR1: f_write returned %d\r\n",
                                                                                         result);
                                                                     }
                                                              }
                                                       }
                                                }
                                                break;
                                         }
                                  }
                                  loop_var2 = 0;
                                  for (loop_var2 = 0; loop_var2 < 2048; loop_var2++) {
                                         BRAM_mem[loop_var2] = 0;
                                         BRAM2_mem[loop_var2] = 0;
                                  }
                                  ind_count = 0;
                           }
                     }
                     //解密模式下
					if (mod == 0) {
                           //loop_var = 0;
                           //mem_count = 0;
                           //for(mem_count=0; mem_count<2048; mem_count++)
                           //{
                           /*BRAM_mem[mem_count] = ((mem_buff[loop_var*mem_count]) & 0xFFFFFFFF);
                            mem_count+=1;
                            BRAM_mem[mem_count] = (((mem_buff[loop_var*mem_count])>>32) & 0xFFFFFFFF);
                            mem_count+=1;
                            xil_printf("THE TEXT READ FROM CIPHER.TXT IN HEXADECIMAL FORM IS: ");
                            xil_printf("%x%x\n", BRAM_mem[mem_count-1], BRAM_mem[mem_count-2]);
                            }
                            */
                           BRAM_mem[ind_count] = ((mem_buff[ind]) & 0xFFFFFFFF);
                           ind_count++;
                           BRAM_mem[ind_count] = (((mem_buff[ind]) >> 32) & 0xFFFFFFFF);
                           ind_count++;
                           xil_printf(
                                         "THE TEXT WRITTEN TO MEMORY IN HEXADECIMAL FORM IS: ");
                           xil_printf("%x%x\n\n", BRAM_mem[ind_count - 1],
                                         BRAM_mem[ind_count - 2]);
                           ind++;
                           if ((ind - 1) != 0 && (ind - 1) % 1023 == 0) {
                                  if (XCodec_IsIdle(&Codec)) {
                                         XCodec_Set_a(&Codec, a);
                                         XCodec_Set_b(&Codec, b);
                                         XCodec_Set_mod_r(&Codec, mod);
                                         XCodec_Start(&Codec);
                                  }
                                  mem_count = 0;
                                  while (1) {
                                         if (XCodec_IsDone(&Codec))
                                                ;
                                         {
                                                usleep(36);
                                                for (mem_count = 0; mem_count < 2048; mem_count +=
                                                              2) {
                                                       if (BRAM2_mem[mem_count + 1] != 0
                                                                     || BRAM2_mem[mem_count] != 0) {
                                                              xil_printf(
                                                                           "THE DECRYPTED TEXT IN HEXADECIMAL FORM IS: %x%x\n",
                                                                            BRAM2_mem[mem_count + 1],
                                                                            BRAM2_mem[mem_count]);
                                                              text_out = BRAM2_mem[mem_count + 1];
                                                              text_out = text_out << 32;
                                                              text_out ^= BRAM2_mem[mem_count];
                                                              xil_printf("THE DECRYPTED TEXT IS: ");
                                                              for (i = 0; i <= 7; i++) {
                                                                     if ((text_out >> (56 - (8 * i)) & 0xFF)
                                                                                  == 0) {
                                                                           temp = i;
                                                                           break;
                                                                     } else {
                                                                           temp = 8;
                                                                            buff2[i] = text_out
                                                                                         >> (56 - (8 * i)) & 0xFF;
                                                                            xil_printf("%c", buff2[i]);
                                                                     }
                                                              }
                                                              xil_printf("\n\n");
                                                              result = f_write(&file2, buff2, temp,
                                                                            &count2);
                                                              if (result != 0) {
                                                                     xil_printf(
                                                                                  "ERROR: f_write returned %d\r\n",
                                                                                  result);
                                                              }
                                                       }
                                                }
                                                break;
                                         }
                                  }
                                  for (loop_var2 = 0; loop_var2 < 2048; loop_var2++) {
                                         BRAM_mem[loop_var2] = 0;
                                         BRAM2_mem[loop_var2] = 0;
                                  }
                                  ind_count = 0;
                           }
                     }
              }
			  //
              if (mod == 1) {
                     if (XCodec_IsIdle(&Codec)) {
                           XCodec_Set_a(&Codec, a);
                           XCodec_Set_b(&Codec, b);
                           XCodec_Set_mod_r(&Codec, mod);
                           XCodec_Start(&Codec);
                     }
                     mem_count = 0;
                     while (1) {
                           if (XCodec_IsDone(&Codec))
                                  ;
                           {
                                  usleep(36);
                                  for (mem_count = 0; mem_count < 2048; mem_count += 2) {
                                         //if(BRAM2_mem[mem_count+1] !=0 || BRAM2_mem[mem_count]!=0)
                                         //{
                                         if (BRAM2_mem[mem_count + 1] != 0
                                                       || BRAM2_mem[mem_count] != 0) {
                                                xil_printf("THE ENCRYPTED TEXT IS: %x%x\n",
                                                              BRAM2_mem[mem_count + 1],
                                                              BRAM2_mem[mem_count]);
                                                text_out = BRAM2_mem[mem_count + 1];
                                                text_out = text_out << 32;
                                                text_out ^= BRAM2_mem[mem_count];
                                                //xil_printf("T = %x", (text_out>>32) & 0xFFFFFFFF);
                                                //xil_printf("%x\n", text_out & 0xFFFFFFFF);
                                                if (text_out != 0) {
                                                       xil_printf(
                                                                     "THE CHARACTERS WRITTEN INTO THE CIPHER.TXT FILE ARE: ");
                                                       for (i = 0; i <= 7; i++) {
                                                              buff3[i] = text_out >> (56 - (8 * i))
                                                                           & 0xFF;
                                                              xil_printf("%c", buff3[i]);
                                                       }
                                                       xil_printf("\n\n");
                                                       result = f_write(&file3, buff3, 8, &count3);
                                                       if (result != 0) {
                                                              xil_printf(
                                                                            "ERROR1: f_write returned %d\r\n",
                                                                            result);
                                                       }
                                                }
                                         }
                                  }
                                  break;
                           }
                     }
              }
              //loop_var = 0;
              //mem_count = 0;
              //for(mem_count=0; mem_count<2048; mem_count++)
              //{
              /*BRAM_mem[mem_count] = ((mem_buff[loop_var*mem_count]) & 0xFFFFFFFF);
               mem_count+=1;
               BRAM_mem[mem_count] = (((mem_buff[loop_var*mem_count])>>32) & 0xFFFFFFFF);
               mem_count+=1;
               xil_printf("THE TEXT READ FROM CIPHER.TXT IN HEXADECIMAL FORM IS: ");
               xil_printf("%x%x\n", BRAM_mem[mem_count-1], BRAM_mem[mem_count-2]);
               }
               */
              if (mod == 0) {
                     if (XCodec_IsIdle(&Codec)) {
                           XCodec_Set_a(&Codec, a);
                           XCodec_Set_b(&Codec, b);
                           XCodec_Set_mod_r(&Codec, mod);
                           XCodec_Start(&Codec);
                     }
                     mem_count = 0;
                     while (1) {
                           if (XCodec_IsDone(&Codec))
                                  ;
                           {
                                  usleep(36);
                                  for (mem_count = 0; mem_count < 2048; mem_count += 2) {
                                         if (BRAM2_mem[mem_count + 1] != 0
                                                       || BRAM2_mem[mem_count] != 0) {
                                                xil_printf(
                                                              "THE DECRYPTED TEXT IN HEXADECIMAL FORM IS: %x%x\n",
                                                              BRAM2_mem[mem_count + 1],
                                                              BRAM2_mem[mem_count]);
                                                text_out = BRAM2_mem[mem_count + 1];
                                                text_out = text_out << 32;
                                                text_out ^= BRAM2_mem[mem_count];
                                                xil_printf("THE DECRYPTED TEXT IS: ");
                                                for (i = 0; i <= 7; i++) {
                                                       if ((text_out >> (56 - (8 * i)) & 0xFF) == 0) {
                                                              temp = i;
                                                              break;
                                                       } else {
                                                              temp = 8;
                                                              buff2[i] = text_out >> (56 - (8 * i))
                                                                           & 0xFF;
                                                              xil_printf("%c", buff2[i]);
                                                       }
                                                }
                                                xil_printf("\n\n");
                                                result = f_write(&file2, buff2, temp, &count2);
                                                if (result != 0) {
                                                       xil_printf("ERROR: f_write returned %d\r\n",
                                                                     result);
                                                }
                                         }
                                  }
                                  break;
                           }
                     }
              }
              f_close(&file1);
              f_close(&file2);
              f_close(&file3);
              f_close(&file4);
              f_close(&file5);
              loop_var = 0;
              for (loop_var = 0; loop_var < 2048; loop_var++) {
                     BRAM_mem[loop_var] = 0;
                     BRAM2_mem[loop_var] = 0;
              }
              xil_printf("\nWOULD YOU LIKE TO EXIT [Y/N]? ");
              while (1) {
                     c = XUartPs_RecvByte(UartPs_Config->BaseAddress);
                     if (c == 13)
                           break;
                     else
                           temp_c = c;
              }
              if (temp_c == 89 || temp_c == 121) {
                     xil_printf("\nTHANK YOU FOR USING THE HARDWARE ACCELERATOR! :)");
                     break;
              }
              if (temp_c == 78 || temp_c == 110)
                     xil_printf("\nYOU CHOSE TO CONTINUE!\n");
              else {
                     xil_printf("\nYOUR RESPONSE WASN'T RECOGNISED BY THE SYSTEM!\n");
                     break;
              }
       }
       cleanup_platform();
       return 0;
}
void Calc_Pub_Key() {
       num_key = 0;
       int k = 0;
       long long priv_key_temp;
       int o;
       for (o = 2; o < z; o++) {
              if (z % o == 0)
                     continue;
              flag = Is_Prime(o);
              if (flag == 1 && o != p && o != q) {
                     public_key[k] = o;
                     priv_key_temp = Calc_Priv_Key(public_key[k]);
                     if (priv_key_temp > 0) {
                           private_key[k] = priv_key_temp;
                           k++;
                           num_key++;
                     }
                     if (k == 99)
                           break;
              }
       }
}
long Calc_Priv_Key(long long x) {
       long long k = 1;
       while (1) {
              k = k + z;
              if (k % x == 0)
                     return (k / x);
       }
}
void RSA_Encrypt() {
       long long plain_text, cipher, k, length, key = public_key[key_sel - 1];
       i = 0;
       length = 20;
       while (i != length) {
              plain_text = m[i];
              k = 1;
              j = 0;
              for (j = 0; j < key; j++) {
                     k = k * plain_text;
                     k = k % n;
              }
              mem[i] = k;
              cipher = k;
              encrypt[i] = cipher;
              i++;
       }
       encrypt[i] = -1;
}
void RSA_Decrypt() {
       long long plain_text, cipher, key, k;
       key = Calc_Priv_Key(pub_key_e);
       i = 0;
       while (encrypt[i] != -1) {
              cipher = encrypt[i];
              k = 1;
              for (j = 0; j < key; j++) {
                     k = k * cipher;
                     k = k % n;
              }
              plain_text = k;
              decrypt[i] = plain_text;
              i++;
       }
       decrypt[i] = -1;
}
