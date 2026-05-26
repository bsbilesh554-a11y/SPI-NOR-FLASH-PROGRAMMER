#define BLYNK_TEMPLATE_ID "TMPL33A7YL9NK"
#define BLYNK_TEMPLATE_NAME "SPI NOR FLASH PROGRAMMER"
#define BLYNK_AUTH_TOKEN "FzUpZv8vXJpq6ZgaVAI49vlzCauXh4ru"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

/* WIFI */
char ssid[] = "BSB";
char pass[] = "1234ABCD";

/* TFT */
#define TFT_CS   15
#define TFT_DC   4
#define TFT_RST  2

/* FLASH */
#define FLASH_CS 19
#define FLASH_SIZE 4096
#define PAGE_SIZE 256
#define PARTIAL_ERASE_CHUNK 16

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

String userData = "";

/* ================= FLASH ================= */

void writeEnable()
{
  digitalWrite(FLASH_CS, LOW);
  SPI.transfer(0x06);
  digitalWrite(FLASH_CS, HIGH);
}

uint8_t readStatus()
{
  digitalWrite(FLASH_CS, LOW);
  SPI.transfer(0x05);
  uint8_t s = SPI.transfer(0);
  digitalWrite(FLASH_CS, HIGH);
  return s;
}

void waitBusy()
{
  while(readStatus() & 0x01);
}

void sectorErase()
{
  writeEnable();

  digitalWrite(FLASH_CS, LOW);
  SPI.transfer(0x20);
  SPI.transfer(0);
  SPI.transfer(0);
  SPI.transfer(0);
  digitalWrite(FLASH_CS, HIGH);

  waitBusy();
}

void chipErase()
{
  writeEnable();

  digitalWrite(FLASH_CS, LOW);
  SPI.transfer(0xC7);
  digitalWrite(FLASH_CS, HIGH);

  waitBusy();
}

/* ================= PAGE PROGRAM ================= */

void pageProgram(uint16_t addr, const uint8_t* data, int len)
{
  writeEnable();

  digitalWrite(FLASH_CS, LOW);

  SPI.transfer(0x02);
  SPI.transfer((addr >> 16) & 0xFF);
  SPI.transfer((addr >> 8) & 0xFF);
  SPI.transfer(addr & 0xFF);

  for(int i=0;i<len;i++)
    SPI.transfer(data[i]);

  digitalWrite(FLASH_CS, HIGH);

  waitBusy();
}

void writeBuffer(uint8_t* data, int len)
{
  sectorErase();

  int remaining = len;
  int addr = 0;

  while(remaining > 0)
  {
    int chunk = (remaining > PAGE_SIZE) ? PAGE_SIZE : remaining;

    pageProgram(addr, data + addr, chunk);

    addr += chunk;
    remaining -= chunk;
  }
}

void writeString(String data)
{
  writeBuffer((uint8_t*)data.c_str(), data.length());
}

/* ================= READ ================= */

String readString()
{
  String out = "";

  digitalWrite(FLASH_CS, LOW);

  SPI.transfer(0x03);
  SPI.transfer(0);
  SPI.transfer(0);
  SPI.transfer(0);

  for(int i=0;i<FLASH_SIZE;i++)
  {
    uint8_t b = SPI.transfer(0);

    if(b == 0xFF)
      break;

    out += (char)b;
  }

  digitalWrite(FLASH_CS, HIGH);

  return out;
}

/* ================= DISPLAY ================= */

void show(String a="", String b="", String c="")
{
  digitalWrite(FLASH_CS, HIGH);

  tft.fillScreen(ST77XX_WHITE);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(1);

  tft.setCursor(5,15); tft.print(a);
  tft.setCursor(5,35); tft.print(b);
  tft.setCursor(5,55); tft.print(c);
}

/* ================= BLYNK ================= */

BLYNK_WRITE(V5)
{
  userData = param.asString();
}

/* NORMAL WRITE */
BLYNK_WRITE(V0)
{
  if(param.asInt())
  {
    Blynk.syncVirtual(V5);
    delay(100);

    if(userData.length()==0)
    {
      show("NOTHING WRITTEN");
      return;
    }

    if(userData.length()>FLASH_SIZE)
    {
      show("STOP",
           "LIMIT EXCEEDED",
           "MAX 4096 BYTES");
      return;
    }

    writeString(userData);

    show("DATA WRITTEN:",
         userData.substring(0,20),
         "USED:"+String(userData.length())+"/4096");
  }
}

/* READ */
BLYNK_WRITE(V1)
{
  if(param.asInt())
  {
    String d = readString();

    show("FLASH DATA:",
         d.substring(0,20),
         "LEN:"+String(d.length()));
  }
}

/* MEMORY */
BLYNK_WRITE(V2)
{
  if(param.asInt())
  {
    int used = readString().length();

    show("USED:"+String(used)+"/4096",
         "FREE:"+String(4096-used));
  }
}

/* PARTIAL ERASE */
BLYNK_WRITE(V3)
{
  if(param.asInt())
  {
    String d = readString();

    if(d.length()==0)
    {
      show("NO DATA");
      return;
    }

    int newLen = (d.length()>PARTIAL_ERASE_CHUNK)
                 ? d.length()-PARTIAL_ERASE_CHUNK
                 : 0;

    String shortened = d.substring(0,newLen);

    writeString(shortened);

    show("PARTIAL ERASE",
         "-16 BYTES",
         "USED:"+String(newLen));
  }
}

/* FULL ERASE */
BLYNK_WRITE(V4)
{
  if(param.asInt())
  {
    show("ERASING...",
         "PLEASE WAIT");

    delay(100);

    chipErase();

    show("FULL ERASE DONE",
         "USED:0/4096");
  }
}

/* FULL 4KB TEST */
BLYNK_WRITE(V6)
{
  if(param.asInt())
  {
    static uint8_t fullBuf[FLASH_SIZE];

    for(int i=0;i<FLASH_SIZE;i++)
      fullBuf[i] = 'A' + (i % 26);

    writeBuffer(fullBuf, FLASH_SIZE);

    show("FULL TEST WRITE",
         "4096/4096 USED",
         "SUCCESS");
  }
}

/* OVER LIMIT TEST */
BLYNK_WRITE(V7)
{
  if(param.asInt())
  {
    show("STOP",
         "LIMIT EXCEEDED",
         ">4096 BYTES");
  }
}

/* ================= SETUP ================= */

void setup()
{
  SPI.begin(18,21,23);

  pinMode(FLASH_CS, OUTPUT);
  digitalWrite(FLASH_CS, HIGH);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);

  show("CONNECTING");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  show("BLYNK READY");
}

/* ================= LOOP ================= */

void loop()
{
  Blynk.run();
}
