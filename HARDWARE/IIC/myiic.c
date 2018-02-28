#include "myiic.h"
#include "delay.h"

#include "gpio_define.h"


//IO��������
void SDA_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	GPIO_InitStructure.GPIO_Pin = gpio_map[EEP_SDA].GPIO_Pin_X;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_Init(gpio_map[EEP_SDA].GPIOX, &GPIO_InitStructure);
	GPIO_SetBits(gpio_map[EEP_SDA].GPIOX, gpio_map[EEP_SDA].GPIO_Pin_X);
}
void SDA_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	GPIO_InitStructure.GPIO_Pin = gpio_map[EEP_SDA].GPIO_Pin_X;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpio_map[EEP_SDA].GPIOX, &GPIO_InitStructure);
	GPIO_SetBits(gpio_map[EEP_SDA].GPIOX, gpio_map[EEP_SDA].GPIO_Pin_X);
}

//IO��������	 
static void IIC_SCL(u8 status)
{
	if(status) 
		GPIO_SetBits(gpio_map[EEP_SCL].GPIOX, gpio_map[EEP_SCL].GPIO_Pin_X);
	else
		GPIO_ResetBits(gpio_map[EEP_SCL].GPIOX, gpio_map[EEP_SCL].GPIO_Pin_X); 
}
static void IIC_SDA(u8 status)
{
	if(status) 
		GPIO_SetBits(gpio_map[EEP_SDA].GPIOX, gpio_map[EEP_SDA].GPIO_Pin_X);
	else
		GPIO_ResetBits(gpio_map[EEP_SDA].GPIOX, gpio_map[EEP_SDA].GPIO_Pin_X); 
}	
u8 READ_SDA(void)
{
	u8 status;
	status = GPIO_ReadInputDataBit(gpio_map[EEP_SDA].GPIOX, gpio_map[EEP_SDA].GPIO_Pin_X);  		  
	return status;
}

//��ʼ��IIC
void IIC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(gpio_map[EEP_SCL].RCC_APB2Periph_X, ENABLE);//ʹ��SCL
	GPIO_InitStructure.GPIO_Pin = gpio_map[EEP_SCL].GPIO_Pin_X ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpio_map[EEP_SCL].GPIOX, &GPIO_InitStructure);
	GPIO_SetBits(gpio_map[EEP_SCL].GPIOX, gpio_map[EEP_SCL].GPIO_Pin_X );
	
	RCC_APB2PeriphClockCmd(gpio_map[EEP_SDA].RCC_APB2Periph_X, ENABLE);//ʹ��SDL 
	GPIO_InitStructure.GPIO_Pin =  gpio_map[EEP_SDA].GPIO_Pin_X;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpio_map[EEP_SDA].GPIOX, &GPIO_InitStructure);
	GPIO_SetBits(gpio_map[EEP_SDA].GPIOX,  gpio_map[EEP_SDA].GPIO_Pin_X);
}

//����IIC��ʼ�ź�
void IIC_Start(void)
{
	SDA_OUT();		//sda�����
	IIC_SDA(1);	  	  
	IIC_SCL(1);
	delay_us(4);
 	IIC_SDA(0);	//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL(0);	//ǯסI2C���ߣ�׼�����ͻ�������� 
}
	  
//����IICֹͣ�ź�
void IIC_Stop(void)
{
	SDA_OUT();		//sda�����
	IIC_SCL(0);
	IIC_SDA(0);	//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL(1); 
	IIC_SDA(1);	//����I2C���߽����ź�
	delay_us(4);							   	
}

//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 IIC_Wait_Ack(void)
{
	u16 ucErrTime = 0;
	IIC_SDA(1);
	SDA_IN();		//SDA����Ϊ����  
	
	delay_us(1);	   
	IIC_SCL(1);
	delay_us(1);	 
	while(READ_SDA())
	{
		ucErrTime++;
		if(ucErrTime > 250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL(0);	//ʱ�����0 	   
	return 0;  
}
 
//����ACKӦ��
void IIC_Ack(void)
{
	IIC_SCL(0);
	SDA_OUT();
	IIC_SDA(0);
	delay_us(2);
	IIC_SCL(1);
	delay_us(2);
	IIC_SCL(0);
}

//������ACKӦ��		    
void IIC_NAck(void)
{
	IIC_SCL(0);
	SDA_OUT();
	IIC_SDA(1);
	delay_us(2);
	IIC_SCL(1);
	delay_us(2);
	IIC_SCL(0);
}
					 				     
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    IIC_SCL(0);		//����ʱ�ӿ�ʼ���ݴ���
    for(t = 0; t < 8; t++)
    {              
        IIC_SDA((txd & 0x80) >> 7);
        txd <<= 1; 	  
		delay_us(2);   //��TEA5767��������ʱ���Ǳ����
		IIC_SCL(1);
		delay_us(2); 
		IIC_SCL(0);	
		delay_us(2);
    }	 
}
	    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i, receive = 0;

	SDA_IN();			//SDA����Ϊ����

    for(i = 0; i < 8; i++)
	{
        IIC_SCL(0); 
        delay_us(2);
		IIC_SCL(1);
        receive <<= 1;
        if(READ_SDA())receive++;   
		delay_us(1); 
    }
						 
    if(!ack)
        IIC_NAck();//����nACK
    else
        IIC_Ack(); //����ACK
		   
    return receive;
}
