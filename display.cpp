#include "display.h"
#include <Wire.h>

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_spd2010.h"
#include "driver/spi_master.h"

#define LCD_SCK 40
#define LCD_D0 46
#define LCD_D1 45
#define LCD_D2 42
#define LCD_D3 41
#define LCD_CS 21
#define LCD_BK 5

#define SDA_PIN 11
#define SCL_PIN 10

esp_lcd_panel_handle_t panel;

static void lcd_reset()
{
  Wire.beginTransmission(0x20);
  Wire.write(0x01);
  Wire.write(0xFB);
  Wire.endTransmission();
  delay(100);

  Wire.beginTransmission(0x20);
  Wire.write(0x01);
  Wire.write(0xFF);
  Wire.endTransmission();
  delay(100);
}

void display_init()
{
  Wire.begin(SDA_PIN,SCL_PIN);

  pinMode(LCD_BK,OUTPUT);
  digitalWrite(LCD_BK,HIGH);

  lcd_reset();

  spi_bus_config_t buscfg = {
    .data0_io_num = LCD_D0,
    .data1_io_num = LCD_D1,
    .sclk_io_num = LCD_SCK,
    .data2_io_num = LCD_D2,
    .data3_io_num = LCD_D3,
    .max_transfer_sz = 4096
  };

  spi_bus_initialize(SPI2_HOST,&buscfg,SPI_DMA_CH_AUTO);

  esp_lcd_panel_io_spi_config_t io_config = {
    .cs_gpio_num = LCD_CS,
    .dc_gpio_num = -1,
    .spi_mode = 0,
    .pclk_hz = 40000000,
    .trans_queue_depth = 10,
    .lcd_cmd_bits = 32,
    .lcd_param_bits = 8,
    .flags = {.quad_mode = 1}
  };

  esp_lcd_panel_io_handle_t io_handle;

  esp_lcd_new_panel_io_spi(SPI2_HOST,&io_config,&io_handle);

  spd2010_vendor_config_t vendor_config = {
    .flags = {.use_qspi_interface = 1}
  };

  esp_lcd_panel_dev_config_t panel_config = {
    .reset_gpio_num = -1,
    .bits_per_pixel = 16,
    .vendor_config = &vendor_config
  };

  esp_lcd_new_panel_spd2010(io_handle,&panel_config,&panel);

  esp_lcd_panel_reset(panel);
  esp_lcd_panel_init(panel);
  esp_lcd_panel_disp_on_off(panel,true);
}void display_flush(int x1,int y1,int x2,int y2,void *data)
{
    esp_lcd_panel_draw_bitmap(
        panel,
        x1,
        y1,
        x2+1,
        y2+1,
        data
    );
}