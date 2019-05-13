/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include "lvgl.h"
#include "lv_conf.h"
#include <math.h>
#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lv_test.h"
#include "lv_drv_conf.h"
//#include "lv_tutorial/1_hello_world/lv_tutorial_hello_world.h"
//#include "lv_tutorial/8_animations/lv_tutorial_animations.h"
#include "ili9341.h"


extern void lv_test_theme(lv_theme_t * th);


int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    CYGRAPHICS_START();
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    lv_init();                                              // Initialize LittlevGL
    ILI9341_init();                                         // Initialize display
    lv_disp_drv_t disp_drv;                                 // Descriptor of a display driver
    lv_disp_drv_init(&disp_drv);                            // Basic initialization
    disp_drv.disp_flush = ILI9341_flush;                    // Used in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)
    lv_disp_drv_register(&disp_drv);                        // Register the driver
    
    /*  Register the display in LittlevGL   */
    
    lv_test_theme(lv_theme_default_init(15, NULL));
    
//    lv_tutorial_animations();
    
//    lv_tutorial_hello_world();
    
    for(;;)
    {
lv_task_handler();
CyDelay(10);
    }
}

/* [] END OF FILE */
