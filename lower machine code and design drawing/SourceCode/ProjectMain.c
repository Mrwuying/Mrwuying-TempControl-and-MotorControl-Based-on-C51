#include<reg51.h>
#include<intrins.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define U8INT unsigned char 
#define U16INT unsigned int 
#define U32INT unsigned long int
#define Max_cmd_length 20

sbit MA=P1^0;
sbit MB=P1^1;
sbit PWM=P1^2;
sbit RS=P2^0;
sbit RW=P2^1;
sbit EN=P2^2;

sbit LED1_Hot_Waring=P2^4;
sbit LED2_Cold_Waring=P2^5;
sbit LED3_Power=P2^6;
sbit LED4_Light=P2^7;
sbit SoundPin=P3^7;


extern void LCD_Init();
extern void Display_String(U8INT row,U8INT col,U8INT *str);
extern void delay_ms(U16INT x);
extern U8INT Get_Temp16bits();
extern U8INT Temp_Bytes[];

U8INT Display_TempString[17];
volatile U8INT Received_Char_Buff[Max_cmd_length+1];
volatile U8INT Buff_Index=0;
volatile U8INT Received_Receipt=0;

extern void Wind_Open();
extern void Wind_Close();
extern void Light_Open();
extern void Light_Close();
volatile float temp=0.00f;//TempF���ڴ�temp��ȫ�����Ľ������Ϊ��;���������߳�ʼ�������������𶶶�
volatile float TempF=0.00f;
int flag=1;//1:��λ������Ȩ 0����λ������Ȩ

void Transmit_String(char *str)//��tempf����SBUF
{
	U8INT i=0;
	while(str[i])
	{
		SBUF=str[i];
		while(TI==0);
		TI=0;
		i++;
	}
}
void Received_DataSPort(void) interrupt 4//�����ж�
{ 
   static U8INT i=0;//�����жϻ��ε��ã���staticһ��֮��Ͳ��ḳֵ��
   if(RI==0)return ;//�д����жϴ�������RI����1
   RI=0;
   
   if(SBUF=='$') 
   {
	 i=0;
	 return;
   }
   if(SBUF==0x0D)//����λ������λ����������ǻس��ӻ���
   {
     return;
   }
   if(SBUF==0x0A)
   {
     Received_Receipt=1;
   }
   else
   {
	 Received_Char_Buff[i]=SBUF;
	 Received_Char_Buff[++i]='\0';
   }
   
  
}
void Serial_Port_Init()
{
  SCON=0x50;//0101 0000����ʽ1,8λ��ʱ���ɱ䲨���ʣ�ren=1�����������
  TMOD=0x20;//8λ�ļ�ʱ����
  PCON=0x80;//smod��1�������ʼӱ�
  TH1=0xFA;//���ݲ����������õģ�TH��λ��TL��λ8λ��ʱ��
  TL1=0xFA;
  EA=1;//���ж�ʹ��
  ES=1;//���������ж�
  TR1=1;//����ʱ��TR1
}
/*===================���ܺ�����tempf�ļ���,��ʾ�ʹ���SBUF=================================*/
void Tempf_TransToSBUF(float TempF)
{
	delay_ms(10);	
	sprintf(Display_TempString,"%5.1f",TempF);//\xDF\x43
	strcat(Display_TempString,"\r\n");
	Transmit_String(Display_TempString);
	strcat(Display_TempString,"\xDF\x43");
	Display_String(1,4,Display_TempString);
	delay_ms(10);
}


/*===================���ܺ�����tempf�ļ���ʹ���SBUF=================================*/

/*===================�����������ͣת=================================*/
void Right_Spin(float TempF)
{
	MA=1;//�ߵ�λ��ת
	MB=0;		 
	PWM=1;//����
	delay_ms(TempF-36);
	PWM=0;//����

}

void Left_Spin(float TempF)
{
	MA=0;//�͵�λ��ת
	MB=1;		 
	PWM=1;//����
	delay_ms(10-TempF);
	PWM=0;//����
	delay_ms(TempF-0);

}

