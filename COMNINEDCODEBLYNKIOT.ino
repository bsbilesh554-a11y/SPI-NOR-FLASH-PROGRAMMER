#define BLYNK_TEMPLATE_ID "TMPL39DhvNNgb"
#define BLYNK_TEMPLATE_NAME "SPI NOR FLASH"
#define BLYNK_AUTH_TOKEN "5a8PXwVU3jYu5JjvraucgB10Se5t15SP"

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

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

/* GLOBAL DATA */
uint8_t mid = 0, did1 = 0, did2 = 0;

char textData[] = "DLITHE SPI NOR FLASH";
int textLen;

bool lastWriteValid = false;
int visibleLen = 0;

/* ================= BASIC FLASH ================= */

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

void sectorErase()
{
  writeEnable();

  digitalWrite(FLASH_CS, LOW);
  SPI.transfer(0x20);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  digitalWrite(FLASH_CS, HIGH);

  waitBusy();
}

/* ================= JEDEC ================= */

void readJEDEC()
{
  digitalWrite(TFT_CS, HIGH);
  digitalWrite(FLASH_CS, LOW);

  SPI.transfer(0x9F);

  mid  = SPI.transfer(0x00);
  did1 = SPI.transfer(0x00);
  did2 = SPI.transfer(0x00);

  digitalWrite(FLASH_CS, HIGH);
}

/* ================= PROGRAM METHODS ================= */

void programData()
{
  uint8_t s = readStatus();

  if (!(s & 0x02))
  {
    lastWriteValid = false;
    return;
  }

  digitalWrite(FLASH_CS, LOW);
  SPI.transfer(0x02);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);

  for (int i = 0; i < textLen; i++)
    SPI.transfer(textData[i]);

  digitalWrite(FLASH_CS, HIGH);

  waitBusy();

  lastWriteValid = true;
}

void writeWithErase()
{
  writeEnable();
  sectorErase();

  writeEnable();

  digitalWrite(FLASH_CS, LOW);
  SPI.transfer(0x02);
  SPI.transfer(0);
  SPI.transfer(0);
  SPI.transfer(0);

  for (int i = 0; i < textLen; i++)
    SPI.transfer(textData[i]);

  digitalWrite(FLASH_CS, HIGH);

  waitBusy();
}

/* ================= READ DATA ================= */

void readData(uint8_t *buffer, int len)
{
  digitalWrite(FLASH_CS, LOW);

  SPI.transfer(0x03);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);

  for (int i = 0; i < len; i++)
    buffer[i] = SPI.transfer(0);

  digitalWrite(FLASH_CS, HIGH);
}

/* ================= CRC ================= */

uint8_t crc(uint8_t *data, int len)
{
  uint8_t c = 0;
  for (int i = 0; i < len; i++)
    c ^= data[i];
  return c;
}

String formatByte(uint8_t val)
{
  char c = (val >= 32 && val <= 126) ? val : '.';
  char buf[20];
  sprintf(buf, "%02X (%c)", val, c);
  return String(buf);
}

/* ================= DISPLAY ================= */

void clearScreen()
{
  digitalWrite(FLASH_CS, HIGH);

  tft.fillScreen(ST77XX_WHITE);
  tft.setTextColor(ST77XX_RED);
}

void showCenter(String msg)
{
  clearScreen();
  tft.setTextSize(2);
  tft.setCursor(10, 60);
  tft.print(msg);
}

void showText(int len)
{
  clearScreen();
  tft.setTextSize(2);
  tft.setCursor(5, 60);

  for (int i = 0; i < len; i++)
    tft.print(textData[i]);
}

void showStatus()
{
  uint8_t s = readStatus();
  char buf[6];
  sprintf(buf, "0x%02X", s);
  showCenter(buf);
}

void showJEDEC()
{
  clearScreen();

  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print("JEDEC ID");

  tft.setTextSize(1);

  tft.setCursor(10, 50); tft.print("MID: 0x"); tft.print(mid, HEX);
  tft.setCursor(10, 70); tft.print("D1: 0x"); tft.print(did1, HEX);
  tft.setCursor(10, 90); tft.print("D2: 0x"); tft.print(did2, HEX);

  tft.setCursor(10, 120);
  if (mid == 0xEF) tft.print("Winbond OK");
  else tft.print("Unknown chip");
}

void show3(String l1="", String l2="", String l3="")
{
  clearScreen();
  tft.setTextSize(1);
  tft.setCursor(5,20); tft.print(l1);
  tft.setCursor(5,40); tft.print(l2);
  tft.setCursor(5,60); tft.print(l3);
}

