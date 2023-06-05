#include "main.h"
#include "tim.h"
#include "key_led.h"
#include "rcc.h"
#include "adc.h"
#include "lcd.h"

#define k 3.3/4096

void key_proc(void);
void led_proc(void);
void adc_proc(void);
void pwm_proc(void);
void lcd_proc(void);

//for delay
__IO uint32_t uwTick_key, uwTick_lcd, uwTick_led, uwTick_adc, uwTick_pwm;

//for key
uint8_t key_val, key_old, key_down, key_up;

//for led
uint8_t led;
uint8_t led_V = 0x01;
uint8_t led_F = 0x02;

//for lcd
uint8_t string_lcd[21];
uint8_t which_index;

//for adc
float AO1_V;
float AO2_V;

//for loop
int i;

//for pwm
uint32_t plus1;
uint32_t plus2;
float plus1_f;
float plus2_f;
uint8_t which_plus;

//for task
uint8_t LD1 = 1;
uint8_t	LD2 = 2;

int main(void)
{

  HAL_Init();
	//HAL_Delay(1);
  SystemClock_Config();
	
	key_led_init();
	
	MX_ADC2_Init();
	
	LCD_Init();
	LCD_SetTextColor(White);
	LCD_SetBackColor(Black);
	LCD_Clear(Black);

	//plus
  MX_TIM2_Init();
	MX_TIM15_Init();
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
	HAL_TIM_IC_Start_IT(&htim15, TIM_CHANNEL_1);
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim15);	
	
	//pwm out
  MX_TIM3_Init();
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

  while (1)
  {
		key_proc();
		led_proc();
		adc_proc();
		pwm_proc();
		lcd_proc();
  }
}

void led_proc(void)
{
	if(uwTick - uwTick_led < 50)	return;
	uwTick_led = uwTick;
	
	led_disp(led);

}

void key_proc(void)
{
	if(uwTick - uwTick_key < 50)	return;
	uwTick_key = uwTick;

	key_val = read_key();
	key_down = key_val & (key_val ^ key_old);
	key_up  = ~key_val & (key_val ^ key_old);
	key_old = key_val;
	
	if(key_down == 1)
	{
		which_index ^= 1;
		LCD_Clear(Black);
		
		if(which_index == 0)
		{
			led = 0x00;
			led_V = 0x01<<(LD1-1);
			led_F = 0x01<<(LD2-1);		
		}
	}
	
	else if(key_down == 2)
	{
		/*
		i = 3;
		led = 0x00;
		led_V = led_V<<1;
		while(i--)
		{
			if(led_V == led_F)
				led_V = led_V<<1;
			if(led_V == 0x00)
				led_V = 0x01;		
		}
		*/
		
		i = 3;
		if(which_index == 1)
		{
			LD1++;
			while(i--)
			{
				if(LD1 >= 9)
					LD1 = 1;
				if(LD1 == LD2)
					LD1 += 1;	
			}
		}
	}
	
	else if(key_down == 3)
	{
		
		/*
		i = 3;
		led = 0x00;
		led_F = led_F<<1;
		while(i--)
		{
			if(led_F == led_V)
				led_F = led_F<<1;
			if(led_F == 0x00)
				led_F = 0x01;		
		}
		*/
		i = 3;
		if(which_index == 1)
		{
			LD2++;
			while(i--)
			{
				if(LD2 >= 9)
					LD2 = 1;
				if(LD2 == LD1)
					LD2 += 1;		
			}
		}
	}
	
	
	else if(key_down == 4)
	{
		which_plus ^= 1;
	}
	
}

void pwm_proc(void)
{
	if(uwTick - uwTick_pwm < 50)	return;
	uwTick_pwm = uwTick;
	
	plus1_f = 1000000.0/plus1;
	plus2_f = 1000000.0/plus2;
	
	if(plus1_f > plus2_f)
	{
		led |= led_F;
	}
	
	else 
	{
		led &= ~led_F;
	}	
	
	
	if(which_plus == 0) //plus1
	{
		__HAL_TIM_SetAutoreload(&htim3, plus1);
		__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, __HAL_TIM_GetAutoreload(&htim3)*0.5);
	}
	
	else if(which_plus == 1)
	{
		__HAL_TIM_SetAutoreload(&htim3, plus2);
		__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, __HAL_TIM_GetAutoreload(&htim3)*0.5);	
	}
		
}

void adc_proc(void)
{
	if(uwTick - uwTick_adc < 50)	return;
	uwTick_adc = uwTick;
	
	HAL_Delay(1);
	get_AO1();
	HAL_Delay(1);
	AO1_V = k*get_AO1();
	
	HAL_Delay(1);
	get_AO2();
	HAL_Delay(1);
	AO2_V = k*get_AO2();
	
	if(AO1_V > AO2_V)
	{
		led |= led_V;
	}
	
	else 
	{
		led &= ~led_V;
	}
}


void lcd_proc(void)
{
	if(uwTick - uwTick_lcd < 50)	return;
	uwTick_lcd = uwTick;

	if(which_index == 0)
	{
		sprintf((char *)string_lcd, "    DATA");
		LCD_DisplayStringLine(Line1, string_lcd);	
		
		sprintf((char *)string_lcd, "V1 : %.1f V  ", AO1_V);
		LCD_DisplayStringLine(Line3, string_lcd);

		sprintf((char *)string_lcd, "V2 : %.1f V  ", AO2_V);
		LCD_DisplayStringLine(Line4, string_lcd);
		
		sprintf((char *)string_lcd, "F1 : %.0f Hz   ", plus1_f);
		LCD_DisplayStringLine(Line5, string_lcd);

		sprintf((char *)string_lcd, "F2 : %.0f Hz   ", plus2_f);
		LCD_DisplayStringLine(Line6, string_lcd);			
	}
	
	else 
	{
		sprintf((char *)string_lcd, "    PARA");
		LCD_DisplayStringLine(Line1, string_lcd);		
	
		sprintf((char *)string_lcd, "    VD : LD%d", LD1);
		LCD_DisplayStringLine(Line3, string_lcd);
		
		sprintf((char *)string_lcd, "    FD : LD%d", LD2);
		LCD_DisplayStringLine(Line4, string_lcd);
	}

}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
			plus1 = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_2);
	}

	else if(htim->Instance == TIM15)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
			plus2 = HAL_TIM_ReadCapturedValue(&htim15, TIM_CHANNEL_1);	
	}

}


void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
