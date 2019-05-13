/**
 * @file ili9341.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "ili9341.h"


/*********************
 *      DEFINES
 *********************/
#define CYPINWRITEHIGH  RST_Write( 0b1 );
#define CYPINWRITELOW   RST_Write( 0b0 ); 
/**********************
 *      TYPEDEFS
 **********************/

/*The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct. */
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void ILI9341_send_cmd(uint16 cmd);
static void ILI9341_send_data(void * data, uint16 length);
static void ili9341_send_color(void * data, uint16 length);
static void sendStartSequence(const uint8_t *buff, uint32_t len);
static void lcd_start_reset (void);


/**********************
 *  STATIC VARIABLES
 **********************/
static const uint8_t mcu35_init_sequence_web_edited[]  = {
//    0xFF,0x00,          // ?
//    0xFF,0x00,          // ?
//    0xDD,5,             // Delay 5
//    0xFF,0x00,          //
//    0xFF,0x00,          //
//    0xFF,0x00,          //
//    0xFF,0x00,          // ?
//    0xDD,10,            // delay 10
    //  
//    0xB0,0x01,0x00,                                     // IF Mode control
//    0xB3,0x04,0x02,0x00,0x00,0x10,                      // Frame Rate Control - only 2 paramters
//    0xB4,0x01,0x11,                                     // Display inversion control 
//    0xC0,0x08,0x13,0x3B,0x00,0x00,0x00,0x01,0x00,0x43,  // Power Control 1
//    0xC1,0x04,0x08,0x15,0x08,0x08,                      // Power Control 2
//    0xC4,0x04,0x15,0x03,0x03,0x01,                      // ?
//    0xC6,0x01,0x02,                                     // ?
    // ??
//    0xC8,0x15,0x0C,0x05,0x0A,0x6B,0x04,0x06,0x15,0x10,0x00,0x31,0x10,0x15,0x06,0x64,0x0D,0x0A,0x05,0x0C,0x31,0x00,
    0x35,0x01,0x00,                     // Tearing Effect 
 //   0x0C,0x01,0x66,                     // Read pixel format?
    0x3A,0x01,0x55,                     // Pixel Format Set
    
    0x44,0x02,0x00,0x01,                // Set Tear Scanline
//    0xD0,0x04,0x07,0x07,0x14,0xA2,      // NVM Write
//    0xD1,0x03,0x03,0x5A,0x10,           // NVM Protection Key
//   0xD2,0x03,0x03,0x04,0x04,           // NVM Status Read

    0x11,0x00,                          // Sleep Out
    0xDD,150,                           // Delay 150ms
    0x2A,0x04,0x00,0x00,0x01,0x3F,      // Column Set Address 320
    0x2B,0x04,0x00,0x00,0x01,0xDF,      // Page Set Address   480
    0xDD,100,                           // Delay 100ms
    0x29,0x00,                          // Display On
    0xDD,30                            // delay 30ms
//    0x2C,0x00                           // Memory Write
};
static const uint8_t ILI9341_regValues_2_4[]  = {        // BOE 2.4"                                                                                                                
0x1,0x0,                                  // Software Reset
0x28,0x0,                                 // Display Off
0x3A,0x1,0x55,                            // Pixel Format RGB=16-bits/pixel MCU=16-bits/Pixel
0xF6,0x3,0x1,0x1,0x0,                     // Interface control .. I have no idea
#if 0    
0xCF,0x3,0x0,0x81,0x30,                   // Not defined
0xED,0x4,0x64,0x3,0x12,0x81,              // Not defined
0xE8,0x3,0x85,0x10,0x78,                  // Not defined
0xCB,0x5,0x39,0x2C,0x0,0x34,0x2,          // Not defined
0xF7,0x1,0x20,                            // Not defined
0xEA,0x2,0x0,0x0,                         // Not defined  
#endif

#if 0   // Part of the extended command set
0xB0,0x1,0x0,                             // RGB Interface Control
0xB1,0x2,0x0,0x1B,                        // Frame Rate Control
0xB4,0x1,0x0,                             // Display Inversion Control
0xC0,0x1,0x21,                            // Power Control 1
0xC1,0x1,0x11,                            // Power Control 2
0xC5,0x2,0x3F,0x3C,                       // VCOM Control 1
0xC7,0x1,0xB5,                            // VCOM Control 2
    
#endif    
0x36,0x1,0x48,                            // Memory Access Control

#if 0
0xF2,0x1,0x0,                             // Not defined
#endif

0x26,0x1,0x1,                             // Gamma Set
//0xE0,0xF,0xF,0x26,0x24,0xB,0xE,0x9,0x54,0xA8,0x46,0xC,0x17,0x9,0xF,0x7,0x0,    // Positive Gamma Correction
//0xE1,0xF,0x0,0x19,0x1B,0x4,0x10,0x7,0x2A,0x47,0x39,0x3,0x6,0x6,0x30,0x38,0xF,  // Negative Gamme Correction
0x11,0x0,                                 // Sleep Out
0x29,0x0,                                 // Display On
0x36,0x1,0x00,                            // Memory Access Control
0x2A,0x4,0x0,0x0,0x0,0xEF,                // Column Address Set = 239
0x2B,0x4,0x0,0x0,0x1,0x3F,                // Row Address Set  = 319
0x33,0x6,0x0,0x0,0x1,0x40,0x0,0x0,        // Vertical Scrolling Definition
0x37,0x2,0x0,0x0,                         // Vertical Scrolling Start Address
0x13,0x0,                                 // Normal Display ON
0x20,0x0                                  // Display Inversion OFF
};
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

 void ILI9341_init(void)
{
 	lcd_init_cmd_t ili_init_cmds[]={
//		{0xCF, {0x00, 0x83, 0X30}, 3},
//		{0xED, {0x64, 0x03, 0X12, 0X81}, 4},
//		{0xE8, {0x85, 0x01, 0x79}, 3},
//		{0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
//		{0xF7, {0x20}, 1},
//		{0xEA, {0x00, 0x00}, 2},
//		{0xC0, {0x26}, 1},			/*Power control*/
//		{0xC1, {0x11}, 1},			/*Power control */
//		{0xC5, {0x35, 0x3E}, 2},	/*VCOM control*/
//		{0xC7, {0xBE}, 1},			/*VCOM control*/
//		{0x36, {0x28}, 1},			/*Memory Access Control*/
//		{0x3A, {0x55}, 1},			/*Pixel Format Set*/
//		{0xB1, {0x00, 0x1B}, 2},
//		{0xF2, {0x08}, 1},
//		{0x26, {0x01}, 1},
//		{0xE0, {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00}, 15},
//		{0XE1, {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F}, 15},
//		{0x2A, {0x00, 0x00, 0x00, 0xEF}, 4},
//		{0x2B, {0x00, 0x00, 0x01, 0x3f}, 4},
//		{0x2C, {0}, 0},
//		{0xB7, {0x07}, 1},
//		{0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},
//		{0x11, {0}, 0x80},
//		{0x29, {0}, 0x80},
//		{0, {0}, 0xff},
        
        
           //  From regVal_2_4
        {0x01, {0}, 0x00},
        {0x28, {0}, 0x00},
        {0x3A, {0x55}, 1},                          /*Pixel Format Set*/
        {0xF6, {0x1, 0x1, 0x00}, 3},                
        {0x36, {0x48}, 1},                          /*Memory Access Control*/
        {0x26, {0x1}, 1},
        {0x11, {0}, 0},
        {0x29, {0}, 0x0},                           //  Display on
        {0x36, {0x0}, 1},
        {0x2A, {0x00, 0x00, 0x01, 0x3F}, 4},    //  Collumn address set 320
        {0x2B, {0x00, 0x00, 0x00, 0xEF}, 4},    //  Row address set 240
        {0x33, {0x0, 0x0, 0x1, 0x40, 0x0, 0x0}, 6},
        {0x37, {0x0, 0x0}, 2},
        {0x13, {0x0}, 1},
        {0x20, {0}, 0},       
        
		{0, {0}, 0xff}
        
	};  

//	Initialize non-SPI GPIOs
//	gpio_set_direction(ILI9341_DC, GPIO_MODE_OUTPUT);
//	gpio_set_direction(ILI9341_RST, GPIO_MODE_OUTPUT);
//	gpio_set_direction(ILI9341_BCKL, GPIO_MODE_OUTPUT);
//
//	Reset the display
//	gpio_set_level(ILI9341_RST, 0);
//	vTaskDelay(100 / portTICK_RATE_MS);
//	gpio_set_level(ILI9341_RST, 1);
//	vTaskDelay(100 / portTICK_RATE_MS);
//
//
//	printf("ILI9341 initialization.\n");    */
lcd_start_reset();

//	Send all the commands
	uint16_t cmd = 0;
	while (ili_init_cmds[cmd].databytes!=0xff) {
		ILI9341_send_cmd(ili_init_cmds[cmd].cmd);
		ILI9341_send_data(ili_init_cmds[cmd].data, ili_init_cmds[cmd].databytes&0x1F);
		if (ili_init_cmds[cmd].databytes & 0x80) {
			CyDelay(100);
		}
		cmd++;
	}
//    lcd_start_reset();
//    sendStartSequence(ILI9341_regValues_2_4, sizeof(ILI9341_regValues_2_4));
    
    
}

