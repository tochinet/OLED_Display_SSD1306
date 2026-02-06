#include <Wire.h>
#include "OLED_Display_SSD1306.h"

#define OLED_Display_SSD1306_Black 0
#define OLED_Display_SSD1306_White 1
#define OLED_Display_SSD1306_Inverse 2

#define swapVars(a, b) (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b)))
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#if defined(I2C_BUFFER_LENGTH)
#define WIRE_MAX min(256, I2C_BUFFER_LENGTH) 
#elif defined(BUFFER_LENGTH)
#define WIRE_MAX min(256, BUFFER_LENGTH) 
#elif defined(SERIAL_BUFFER_SIZE)
#define WIRE_MAX min(255, SERIAL_BUFFER_SIZE - 1) 
#else
#define WIRE_MAX 32 
#endif

uint8_t OLED_Display_SSD1306_i2caddr;
uint8_t OLED_Display_SSD1306_Width = 128;
uint8_t OLED_Display_SSD1306_Height = 64;
int OLED_Display_SSD1306_Rotation = 0;
uint8_t *OLED_Display_SSD1306_Buffer;

#if defined(ESP8266)
static void yield(void) {}
#endif

void OLED_Display_SSD1306_drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if ((x >= 0) && (x < OLED_Display_SSD1306_Width) && (y >= 0) && (y < OLED_Display_SSD1306_Height)) {
        switch (OLED_Display_SSD1306_Rotation) {
            case 1:
                swapVars(x, y);
                x = OLED_Display_SSD1306_Width - x - 1;
                break;
            case 2:
                x = OLED_Display_SSD1306_Width - x - 1;
                y = OLED_Display_SSD1306_Height - y - 1;
                break;
            case 3:
                swapVars(x, y);
                y = OLED_Display_SSD1306_Height - y - 1;
                break;
        }
        switch (color) {
            case OLED_Display_SSD1306_White:
                OLED_Display_SSD1306_Buffer[x + (y / 8) * OLED_Display_SSD1306_Width] |= (1 << (y & 7));
                break;
            case OLED_Display_SSD1306_Black:
                OLED_Display_SSD1306_Buffer[x + (y / 8) * OLED_Display_SSD1306_Width] &= ~(1 << (y & 7));
                break;
            case OLED_Display_SSD1306_Inverse:
                OLED_Display_SSD1306_Buffer[x + (y / 8) * OLED_Display_SSD1306_Width] ^= (1 << (y & 7));
                break;
        }
    }
}

void OLED_Display_SSD1306_display(void)
{
    Wire.beginTransmission(OLED_Display_SSD1306_i2caddr);
    Wire.write((uint8_t)0x00);
    Wire.write(0x22); // Set Page addr
    Wire.write(0x00); // Page start address
    Wire.write(0xFF); // Page end
    Wire.write(0x21); // Set Column addr
    Wire.write(0x00); // Column start address
    Wire.endTransmission();
    Wire.beginTransmission(OLED_Display_SSD1306_i2caddr);
    Wire.write((uint8_t)0x00);
    Wire.write(0x7F); // Column end address (total width - 1)
    Wire.endTransmission();
    #if defined(ESP8266)
    yield();
    #endif
    uint16_t count = OLED_Display_SSD1306_Width * ((OLED_Display_SSD1306_Height + 7) / 8);
    uint8_t *ptr = OLED_Display_SSD1306_Buffer;
    Wire.beginTransmission(OLED_Display_SSD1306_i2caddr);
    Wire.write((uint8_t)0x40);
    uint16_t bytesOut = 1;
    while (count--) {
        if (bytesOut >= WIRE_MAX) {
            Wire.endTransmission();
            Wire.beginTransmission(OLED_Display_SSD1306_i2caddr);
            Wire.write((uint8_t)0x40);
            bytesOut = 1;
        }
        Wire.write(*ptr++);
        bytesOut++;
    }
    Wire.endTransmission();
    #if defined(ESP8266)
    yield();
    #endif
}

void OLED_Display_SSD1306_drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w,int16_t h, uint16_t color)
{
    int16_t byteOLED_Display_SSD1306_Width = (w + 7) / 8;
    uint8_t b = 0;
    for(int j=0;j<h;j++,y++){
        for(int i=0;i<w;i++){
            if (i & 7){
                b <<= 1;
            }
            else{
                b = bitmap[j * byteOLED_Display_SSD1306_Width + i / 8];
            }
            if (b & 0x80){
                OLED_Display_SSD1306_drawPixel(x + i, y, 1);
            }
            else{
                OLED_Display_SSD1306_drawPixel(x + i, y, 0);
            }
        }
    }
}

