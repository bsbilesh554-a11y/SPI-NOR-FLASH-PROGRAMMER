Project Title  : SPI NOR Flash Programmer using ESP32
Author         : B.S. Bilesh
Organization   : DLithe Internship Project
Platform       : ESP32 Development Module
IDE            : Arduino IDE
Language       : Embedded C (Arduino Framework) 
Hardware Used:
  1. ESP32 Development Module
  2. W25Q32 SPI NOR Flash Memory
  3. ST7735 TFT Display
  4. Perf PCB
  5. Wi-Fi Connectivity for Blynk IoT

It is to be noted that Blynk IoT is used to create dashboards. THis is intyegrated to the existing code by generating an Auth token while creating the dashboards and then creating the virtual pins for the working of the defined fucntions in the code


There are 3 programs which has different functions and behave differently

(*)The file named COMBINEDCODEBLYNKIOT serves as follows
Description:
This project implements an ESP32-based SPI NOR Flash Programmer using W25Q32 external flash memory through the SPI communication protocol. The system performs flash memory operations such as JEDEC ID reading, status monitoring, write enable (WREN), page programming, data reading, verification, sector erase,full chip erase, and write-with-erase operations and displayed on TFT display interface for local monitoring.
  Additional features include:
 * - CRC/XOR-based data validation
 * - Controlled fault injection and corruption testing
 * - Error detection and analysis
 * - Blynk IoT integration for remote operation
 * - User-driven flash memory management

 * SPI Flash Instruction Set Used:
 * - 0x9F : Read JEDEC ID
 * - 0x05 : Read Status Register
 * - 0x06 : Write Enable (WREN)
 * - 0x02 : Page Program
 * - 0x03 : Read Data
 * - 0x20 : Sector Erase
 * - 0xC7 : Full Chip Erase
 
 Functions of the buttons called from the Blynk IoT Dashboard:
 * V0 - Read JEDEC ID
 * V1 - Read Status Register
 * V2 - Write Enable
 * V3 - Program Flash Data
 * V4 - Verify Written Data
 * V5 - Partial / Full Erase
 * V6 - Write with Erase
 * V7 - CRC Validation
 * V8 - Controlled Error Injection
 * V9 - Error Detection

(*)The file named SPINORFLASHMEMORYBLYNK serves as follows
Description:
This project implements a user-driven SPI NOR Flash Programmer using ESP32 and W25Q32 external flash memory through the SPI communication protocol. The system enables users to manuallyinput and manage flash memory data using a Blynk IoT interface.
The project supports user-controlled memory operations such as:
 * - User data input and programming
 * - Reading stored flash data
 * - Memory usage monitoring
 * - Partial erase operation
 * - Full chip erase
 * - Full flash capacity testing
 * - Memory limit validation
  

 * SPI Flash Instruction Set Used:
 * 0x06  → Write Enable (WREN)
 * 0x05  → Read Status Register
 * 0x02  → Page Program (Write)
 * 0x03  → Read Data
 * 0x20  → Sector Erase (4KB)
 * 0xC7  → Full Chip Erase

Functions of the buttons called from the Blynk IoT Dashboard:
 * V0 → Normal Write
 * V1 → Read Flash Data
 * V2 → Memory Usage
 * V3 → Partial Erase
 * V4 → Full Erase
 * V5 → User Data Input
 * V6 → Full Capacity Test
 * V7 → Over Limit Test


The 3rd program named erasecode completely erases the data which is present inside the memory chip

Thank you
 
