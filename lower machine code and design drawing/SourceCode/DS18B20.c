#include<reg51.h>
#include<intrins.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define U8INT unsigned char 
#define U16INT unsigned int 
#define U32INT unsigned long int
sbit DQ=P2^3;
U8INT Temp_Bytes[]={0x00,0x00};
void delayx(U8INT x)
{
   while(x--);
}

U8INT DS_Init()
{	U8INT DS_Status=0x00;
	DQ=1;delayx(8);	 
	DQ=0;delayx(90);	  
	DQ=1;delayx(8);
	DS_Status=DQ;
	delayx(10);
	return DS_Status;//successful 0  unsuccessful 1
 
}
U8INT  DS_Read8bits()
{	U8INT DS_Readdat=0x00;	
    U8INT i=0x00;
	for(i=0x01;i!=0x00;i<<=1)
	{
	  	DQ=0;_nop_();	  
	    DQ=1;_nop_();
		if(DQ)
		DS_Readdat=DS_Readdat|i;
		delayx(10);

	}
	return DS_Readdat;//successful 0  unsuccessful 1
 
}
void DS_Write8cmd(U8INT cmd  )
{		
    U8INT i=0x00;
	for(i=0x01;i!=0x00;i<<=1)
	{
	  	DQ=1;_nop_();	  
	    DQ=0;_nop_();
		DQ=cmd&0x01; 
		delayx(10);
		cmd=cmd>>1;
	}
 
}
U8INT  Get_Temp16bits()
{

   if(DS_Init()) return 0;
   else
   {
	   DS_Write8cmd(0xCC);	  
	   DS_Write8cmd(0x44);
	   DS_Init();
	   DS_Write8cmd(0xCC);	  
	   DS_Write8cmd(0xBE);
	   Temp_Bytes[0]=DS_Read8bits(); 
	   Temp_Bytes[1]=DS_Read8bits();
	   delayx(10);
	   return 1;
     
   }
}














