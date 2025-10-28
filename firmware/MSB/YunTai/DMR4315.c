/*************
///海泰机电 DMR4315&DMR3515 云台电机 串口RS485驱动
///接线方式：
///  单片机------TTL转RS485模块----云台电机
///   VCC---------VCC
///   UART_TX-----TX
///   UART_RX-----RX
///   GND---------GND
/// 2023_7 V1.0
/// 2024_7 V2.0
/// Composed by
/// UESTC-School Of Automation Engineering-Technology Association-Turtle
**************/
#include "DMR4315.h"
#include "../Core/Inc/usart.h"
static uint16_t crc16(uint8_t *addr,uint8_t num);//软件CRC16校验函数
static uint8_t data[67]={0};  //发送指令buffer
static uint8_t ins=0x00;  //指令码
static uint16_t crc=0x0000;

MOTOR_STATE yuntai_motor[3];

void motor_set_speed(uint8_t id,int speed){
	
	ins=0x54;
	data[0]=0x3e;		//
	data[1]=0x00;
	data[2]=id;	
	data[3]=ins;  //
	data[4]=0x02;
	data[5]=(uint8_t)(speed>>0);
	data[6]=(uint8_t)(speed>>8);
	crc=crc16(data,7);
	
	data[7]=(uint8_t)crc;
	data[8]=(uint8_t)(crc>>8);
//	HAL_UART_Transmit(&huart2,data,9,0xff);
    HAL_UART_Transmit_DMA(DMR_UART_HANDLER,data,9);
//	delay_ms(10);
//	HAL_UART_Receive(&huart2,RxData,15,0xff);
	
}

/**************************
*@brief电机绝对值位置闭环控制  指令号0x55
*     电机将按照“位置闭环目标速度”这个设定参数值作为最大运行速度
*     电机应答当前状态
*@param id:云台电机设备id
*           拨码开关选择1~8 （拨码开关二进制值+1=id)
*@param pos: 四字节值，16384代表电机转一圈
**************************/
void motor_set_position(uint8_t id,uint32_t pos){
	
	ins=0x55;
	data[0]=0x3e;		//
	data[1]=0x00;
	data[2]=id;	
	data[3]=ins;  // 指令码
	data[4]=0x04;
	data[5]=(uint8_t)(pos>>0);
	data[6]=(uint8_t)(pos>>8);
	data[7]=(uint8_t)(pos>>16);
	data[8]=(uint8_t)(pos>>24);	
	crc=crc16(data,9);
//     crc=HAL_CRC_Calculate(&hcrc,(uint32_t*)data,9);
	
	data[9]=(uint8_t)crc;
	data[10]=(uint8_t)(crc>>8);
//	HAL_UART_Transmit(&huart2,data,11,0xff);
    HAL_UART_Transmit_DMA(DMR_UART_HANDLER,data,11);
//	delay_ms(10);
//	HAL_UART_Receive(&huart2,RxData,15,0xff);
}
/**************************
*@brief电机绝对位置闭环控制   填入角度制小数值  可以为正负
*     电机将按照“位置闭环目标速度”这个设定参数值作为最大运行速度
*     电机应答当前状态
*@param id:云台电机设备id
*           拨码开关选择1~8 （拨码开关二进制值+1=id)
*
*@param angle: 取值 -720 ~ 720，为负值时表示电机反转   顺时针正，逆时针负
*            电机旋转一圈值为360度
**************************/
void motor_set_position_angle(uint8_t id,float angle){
     int32_t pos;                 
     pos = (int32_t)(angle*45.5111111111111111f);  
     motor_set_position(id,pos);  

}
/**************************
*@brief电机绝对位置闭环控制   填入角度弧度值  可以为正负
*     电机将按照“位置闭环目标速度”这个设定参数值作为最大运行速度
*     电机应答当前状态
*@param id:云台电机设备id
*           拨码开关选择1~8 （拨码开关二进制值+1=id)
*
*@param angle: 取值 -4PI ~ +4PI，为负值时表示电机反转   顺时针正，逆时针负
*            电机旋转一圈值为2PI
**************************/
void motor_set_position_rad(uint8_t id,float rad){
     int32_t pos;                 
     pos = (int32_t)(rad * 16384.0f / 6.28318530718f);  
     motor_set_position(id,(int32_t)(pos));  

}
/**************************
*@brief电机相对位置闭环控制  指令号0x56
*     电机将按照“位置闭环目标速度”这个设定参数值作为最大运行速度
*     电机应答当前状态
*@param id:云台电机设备id
*           拨码开关选择1~8 （拨码开关二进制值+1=id)
*@param pos: int16_t，取值-32768~32767，为负值时表示电机反转
*            电机旋转一圈值为16384
**************************/
void motor_set_related_position(uint8_t id,int16_t pos){
	
	ins=0x56;
	data[0]=0x3e;		//
	data[1]=0x00;
	data[2]=id;	
	data[3]=ins;  //
	data[4]=0x02;
	data[5]=(uint8_t)(pos>>0);
	data[6]=(uint8_t)(pos>>8);
	
	crc=crc16(data,7);
	//crc=HAL_CRC_Calculate(&hcrc,(uint32_t*)data,7);
	data[7]=(uint8_t)crc;
	data[8]=(uint8_t)(crc>>8);
//	HAL_UART_Transmit(&huart2,data,9,0xff);
     HAL_UART_Transmit_DMA(DMR_UART_HANDLER,data,9);
//	delay_ms(2);
//	HAL_UART_Receive(&huart2,RxData,15,0xff);
	
}