void ILI9341_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
{
	/* Display controller does not support
	* setting a RAM address. It always starts 
	* from XS/YS reading and writing! */
	
	uint8_t data[4];
	
//	Set rectangle

	/*Column address set*/
	ILI9341_send_cmd(0x2A);
	data[0] = (x1 >> 8) & 0xFF;	/* High byte for x1 left coordinate start column; &0xFF is unnecessary	*/
	data[1] = x1 & 0xFF;		/* Low byte for x1 left coordinate start column	*/
	data[2] = (x2 >> 8) & 0xFF;	/* High byte for x2 right coordinate end column; &0xFF is unnecessary	*/
	data[3] = x2 & 0xFF;		/* Low byte for x2 right coordinate end column	*/
	ILI9341_send_data(data, 4);

	/*Page(Row) address set*/
	ILI9341_send_cmd(0x2B);
	data[0] = (y1 >> 8) & 0xFF;	/* High byte for y1 top coordinate start row; &0xFF is unnecessary	*/
	data[1] = y1 & 0xFF;		/* Low byte for y1 top coordinate start row	*/
	data[2] = (y2 >> 8) & 0xFF;	/* High byte for y2 bottom coordinate end row; &0xFF is unnecessary	*/
	data[3] = y2 & 0xFF;		/* Low byte for y2 bottom coordinate end row	*/
	ILI9341_send_data(data, 4);

	/*Memory write mode*/
	ILI9341_send_cmd(0x2C);	//Rectangle set

	uint32_t size = (x2 - x1 + 1) * (y2 - y1 + 1);
	uint16_t buf[LV_HOR_RES];

	uint32_t i;
	if(size < LV_HOR_RES) {
		for(i = 0; i < size; i++) buf[i] = color.full;

	} else {
		for(i = 0; i < LV_HOR_RES; i++) buf[i] = color.full;
	}

	while(size > LV_HOR_RES) {
		ili9341_send_color(buf, LV_HOR_RES * 2);
		size -= LV_HOR_RES;
	}

	ili9341_send_color(buf, size * 2);	/*Send the remaining data*/
}