/* ================= VERIFY ================= */

void verifyData()
{
  uint8_t buffer[50];
  readData(buffer, textLen);

  bool match = true;

  for (int i = 0; i < textLen; i++)
  {
    if (buffer[i] != textData[i])
    {
      match = false;
      break;
    }
  }

  if (!lastWriteValid)
  {
    showCenter("NO VALID WRITE");
    return;
  }

  showCenter(match ? "VERIFY PASS" : "VERIFY FAIL");
}

/* ================= BLYNK HANDLERS ================= */

BLYNK_WRITE(V0){ if(param.asInt()) { readJEDEC(); showJEDEC(); } }
BLYNK_WRITE(V1){ if(param.asInt()) showStatus(); }
BLYNK_WRITE(V2){ if(param.asInt()) { writeEnable(); showCenter("WREN ON"); } }
BLYNK_WRITE(V3){ if(param.asInt()) {
  programData();
  if(lastWriteValid){ visibleLen=textLen; showText(visibleLen); }
  else showCenter("WRITE BLOCKED");
}}
BLYNK_WRITE(V4){ if(param.asInt()) verifyData(); }
BLYNK_WRITE(V5){ if(param.asInt()) {
  if(visibleLen>5){ visibleLen-=5; showText(visibleLen); }
  else{ sectorErase(); visibleLen=0; lastWriteValid=false; showCenter("FULL ERASE"); }
}}
BLYNK_WRITE(V6){ if(param.asInt()) { writeWithErase(); show3("WRITTEN:", textData); } }

BLYNK_WRITE(V7){ if(param.asInt()) {
  uint8_t buf[50]; readData(buf,textLen);
  uint8_t crcOriginal=crc((uint8_t*)textData,textLen);
  uint8_t crcRead=crc(buf,textLen);
  String flashText="";
  for(int i=0;i<textLen;i++) flashText+=(char)buf[i];

  show3("ORIGINAL:"+String(textData),
        "FLASH:"+flashText,
        "CRC O:"+String(crcOriginal,HEX)+" R:"+String(crcRead,HEX));

  delay(2500);
  showCenter((crcOriginal==crcRead)?"STATUS: PASS":"STATUS: FAIL");
}}

BLYNK_WRITE(V8){ if(param.asInt()) {
  uint8_t buf[50]; readData(buf,textLen);
  int idx=random(0,textLen);

  uint8_t oldVal=buf[idx];
  uint8_t newVal=oldVal^0xFF;

  writeEnable();

  digitalWrite(FLASH_CS, LOW);
  SPI.transfer(0x02);
  SPI.transfer(0);
  SPI.transfer(0);
  SPI.transfer(idx);
  SPI.transfer(newVal);
  digitalWrite(FLASH_CS, HIGH);

  waitBusy();

  readData(buf,textLen);

  String flashText="";
  for(int i=0;i<textLen;i++)
  {
    char c=(buf[i]>=32&&buf[i]<=126)?buf[i]:'.';
    flashText+=c;
  }

  show3("INDEX:"+String(idx),
        "OLD:"+formatByte(oldVal),
        "NEW:"+formatByte(newVal));

  delay(2500);
  show3("FLASH DATA:",flashText,"");
}}

BLYNK_WRITE(V9){ if(param.asInt()) {
  uint8_t buf[50]; readData(buf,textLen);

  String flashText="";
  for(int i=0;i<textLen;i++)
  {
    char c=(buf[i]>=32&&buf[i]<=126)?buf[i]:'.';
    flashText+=c;
  }

  bool found=false;

  for(int i=0;i<textLen;i++)
  {
    if(buf[i]!=textData[i])
    {
      found=true;
      show3("ERR POS:"+String(i),
            "EXP:"+formatByte(textData[i]),
            "ACT:"+formatByte(buf[i]));
      delay(2000);
    }
  }

  if(found)
    show3("ORIGINAL:",String(textData),"FLASH:"+flashText);
  else
    show3("NO ERROR","","");
}}

/* ================= SETUP ================= */

void setup()
{
  Serial.begin(115200);

  SPI.begin(18,21,23);

  pinMode(TFT_CS, OUTPUT);
  pinMode(FLASH_CS, OUTPUT);

  digitalWrite(TFT_CS, HIGH);
  digitalWrite(FLASH_CS, HIGH);

  textLen = strlen(textData);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);

  showCenter("CONNECTING");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  showCenter("BLYNK READY");
}

/* ================= LOOP ================= */

void loop()
{
  Blynk.run();
}