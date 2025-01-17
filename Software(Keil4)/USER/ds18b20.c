#include <ds18b20.h>
#include "GPIOLIKE51.h"
#include "display.h"

int temperature;									//全局变量 温度
int Set_temp;											//设置温度
char Fuhao;

void delay_us(u32 nCount)					//约9.5us
{
    uint x,y;
	for(x=nCount;x>0;x--)
		for(y=10;y>0;y--);
}

//??DS18B20
void DS18B20_Rst(void)     
{                 
    DS18B20_IO_OUT(); //SET PA0 OUTPUT
    DS18B20_DQ_OUT=0; //??DQ
    delay_us(750);    //??750us
    DS18B20_DQ_OUT=1; //DQ=1 
    delay_us(15);     //15US
}
//??DS18B20???

u8 DS18B20_Check(void)     
{   
    u8 retry=0;
    DS18B20_IO_IN();//SET PA0 INPUT  
    while (DS18B20_DQ_IN&&retry<200)
    {
        retry++;
        delay_us(1);
    };   
    if(retry>=200)return 1;
    else retry=0;
    while (!DS18B20_DQ_IN&&retry<240)
    {
        retry++;
        delay_us(1);
    };
    if(retry>=240)return 1;     
    return 0;
}
//?DS18B20?????
//???:1/0
u8 DS18B20_Read_Bit(void)            // read one bit
{
    u8 data;
    DS18B20_IO_OUT();//SET PA0 OUTPUT
    DS18B20_DQ_OUT=0; 
    delay_us(2);
    DS18B20_DQ_OUT=1; 
    DS18B20_IO_IN();//SET PA0 INPUT
    delay_us(12);
    if(DS18B20_DQ_IN)data=1;
    else data=0;     
    delay_us(50);           
    return data;
}
//?DS18B20??????
//???:?????
u8 DS18B20_Read_Byte(void)    // read one byte
{        
    u8 i,j,dat;
    dat=0;
    for (i=1;i<=8;i++) 
    {
        j=DS18B20_Read_Bit();
        dat=(j<<7)|(dat>>1);
    }                           
    return dat;
}
//??????DS18B20
//dat:??????
void DS18B20_Write_Byte(u8 dat)     
 {             
    u8 j;
    u8 testb;
    DS18B20_IO_OUT();//SET PA0 OUTPUT;
    for (j=1;j<=8;j++) 
    {
        testb=dat&0x01;
        dat=dat>>1;
        if (testb) 
        {
            DS18B20_DQ_OUT=0;// Write 1
            delay_us(2);                            
            DS18B20_DQ_OUT=1;
            delay_us(60);             
        }
        else 
        {
            DS18B20_DQ_OUT=0;// Write 0
            delay_us(60);             
            DS18B20_DQ_OUT=1;
            delay_us(2);                          
        }
    }
}
//??????
void DS18B20_Start(void)// ds1820 start convert
{                                          
    DS18B20_Rst();     
    DS18B20_Check();     
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0x44);// convert
} 
//???DS18B20?IO? DQ ????DS???
//??1:???
//??0:??         
u8 DS18B20_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);    //??PORTA??? 

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;               //PORTA0 ????
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;          
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_SetBits(GPIOB,GPIO_Pin_0);    //关闭所有LED灯

    DS18B20_Rst();

    return DS18B20_Check();
}  
//?ds18b20?????
//??:0.1C
//???:??? (-550~1250) 
int DS18B20_Get_Temp(void)
{
    int a,b;						//高八位，低八�
    DS18B20_Start ();                    // ds1820 start convert
    DS18B20_Rst();
    DS18B20_Check();     
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0xbe);// convert     
    a=DS18B20_Read_Byte(); // LSB   
    b=DS18B20_Read_Byte(); // MSB  

    
	b<<=8;							//把高位左移八位
	b=b|a;							//高八位与第八位进行或运算，得到总和
	if(b>=0)						//如果温度大于等于0
	{
		b=b*0.0625;					//直接乘以0.0625
		write_com(0x80+11);			//在LCD1602对应的位置不写内容
		write_date(0x20);
		if((b>0)&(b<100))
		Fuhao=1;					//正温度
	}
    else							//温度小于0
	{	
		b=~b+1;						//取反码再加一
		b=b*0.0625;					//再运算
		write_com(0x80+11);			//在LCD1602对应的位置写一个负号
		write_date(0x2d);
		if((b>0)&(b<100))
		Fuhao=0;					//负温度
	}
	return b;						//返回b，此时以表示温度
} 





