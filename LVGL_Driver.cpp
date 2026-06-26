/*****************************************************************************
  | File        :   LVGL_Driver.c
  | Description :   LVGL display driver for SPD2010 round display
*****************************************************************************/

#include "LVGL_Driver.h"

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[LVGL_BUF_LEN];
static lv_color_t buf2[LVGL_BUF_LEN];

/* -------------------------------------------------- */
/* Debug                                              */
/* -------------------------------------------------- */

void Lvgl_print(const char * buf)
{
    // Serial.printf(buf);
}

/* -------------------------------------------------- */
/* ROUNDER                                            */
/* -------------------------------------------------- */

void Lvgl_port_rounder_callback(struct _lv_disp_drv_t * disp_drv, lv_area_t * area)
{
    uint16_t x1 = area->x1;
    uint16_t x2 = area->x2;

    area->x1 = (x1 >> 2) << 2;
    area->x2 = ((x2 >> 2) << 2) + 3;
}

/* -------------------------------------------------- */
/* DISPLAY FLUSH                                      */
/* -------------------------------------------------- */

void Lvgl_Display_LCD(lv_disp_drv_t *disp_drv,
                      const lv_area_t *area,
                      lv_color_t *color_p)
{
    /* Send LVGL buffer directly */
    LCD_addWindow(
        area->x1,
        area->y1,
        area->x2,
        area->y2,
        (uint16_t *)color_p
    );

    lv_disp_flush_ready(disp_drv);
}

/* -------------------------------------------------- */
/* TOUCH INPUT                                        */
/* -------------------------------------------------- */

void Lvgl_Touchpad_Read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    bool tp_pressed = false;
    uint16_t tp_x = 0;
    uint16_t tp_y = 0;
    uint8_t tp_cnt = 0;

    tp_pressed = Touch_Get_xy(&tp_x, &tp_y, NULL, &tp_cnt, CONFIG_ESP_LCD_TOUCH_MAX_POINTS);

    if(tp_pressed && tp_cnt > 0)
    {
        data->point.x = tp_x;
        data->point.y = tp_y;
        data->state = LV_INDEV_STATE_PR;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

/* -------------------------------------------------- */
/* LVGL TICK TIMER                                    */
/* -------------------------------------------------- */

void example_increase_lvgl_tick(void *arg)
{
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

/* -------------------------------------------------- */
/* LVGL INIT                                          */
/* -------------------------------------------------- */

void Lvgl_Init(void)
{
    lv_init();

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LVGL_BUF_LEN);

    /* Display driver */

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = LCD_WIDTH;
    disp_drv.ver_res = LCD_HEIGHT;

    disp_drv.flush_cb = Lvgl_Display_LCD;
    disp_drv.rounder_cb = Lvgl_port_rounder_callback;

    disp_drv.full_refresh = 0;   // faster and avoids brightness drop

    disp_drv.draw_buf = &draw_buf;

    lv_disp_drv_register(&disp_drv);

    /* Touch driver */

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);

    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = Lvgl_Touchpad_Read;

    lv_indev_drv_register(&indev_drv);

    /* Test label */

    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "RAKSHAK DISPLAY READY");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    /* LVGL tick timer */

    const esp_timer_create_args_t lvgl_tick_timer_args =
    {
        .callback = &example_increase_lvgl_tick,
        .name = "lvgl_tick"
    };

    esp_timer_handle_t lvgl_tick_timer = NULL;

    esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000);
}

/* -------------------------------------------------- */
/* LVGL LOOP                                          */
/* -------------------------------------------------- */

void Lvgl_Loop()
{
    static uint32_t lastTick = 0;

    uint32_t now = millis();
    uint32_t diff = now - lastTick;

    if(diff > 0)
    {
        lv_tick_inc(diff);
        lastTick = now;
    }

    lv_timer_handler();
}