void ILI9341_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_map)
{
      /*Return if the area is out the screen*/
//    if (x2 < 0 || y2 < 0 || x1 > LV_HOR_RES - 1 || y1 > LV_VER_RES - 1)
//    {
//        lv_flush_ready();
//        return;
//    }

    /*Truncate the area to the screen*/
   int32 act_x1 = x1 < 0 ? 0 : x1;
   int32 act_y1 = y1 < 0 ? 0 : y1;
   int32 act_x2 = x2 > LV_HOR_RES - 1 ? LV_HOR_RES - 1 : x2;
   int32 act_y2 = y2 > LV_VER_RES - 1 ? LV_VER_RES - 1 : y2;  
    
	

/* Display controller does not support
 * setting a RAM address. It always starts 
 * from XS/YS reading and writing! */

/* - Orientation managed completely by controller */

//	Set rectangle
    uint8 data[4];
	/*Column address set*/
	ILI9341_send_cmd(0x2A);
	data[0] = (act_x1 >> 8) & 0xFF;
	data[1] = act_x1 & 0xFF;
	data[2] = (act_x2 >> 8) & 0xFF;
	data[3] = act_x2 & 0xFF;
	ILI9341_send_data(data, sizeof(data));

	/*Page address set*/
	ILI9341_send_cmd(0x2B);
	data[0] = (act_y1 >> 8) & 0xFF;
	data[1] = act_y1 & 0xFF;
	data[2] = (act_y2 >> 8) & 0xFF;
	data[3] = act_y2 & 0xFF;
	ILI9341_send_data(data, sizeof(data));

	/*Memory write mode*/
	ILI9341_send_cmd(0x2C);	//Rectangle set
    uint16 full_w = x2 - x1 +1;
    int16 i;
    uint16 act_w = act_x2 - act_x1 +1;
    for(i = act_y1; i <= act_y2; i++)
    {
        GraphicLCDIntf_WriteM16_A1((uint16 *) color_map, act_w);
        color_map += full_w;
    }
    
    
    
    
	lv_flush_ready();

}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void lcd_start_reset (void)
    {
     //************* Reset LCD Driver ****************//
CYPINWRITEHIGH
CyDelay(1); // Delay 1ms
CYPINWRITELOW
CyDelay(10); // Delay 10ms // This delay time is necessary
CYPINWRITEHIGH
CyDelay(120); // Delay 120 ms
    } 

