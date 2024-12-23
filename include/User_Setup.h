#define USER_SETUP_INFO "User_Setup"

#define GC9A01_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 240
#define TFT_RGB_ORDER TFT_RGB
#define TFT_INVERSION_ON

// ESP32-C3 Pins from manufacturer example
#define TFT_MISO -1
#define TFT_MOSI 7
#define TFT_SCLK 6
#define TFT_CS   10
#define TFT_DC   2
#define TFT_RST  -1

// Additional settings
#define SPI_FREQUENCY  80000000  // Use full speed as in example
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
#define SUPPORT_TRANSACTIONS