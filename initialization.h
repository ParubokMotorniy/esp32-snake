#ifndef INIT_H
#define INIT_H

#include "lvgl.h"

#ifndef I2C_SDA
#define I2C_SDA 2
#endif
#ifndef I2C_SCL
#define I2C_SCL 1
#endif
#define RST_N_PIN -1
#define INT_N_PIN -1

#define TFT_DIRECTION 1

#include "TFT_eSPI.h"
#include "FT6336U.h"

static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * 10 ];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */
FT6336U ft6336u(I2C_SDA, I2C_SCL, RST_N_PIN, INT_N_PIN); 
FT6336U_TouchPointType tp; 

/* Display flushing */
inline void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors((uint16_t*)&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}

/*Read the touchpad*/
inline void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
    uint16_t touchX, touchY;

    //bool touched = tft.getTouch( &touchX, &touchY, 600 );
    tp = ft6336u.scan(); 
    int touched = tp.touch_count;

    if( touched == 0 )
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        int x = tp.tp[0].y;
        int y = screenHeight - tp.tp[0].x;

        if(x >= 0 && x < screenWidth && y >= 0 && y < screenHeight)
        {
            data->state = LV_INDEV_STATE_PR;
            data->point.x = x;
            data->point.y = y;
        }
    }
}

inline void initHardware(void)
{
    ft6336u.begin(); 
    lv_init();
    tft.begin();          /* TFT init */
    tft.setRotation( TFT_DIRECTION ); /* Landscape orientation, flipped */
    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10 );

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );
}

inline void updateUi(void)
{
    lv_task_handler();
}

#endif
