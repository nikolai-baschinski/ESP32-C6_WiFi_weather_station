#ifndef LCD_H_
#define LCD_H_

#include "stdint.h"
#include "GUI_Paint.h"
#include "../ProcessImage.h"

void init_LCD();
void cyclic_LCD(struct ProcessImage* p_pi);

#endif /* LCD_H_ */
