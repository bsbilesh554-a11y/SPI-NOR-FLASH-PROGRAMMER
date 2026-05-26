#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

/* TFT */
#define TFT_CS 15
#define TFT_DC 4
#define TFT_RST 2

/* FLASH */
#define FLASH_CS 19

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

/* ================= FLASH ================= */

uint8_t readStatus()
{
  digitalWrite(FLASH_CS, LOW);
  SPI.transfer(0x05);
  uint8_t s = SPI.transfer(0);
  digitalWrite(FLASH_CS, HIGH);
  return s;
}

void writeEnable()
{
  digitalWrite(FLASH_CS, LOW);
  SPI.transfer(0x06);
  digitalWrite(FLASH_CS, HIGH);
}

void waitBusy()
{
  while (readStatus() & 0x01);
}

/* FULL CHIP ERASE (4KB SECTOR) */
void eraseAll()
{
  writeEnable();

  digitalWrite(FLASH_CS, LOW);
  SPI.transfer(0x20);   // sector erase
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  digitalWrite(FLASH_CS, HIGH);

  waitBusy();
}

/* ================= DISPLAY ================= */

void showMessage(const char *msg)
{
  tft.fillScreen(ST77XX_WHITE);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  tft.setCursor(10, 60);
  tft.print(msg);
}

/* ================= SETUP ================= */


void setup()
{
  SPI.begin();

  pinMode(FLASH_CS, OUTPUT);
  digitalWrite(FLASH_CS, HIGH);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);

  showMessage("ERASING...");

  eraseAll();

  showMessage("ERASE DONE");
}

void loop()
{
  // Nothing here
}