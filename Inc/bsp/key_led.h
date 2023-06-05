#ifndef __KEYLED_H__
#define __KEYLED_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"



void key_led_init(void);

uint8_t read_key(void);
void led_disp(uint8_t led);


#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