/**************************
*@brief 关闭电机。关闭后电机松弛，可自由活动。  指令号0x50
*     
*@param id:云台电机设备id
*           拨码开关选择1~8 （拨码开关二进制值+1=id)
*
**************************/
void motor_turn_off(uint8_t id){
	
	ins=0x50;
	data[0]=0x3e;		//
	data[1]=0x00;
	data[2]=id;	
	data[3]=ins;  //
	data[4]=0x00;
	
	crc=crc16(data,5);
	//crc=HAL_CRC_Calculate(&hcrc,(uint32_t*)data,5);
	data[5]=(uint8_t)crc;
	data[6]=(uint8_t)(crc>>8);
//	HAL_UART_Transmit(&huart2,data,7,0xff);
    HAL_UART_Transmit_DMA(DMR_UART_HANDLER,data,7);
//	delay_ms(5);
//	HAL_UART_Receive(&huart2,RxData,15,0xff);
	
}
/**************************
*@brief 让电机回到设定的零点  指令号0x52
*     
*@param id:云台电机设备id
*           拨码开关选择1~8 （拨码开关二进制值+1=id)
*
**************************/
void motor_set2zero(uint8_t id){
	
	ins=0x52;
	data[0]=0x3e;		//
	data[1]=0x00;
	data[2]=id;	
	data[3]=ins;  //
	data[4]=0x00;
	
	crc=crc16(data,5);
//	crc=HAL_CRC_Calculate(&hcrc,(uint32_t*)data,5);
	data[5]=(uint8_t)crc;
	data[6]=(uint8_t)(crc>>8);
	//HAL_UART_Transmit(&huart8,data,7,0xff);
    HAL_UART_Transmit_DMA(DMR_UART_HANDLER,data,7);
//	delay_ms(5);
//	HAL_UART_Receive(&huart2,RxData,15,0xff);
	
}
//读取电机状态，由于每次发送指令时电机回自动返回一次当前状态，一般不必单独使用这个函数来读取信息。
/**************************
*@brief 读取电机状态  指令号0x2f
*         电机将从返回当前的累计绝对值角度，单圈内位置角度和角速度等信息
*       由于每次发送指令时电机回自动返回一次当前状态，一般不必单独使用这个函数来读取信息。
*@param id:云台电机设备id
*           拨码开关选择1~8 （拨码开关二进制值+1=id)
*
**************************/
void motor_read_state(uint8_t id){
	

	ins=0x2f;
	data[0]=0x3e;		//
	data[1]=0x00;
	data[2]=id;	
	data[3]=ins;  // 指令码
	data[4]=0x00;
	
	crc=crc16(data,5);
//	crc=HAL_CRC_Calculate(&hcrc,(uint32_t*)data,5);
	data[5]=(uint8_t)crc;
	data[6]=(uint8_t)(crc>>8);

	
//	HAL_UART_Transmit(&huart2,data,7,0xff);
    HAL_UART_Transmit_DMA(DMR_UART_HANDLER,data,7);

	
}
/**************************
*@brief 单独设定电机速度环目标速度值  指令号0x57
*     
*@param id:云台电机设备id
*           拨码开关选择1~8 （拨码开关二进制值+1=id)
*@param spd: int16_t，单位0.1rpm，电机速度环目标速度值。对于比较容易震荡的结构，目标速度值应该设定较小(例如spd=100)
*
**************************/
void motor_set_target_spd(uint8_t id,int16_t spd){
     ins=0x57;
	data[0]=0x3e;
	data[1]=0x00;
	data[2]=id;
	data[3]=ins;
	data[4]=0x03;
	data[5]=0x01;
	data[6]=(uint8_t)spd;
	data[7]=(uint8_t)(spd>>8);
	
	crc=crc16(data,8);
//	crc=HAL_CRC_Calculate(&hcrc,(uint32_t*)data,8);
     data[8]=(uint8_t)crc;
	data[9]=(uint8_t)(crc>>8);
//	HAL_UART_Transmit(&huart2,data,10,0xff);
		HAL_UART_Transmit_DMA(DMR_UART_HANDLER,data,10);
//	delay_ms(5);
}
/**************************
*@brief 设定电机当前位置为机械零点  指令号0x21
*     
*@param id:云台电机设备id
*           拨码开关选择1~8 （拨码开关二进制值+1=id)
*
**************************/
void motor_set_zero_point(uint8_t id){
     ins=0x21;
	data[0]=0x3e;
	data[1]=0x00;
	data[2]=id;
	data[3]=ins;
	data[4]=0x00;
	crc=crc16(data,5);
//	crc=HAL_CRC_Calculate(&hcrc,(uint32_t*)data,5);
     data[5]=(uint8_t)crc;
	data[6]=(uint8_t)(crc>>8);
//	HAL_UART_Transmit(&huart2,data,10,0xff);
    HAL_UART_Transmit_DMA(DMR_UART_HANDLER,data,10);
//	delay_ms(5);
	


}