void Stop_Spin()
{
	MA=0;
	MB=0;
}

/*===================�����������ͣת=================================*/
 
/*===================��λ���¼������壬���ã����£�����=================================*/
void Ring_Open()//����
{
	U8INT i=0;
	SoundPin=0;
	for(i=0;i<100;i++)
	{
		SoundPin=!SoundPin;delay_ms(1);
	}
	delay_ms(300);
}

void Temp_Up(float TempF)
{
	flag=0;
	Tempf_TransToSBUF(TempF);
	
}
void Temp_Down(float TempF)
{
	flag=0;
	Tempf_TransToSBUF(TempF);
	
}
/*===================��λ���¼������壬���ã����£�����=================================*/

/*===================�����¶ȿ���LED�;���===================*/
void TempControl(float TempF)
{
		delay_ms(10);
		if(TempF>100) 
		{
			TempF=100;
		}
	 	if(TempF>=10&&TempF<=36)//10~36 ����
		{
			LED1_Hot_Waring=0;
			LED2_Cold_Waring=0;
			Stop_Spin();
		}
		else if(TempF>36&&TempF<=60)//36~60 ��ת��������
		{
			LED1_Hot_Waring=0;
			LED2_Cold_Waring=0;
			Right_Spin(TempF);
		}
		else if(TempF>=0&&TempF<=10)//0~10 ��ת�������
		{
			LED1_Hot_Waring=0;
			LED2_Cold_Waring=1;
			Left_Spin(TempF);
		}
		else if(TempF>=60&&TempF<=100)//�¶���60�������屨�������ȵ�
		{
			 LED1_Hot_Waring=1;
   			 LED2_Cold_Waring=0;
			 Ring_Open();
		}
		
		delay_ms(10);
}
/*===================�����¶ȿ���LED�;���===================*/
void main()
{   
     
   Serial_Port_Init();
   LCD_Init();
   
   Display_String(0,0,"TempControlSystem");
   Display_String(1,0,"  Waiting.....");
   Get_Temp16bits();
   delay_ms(1000);
   Display_String(1,0,"              ");
   while(1)
   {
     if(Get_Temp16bits()&&flag==1)
	 {
	    temp=(int)((Temp_Bytes[1]<<8)|Temp_Bytes[0])*0.0625;
		TempF=temp;
		Tempf_TransToSBUF(TempF);//tempf�ļ���ʹ���SBUF
		TempControl(TempF);//�����¶ȸ�����Ӧ
	 }
	 if(Received_Receipt==1)
	 {
		Received_Receipt=0;
		
		if(strcmp(Received_Char_Buff,"RING_OPEN")==0)
		{
		  	 Ring_Open();//����
		}
		else if(strcmp(Received_Char_Buff,"TEMP_UP")==0)
		{
		  	 Temp_Up(++TempF);//+1��
			 TempControl(TempF);//�����¶ȸ�����Ӧ
		}
	    else if(strcmp(Received_Char_Buff,"TEMP_DOWN")==0)
		{
		  	 Temp_Down(--TempF);//-1��
			 TempControl(TempF);//�����¶ȸ�����Ӧ
		}
		else if(strcmp(Received_Char_Buff,"WIND_OPEN")==0)
		{
		  	 Wind_Open();
		}
		else if(strcmp(Received_Char_Buff,"WIND_CLOSE")==0)
		{
		  	 Wind_Close();
		}
		else if(strcmp(Received_Char_Buff,"LIGHT_OPEN")==0)
		{
		  	 Light_Open();
		}
		else if(strcmp(Received_Char_Buff,"LIGHT_CLOSE")==0)
		{
		  	 Light_Close();
		}
		if(flag==0)
		{
			if(strcmp(Received_Char_Buff,"RESET")==0)
			{	
			 	Ring_Open();		 
			 	flag=1;
			}
		}
				  
	  }
	  TempControl(TempF);//��λ���������Ȩ��Ҳ�ܾ���
   }
}