
#define ST128x160


#ifdef ST128x160
    #define ST7735_DRIVER
    #define ST7735_GREENTAB

    #define TFT_WIDTH  128
    #define TFT_HEIGHT 160

    #define TFT_MOSI 9
    #define TFT_SCLK 8
    #define TFT_CS   5
    #define TFT_DC   18
    #define TFT_RST  16
    #define TFT_BL   21

    #define LOAD_GLCD
    #define LOAD_FONT2
    #define LOAD_FONT4
    #define LOAD_GFXFF
    #define SMOOTH_FONT

    #define SPI_FREQUENCY       27000000
    #define SPI_READ_FREQUENCY  20000000
#endif