#define USER_SETUP_INFO "User_Setup"

#define GC9A01_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 240
#define TFT_RGB_ORDER TFT_RGB
#define TFT_INVERSION_ON

// ESP32-C3 Pins from manufacturer example
#define TFT_MISO -1    // Not used
#define TFT_MOSI 7     // GPIO7
#define TFT_SCLK 6     // GPIO6
#define TFT_CS   10    // GPIO10
#define TFT_DC   2     // GPIO2
#define TFT_RST  -1    // Not used


// cfg.pin_sclk = 6;  // Set SCLK pin number for SPI
// cfg.pin_mosi = 7;  // Set MOSI pin number for SPI
// cfg.pin_miso = -1; // Set MISO pin number for SPI (-1 = disable)
// cfg.pin_dc = 2;    // Set D/C pin number for SPI (-1 = disable)
// cfg.pin_cs = 10;   // Set CS pin number (-1 = disable)
// cfg.pin_rst = -1;  // Set RST pin number (-1 = disable)

#define SPI_FREQUENCY  27000000  // Try lower frequency