static void sendStartSequence(const uint8_t *buff, uint32_t len)
{
    for(unsigned int i=0;i<len;i++)
    {
        if(buff[i] == 0xDD) // 
        {
            //printf("Delay %d\r\n",buff[i+1]);
            CyDelay(buff[i+1]);
            i=i+1;
        }
        else
        {
            ILI9341_send_cmd(buff[i]);
            //printf("C = %02X ",buff[i]);
            i=i+1;
            unsigned int count;
            count = buff[i];
            //printf("%02X ",count);
            for(unsigned int j=0;j<count;j++)
            {
                i=i+1;
                GraphicLCDIntf_Write16_A1(buff[i]);
            //    printf(" %02X",buff[i]);
            }
            //printf("\r\n");
        }
    }
}

static void ILI9341_send_cmd(uint16 cmd)
{
//	gpio_set_level(ILI9341_DC, 0);	 /*Command mode*/
//	disp_spi_send_data(&cmd, 1);
    
    GraphicLCDIntf_Write16_A0(cmd);
}

static void ILI9341_send_data(void * data, uint16 length)
{
//	gpio_set_level(ILI9341_DC, 1);	 /*Data mode*/
//	disp_spi_send_data(data, length);
    
    GraphicLCDIntf_WriteM16_A1(data, length);
}

static void ili9341_send_color(void * data, uint16 length)
{
//    gpio_set_level(ILI9341_DC, 1);   /*Data mode*/
//    disp_spi_send_colors(data, length);
    
    GraphicLCDIntf_WriteM16_A1(data, length);
}