/**************************
*@brief 单片机主控设置云台电机运动参数  指令号0x0e
*     
*@param id:云台电机设备id
*           拨码开关选择1~8 （拨码开关二进制值+1=id)
*@param pkp: 位置环Kp
*@param skp: 速度环Kp
*@param pki: 速度环Ki
*@param tgt_spd: 速度环目标速度  单位0.1rpm  
**************************/
void motor_set_param(uint8_t id,float pkp,float skp,float ski,float tgt_spd)
{

	ins=0x0e;
	data[0]=0x3c;		//
	data[1]=0x00;
	data[2]=id;	
	data[3]=ins;  // 指令码
	data[4]=0x1a;
     data[5]=0x00;  //设备地址 （对于不可软件配置地址的设备，这个参数无效，DM-R-4315/3515不可软件配置地址)
     data[6]=0x22;  //电流阈值   0x22 = 0d34 1Byte = 真实值/0.03
	data[7]=0x82;  //电压阈值   0x82 = 0d130  1Byte = 真实值/0.2
     data[8]=0x00;  //RS485串口波特率  字节低4位：0：115200  1：57600  2：38400  3：19200  4：9600
                    //can波特率        字节高4位：0：1MHz  1：500kHz  2：250kHz  3：125kHz  4：100kHz
     data[9]=((uint8_t*)(&pkp))[0];
     data[10]=((uint8_t*)(&pkp))[1];
     data[11]=((uint8_t*)(&pkp))[2];
     data[12]=((uint8_t*)(&pkp))[4];  //位置环Kp
     
     data[13]=((uint8_t*)(&tgt_spd))[0];
     data[14]=((uint8_t*)(&tgt_spd))[1];
     data[15]=((uint8_t*)(&tgt_spd))[2];
     data[16]=((uint8_t*)(&tgt_spd))[4];  //闭环目标速度  单位0.1rpm
     
     data[17]=((uint8_t*)(&skp))[0];
     data[18]=((uint8_t*)(&skp))[1];
     data[19]=((uint8_t*)(&skp))[2];
     data[20]=((uint8_t*)(&skp))[4];  //速度环Kp
     
     data[21]=((uint8_t*)(&ski))[0];
     data[22]=((uint8_t*)(&ski))[1];
     data[23]=((uint8_t*)(&ski))[2];
     data[24]=((uint8_t*)(&ski))[4];  //速度环Ki
     
     data[25]=0x00;
     data[26]=0x00;
     data[27]=0x00;
     data[28]=0x00;  //预留
     
     data[29]=40;    //速度滤波系数  1Byte = 滤波系数*100
     data[30]=92;   //功率百分比 0~100  默认92
     
     crc = crc16(data,31);
//	crc=HAL_CRC_Calculate(&hcrc,(uint32_t*)data,31);
	data[31]=(uint8_t)crc;
	data[32]=(uint8_t)(crc>>8);

   HAL_UART_Transmit_DMA(DMR_UART_HANDLER,data,33);
}


