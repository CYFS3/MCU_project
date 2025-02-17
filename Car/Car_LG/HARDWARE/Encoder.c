#include "Encoder.h"


void Encoder_Left_Front_TIM2Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 			//	引脚初始化的结构体  
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;	//	定时器初始化的结构体
	TIM_ICInitTypeDef TIM_ICInitStructure; 			//	定时器编码器模式初始化的结构体
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); 	//	使能TIM2时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); 	//	使能GPIOA时钟	
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;		
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 	//	浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//	根据GPIO_InitStructure的参数初始化GPIOA

	
	TIM_DeInit(TIM2);
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 60000; 				//	设定计数器自动重装值
	TIM_TimeBaseStructure.TIM_Prescaler = 0; 						//	预分频器 
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 		//	选择时钟分频：不分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 	//	TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); 				//	根据TIM_TimeBaseInitStruct的参数初始化定时器TIM2
	
	TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);//使用编码器模式3：CH1、CH2同时计数，四分频
	
	//初始化TIM2输入捕获参数
	TIM_ICStructInit(&TIM_ICInitStructure); 							//	把TIM_ICInitStruct 中的每一个参数按缺省值填入
	TIM_ICInitStructure.TIM_ICFilter = 0x03;  							// 	IC1F=0011 配置输入滤波器（连续采样八次）
	TIM_ICInit(TIM2, &TIM_ICInitStructure); 							//	根TIM_ICInitStructure参数初始化定时器TIM2编码器模式

	TIM2->CNT = ENCODER_STARTER;
	TIM_Cmd(TIM2, ENABLE); 							
}	


void Encoder_Left_Back_TIM3Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 			//	引脚初始化的结构体  
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;	//	定时器初始化的结构体
	TIM_ICInitTypeDef TIM_ICInitStructure; 			//	定时器编码器模式初始化的结构体

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); 	//	使能TIM3时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); 	//	使能GPIOB时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//使能AFIO时钟
	
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);//重映射
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;	//	PB4、PB5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 	//	浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//	根据GPIO_InitStructure的参数初始化GPIOA

	TIM_DeInit(TIM3);
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = 60000; 				//	设定计数器自动重装值
	TIM_TimeBaseStructure.TIM_Prescaler = 0; 						//	预分频器 
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 		//	选择时钟分频：不分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 	//	TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); 				//	根据TIM_TimeBaseInitStruct的参数初始化定时器TIM4

	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);//使用编码器模式3：CH1、CH2同时计数，四分频

	//初始化TIM3输入捕获参数
	TIM_ICStructInit(&TIM_ICInitStructure); 							//	把TIM_ICInitStruct 中的每一个参数按缺省值填入
	TIM_ICInitStructure.TIM_ICFilter = 0x03;  							// 	IC1F=0011 配置输入滤波器（连续采样八次）
	TIM_ICInit(TIM3, &TIM_ICInitStructure); 							//	根TIM_ICInitStructure参数初始化定时器TIM3编码器模式

	TIM3->CNT = ENCODER_STARTER;
	TIM_Cmd(TIM3, ENABLE); 							
}




void Init_Encoder(void)
{
	Encoder_Left_Front_TIM2Init();
	Encoder_Left_Back_TIM3Init();
}

int Read_Encoder(u8 TIMX)
{
   int Encoder_TIM;    
   switch(TIMX)
	 {
	   case 2:
		 {
			 Encoder_TIM= (short)TIM2 -> CNT;
			 TIM2 -> CNT=ENCODER_STARTER;//防止超出范围
			 break;
		 }
		 case 3:
		 {
			 Encoder_TIM= (short)TIM3 -> CNT;
			 TIM3 -> CNT=ENCODER_STARTER;//防止超出范围
			 break;
		 }
		 default:  Encoder_TIM=0;
	 }
		return Encoder_TIM;
}


int Encoder_Val=0;  //编码器脉冲数
float CarSpeed=0;//单位时间内小车的速度
float Distance=0;//单位时间内的距离

float GetCarSpeed(WheelNumber WHEEL)
{
		Encoder_Val=Read_Encoder(WHEEL)-ENCODER_STARTER;                   //读取编码器脉冲个数
		Distance = (float)Encoder_Val/1320*3.1415*0.065;//计算距离
		CarSpeed = Distance/0.1;//计算速度，单位m/s
		return CarSpeed;
}

float Car_Wheel_Speed[2];//单位时间内小车轮子的速度
float oneDistanceSum=0;//单位时间内四轮的路程总和
float DistanceSum=0;//行使的总路程
float Car_Speed=0;//小车的平均速度
void get_Car_All_Speed(void)
{
	Car_Speed=0;
	oneDistanceSum=0;
	for(int i=0;i<2;i++)
	{
		Encoder_Val=Read_Encoder(i+2) - ENCODER_STARTER;					 //读取编码器脉冲个数
		Distance = (float)Encoder_Val/1320*3.1415*0.065;//计算距离
		Car_Wheel_Speed[i]=Distance/0.1;
		oneDistanceSum+=Distance>0?Distance:-Distance;
		Car_Speed+=Car_Wheel_Speed[i];
	}
	DistanceSum+=(oneDistanceSum / 2);
	Car_Speed=Car_Speed / 2;
}
	