void OLED_Display_SSD1306_clearDisplay(void) {
    memset(OLED_Display_SSD1306_Buffer, 0, OLED_Display_SSD1306_Width * ((OLED_Display_SSD1306_Height + 7) / 8));
}

void OLED_Display_SSD1306_init(uint8_t _i2caddr)
{
    OLED_Display_SSD1306_i2caddr = _i2caddr;
    Wire.begin();
    Wire.beginTransmission(OLED_Display_SSD1306_i2caddr);
    Wire.write((uint8_t)0x00); // Co 0 D/C 0 (Write Command Mode)
    Wire.write(0xAE); // Display off
    Wire.write(0xD5); // Set display clock divider
    Wire.write(0x80); // the suggested ratio 0x80
    Wire.write(0xA8); // Set Mux Ratio
    Wire.endTransmission();

    Wire.beginTransmission(OLED_Display_SSD1306_i2caddr);
    Wire.write((uint8_t)0x00); // Co 0 D/C 0 (Write Command Mode)
    Wire.write(0x3F); // 63
    Wire.endTransmission();

    Wire.beginTransmission(OLED_Display_SSD1306_i2caddr);
    Wire.write((uint8_t)0x00); // Co 0 D/C 0 (Write Command Mode)
    Wire.write(0xD3); // Set Display Offset
    Wire.write(0x0);  // No offset
    Wire.write(0x40); // Set Display Start Line (0x0);
    Wire.write(0x8D); // Enable Charge Pump Regulator
    Wire.endTransmission();

    Wire.beginTransmission(OLED_Display_SSD1306_i2caddr);
    Wire.write((uint8_t)0x00); // Co 0 D/C 0 (Write Command Mode)
    Wire.write(0x14); // 0x10;
    Wire.endTransmission();

    Wire.beginTransmission(OLED_Display_SSD1306_i2caddr);
    Wire.write((uint8_t)0x00); // Co 0 D/C 0 (Write Command Mode)
    Wire.write(0x20); // Set Memory Addr Mode
    Wire.write(0x00); // Horizontal Addr Mode
    Wire.write(0xA1); // Set SEGREMAP (0xA1 -> column addr 127 to seg0)
    Wire.write(0xC8); // Set COM Output Scan Direction (c0h/c8h)
    Wire.endTransmission();

    Wire.beginTransmission(OLED_Display_SSD1306_i2caddr);
    Wire.write((uint8_t)0x00); // Co 0 D/C 0 (Write Command Mode)
    Wire.write(0xDA); // Set COM pins
    Wire.endTransmission();

    Wire.beginTransmission(OLED_Display_SSD1306_i2caddr);
    Wire.write((uint8_t)0x00); // Co 0 D/C 0 (Write Command Mode)
    Wire.write(0x12); // 0x12
    Wire.endTransmission();

    Wire.beginTransmission(OLED_Display_SSD1306_i2caddr);
    Wire.write((uint8_t)0x00); // Co 0 D/C 0 (Write Command Mode)
    Wire.write(0x81); // Set Contrast Control
    Wire.endTransmission();

    Wire.beginTransmission(OLED_Display_SSD1306_i2caddr);
    Wire.write((uint8_t)0x00); // Co 0 D/C 0 (Write Command Mode)
    Wire.write(0xCF); // Set Contrast
    Wire.endTransmission();

    Wire.beginTransmission(OLED_Display_SSD1306_i2caddr);
    Wire.write((uint8_t)0x00); // Co 0 D/C 0 (Write Command Mode)
    Wire.write(0xD9); // Set precharge
    Wire.endTransmission();

    Wire.beginTransmission(OLED_Display_SSD1306_i2caddr);
    Wire.write((uint8_t)0x00); // Co 0 D/C 0 (Write Command Mode)
    Wire.write(0xF1);
    Wire.endTransmission();

    Wire.beginTransmission(OLED_Display_SSD1306_i2caddr);
    Wire.write((uint8_t)0x00); // Co 0 D/C 0 (Write Command Mode)
    Wire.write(0xDB); // Set vcomdetect
    Wire.write(0x40);
    Wire.write(0xA4); // Disable entire display on
    Wire.write(0xA6); // Set Normal Display
    Wire.write(0x2E); // Set Osc Freq
    Wire.write(0xAF); // Display On
    Wire.endTransmission();
    OLED_Display_SSD1306_Buffer = (uint8_t *)malloc(OLED_Display_SSD1306_Width * ((OLED_Display_SSD1306_Height + 7) / 8));
    OLED_Display_SSD1306_clearDisplay();

}