/**************************
*@brief   读取电机参数  指令号0x0c
*         电机将返回当前设定的各种参数（包括闭环运行参数）     
*
*@param id:云台电机设备id
*           拨码开关选择1~8 （拨码开关二进制值+1=id)
*
**************************/
void motor_read_param(uint8_t id){
     ins=0x0c;
	data[0]=0x3e;		//
	data[1]=0x00;
	data[2]=id;	
	data[3]=ins;  // 指令码
	data[4]=0x00;
	
	crc=crc16(data,5);
//	crc=HAL_CRC_Calculate(&hcrc,(uint32_t*)data,5);
	data[5]=(uint8_t)crc;
	data[6]=(uint8_t)(crc>>8);

	
//	HAL_UART_Transmit(&huart2,data,7,0xff);
     HAL_UART_Transmit_DMA(DMR_UART_HANDLER,data,7);
}

//H7有可配置参数的硬件CRC，可以不用下面的软件crc算法生成校验码。
/**************************
*@brief   软件crc16-modbus校验算法  
*
*@param   addr 校验内容buffer头指针
*           
*@param   num  校验内容长度
*
*@retval 生成的crc-modbus校验码
**************************/
uint16_t crc16(uint8_t *addr,uint8_t num)
{
	int i,j,temp;
	uint16_t crc=0xFFFF;	
	for(i=0;i<num;i++)
	{
		crc=crc^(*addr);
		for(j=0;j<8;j++)
		{
			temp=crc&0x0001;
			crc=crc>>1;
			if(temp)
			{
				crc=crc^0xA001;
			}
		}
		addr++;
	}
	return crc;
}

/**************************
*@brief   云台电机返回数据包解析函数
*         在串口接收完成中断内调用此函数，完成对数据包的解析
*         返回数据包长度不定且有些数据包很长，建议使用DMA+空闲中断来接收
*         包含了DM-R4315/3515云台电机返回数据包的解析以及存储
**************************/
void yuntai_motor_rec(uint8_t cnt,uint8_t* _uart_rx_buff){
     /*解析协议*/
         uint8_t i=0;
         while(i<cnt&&(!(_uart_rx_buff[i] == 0x3c && _uart_rx_buff[i+3] == ins && _uart_rx_buff[i+1] == 0x00))) i++; //根据帧头寻找定位数据包
         uint8_t length = _uart_rx_buff[i+4]+7; 
					crc = crc16(_uart_rx_buff+i,length-2);
//         crc = HAL_CRC_Calculate(&hcrc,(uint32_t*)(_uart_rx_buff+i),length-2);
         if(_uart_rx_buff[i+length-2] == (uint8_t)crc && _uart_rx_buff[i+length-1] == (uint8_t)(crc>>8)){  // 接收端CRC校验
              if(!(ins==0x0c||ins==0x0d||ins==0x0e)){  //电机正常工作返回信息为编码器角度值和角速度值
                    yuntai_motor[_uart_rx_buff[i+2]-1].val1 = (uint16_t)(_uart_rx_buff[i+5] + (_uart_rx_buff[i+6]<<8));  //单圈绝对值角度  uint16_t
                    yuntai_motor[_uart_rx_buff[i+2]-1].val2 = (int32_t)(_uart_rx_buff[i+7] + (_uart_rx_buff[i+8]<<8) + (_uart_rx_buff[i+9]<<16) + (_uart_rx_buff[i+10]<<24));  //多圈绝对值角度 int32_t
                    yuntai_motor[_uart_rx_buff[i+2]-1].val3 = (int16_t)(_uart_rx_buff[i+11] + (_uart_rx_buff[i+12]<<8));  //机械速度  int16_t
                   
                   yuntai_motor[_uart_rx_buff[i+2]-1].angle_s = (float)yuntai_motor[_uart_rx_buff[i+2]-1].val1 * (360.0f/16384.0f); //单圈绝对值角度真实值 float
                   yuntai_motor[_uart_rx_buff[i+2]-1].angle = (float)yuntai_motor[_uart_rx_buff[i+2]-1].val2 * (360.0f/16384.0f);   //多圈绝对值角度真实值 float
                   yuntai_motor[_uart_rx_buff[i+2]-1].rpm = (float)yuntai_motor[_uart_rx_buff[i+2]-1].val2 * 0.1f;                  //机械速度rpm真实值 float
              }
              else {     //配置指令  电机返回信息为配置参数
                   yuntai_motor[_uart_rx_buff[i+2]-1].skp = *((float*)(_uart_rx_buff+i+9));   
                   yuntai_motor[_uart_rx_buff[i+2]-1].ski = *((float*)(_uart_rx_buff+i+21));
                   yuntai_motor[_uart_rx_buff[i+2]-1].pkp = *((float*)(_uart_rx_buff+i+17));
                   yuntai_motor[_uart_rx_buff[i+2]-1].tgt_spd = *((int16_t*)(_uart_rx_buff+i+13));
              }
         }

}


