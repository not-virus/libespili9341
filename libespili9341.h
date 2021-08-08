/**
 * libesp9341.h
 * 
 * Written by Cameron Krueger
 * 8 August 2021
 * 
 * Header file for the libespili9341 display driver library
 */

#ifndef __LIBESPILI9341_H__
#define __LIBESPILI9341_H__

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_err.h"

#include "driver/gpio.h"
#include "driver/spi.h"

// This library uses SPI to communicate with the display. These values must be
//  defined for the library to work.
#ifndef ILI9341_PIN_MISO
#error ILI9341_PIN_MISO not defined
#endif

#ifndef ILI9341_PIN_MOSI
#error ILI9341_PIN_MOSI not defined
#endif

#ifndef ILI9341_PIN_CLK
#error ILI9341_PIN_CLK not defined
#endif

#ifndef ILI9341_PIN_DC
#error ILI9341_PIN_DC not defined
#endif

// These aren't strictly necessary, but may be useful later or for other
//  applications
/*#ifndef ILI9341_PIN_CS
#error ILI9341_PIN_CS not defined
#endif

#ifndef ILI9341_PIN_RST
#error ILI9341_PIN_RST not defined
#endif

#ifndef ILI9341_PIN_LITE
#error ILI9341_PIN_LITE not defined
#endif
*/

// ILI9341 Level 1 command set
#define ILI9341_NOP 0x00 // No operation
#define ILI9341_SWRESET 0x01 // Software reset
#define ILI9341_RDDID 0x09 // Read display identification
#define ILI9341_RDDPM 0x0A // Read display power mode
#define ILI9341_RDDMADCTL 0x0B // Read display MADCTL
#define ILI9341_RDDPF 0x0C // Read display pixel format
#define ILI9341_RDDIF 0x0D // Read display image format
#define ILI9341_RDDSM 0x0E // Read display signal mode
#define ILI9341_RDDSDR 0x0F // Read display self-diagnostic result

#define ILI9341_SLEEPIN 0x10 // Enter sleep mode (named to match sleep out)
#define ILI9341_SLEEPOUT 0x11 // Sleep OUT
#define ILI9341_PARTON 0x12 // Partial mode ON
#define ILI9341_NORMON 0x13 // Normal display mode ON

#define ILI9341_INVOFF 0x20 // Display inversion OFF
#define ILI9341_INVON 0x21 // Display inversion ON
#define ILI9341_GAMSET 0x26 // Gamma set
#define ILI9341_DISPOFF 0x28 // Display OFF
#define ILI9341_DISPON 0x29 // Display ON

#define ILI9341_CASET 0x2A // Column address set register
#define ILI9341_PASET 0x2B // Page address set register
#define ILI9341_RAMEMWR 0x2C // Memory write
#define ILI9341_COLORSET 0x2D // Color set
#define ILI9341_RAMEMRD 0x2E // Memory read

#define ILI9341_PARTAR 0x30 // Partial area
#define ILI9341_VSCRDEF 0x33 // Vertical scrolling definition
#define ILI9341_TELOFF 0x34 // Tearing effect line OFF
#define ILI9341_TELON 0x35 // Tearing effect line ON
#define ILI9341_MADCTL 0x36 // Memory access control (called MADCTL in datasheet)
#define ILI9341_VSSAD 0x37 // Vertical scrolling start address
#define ILI9341_IDLEOFF 0x38 // Idle mode OFF
#define ILI9341_IDLEON 0x39 // Idle mode ON
#define ILI9341_PIXFMTSET 0x3A // Pixel format set
#define ILI9341_WRMEMCONT 0x3C // Write memory continue
#define ILI9341_RDMEMCONT 0x3E // Read memory continue
#define ILI9341_SETTEARSLN 0x44 // Set tear scanline
#define ILI9341_GETSLN 0x45 // Get scanline

#define ILI9341_WRDISPBRT 0x51 // Write display brightness
#define ILI9341_RDDISPBRT 0x52 // Read display brightness
#define ILI9341_WRCTLDISP 0x53 // Write CTRL display
#define ILI9341_RDCTLDISP 0x54 // Read CTRL display
#define ILI9341_WRCTABRTCTL 0x55 // Write content adaptive brightness control
#define ILI9341_RDCTABRTCTL 0x56 // Read content adaptive brightness control
#define ILI9341_WRCABCMINBRT 0x5E // Write CABC minimum brightness
#define ILI9341_RDCABCMINBRT 0x5F // Read CABC minimum brightness

#define ILI9341_RDID1 0xDA // Read ID1
#define ILI9341_RDID2 0xDB // Read ID2
#define ILI9341_RDID3 0xDC // Read ID3

// Level 2 command set
#define ILI9341_RGBIFSIGCTL 0xB0 // RGB interface signal control
#define ILI9341_FRMCTLNORM 0xB1 // Frame control (in Normal Mode)
#define ILI9341_FRMCTLPART 0xB2 // Frame control (in Partial Mode)
#define ILI9341_INVCTL 0xB4 // Display inversion control
#define ILI9341_BLNKPRCHCTL 0xB5 // Blanking porch control
#define ILI9341_DISPFNCTL 0xB6 // Display function control
#define ILI9341_ENTSET 0xB7 // Entry mode set

#define ILI9341_BLTCNT1 0xB8 // Backlight control 1
#define ILI9341_BLTCNT2 0xB9 // Backlight control 2
#define ILI9341_BLTCNT3 0xBA // Backlight control 3
#define ILI9341_BLTCNT4 0xBB // Backlight control 4
#define ILI9341_BLTCNT5 0xBC // Backlight control 5
#define ILI9341_BLTCNT6 0xBD // Backlight control 6
#define ILI9341_BLTCNT7 0xBE // Backlight control 7
#define ILI9341_BLTCNT8 0xBF // Backlight control 8

#define ILI9341_PWRCTL1 0xC0 // Power control 1
#define ILI9341_PWRCTL2 0xC1 // Power control 2

#define ILI9341_VCOMCTL1 0xC5 // VCOM control 1
#define ILI9341_VCOMCTL2 0xC7 // VCOM control 2

#define ILI9341_NVMEMWR 0xD0 // NV memory write
#define ILI9341_NVMEMPK 0xD1 // NV memory protection key
#define ILI9341_NVMEMSTRD 0xD2 // NV memory status read

#define ILI9341_RDID4 0xD3 // Read ID4

#define ILI9341_PGAM 0xE0 // Positive gamma control
#define ILI9341_NGAM 0xE1 // Negative gamma control
#define ILI9341_DIGGAMCTL1 0xE2 // Digital gamma control 1
#define ILI9341_DIGGAMCTL2 0xE3 // Digital gamma control 2

#define ILI9341_IFACECTL 0xF6 // Interface control

// Extended register command set
#define ILI9341_PWRCTLA 0xCB // Power control A
#define ILI9341_PWRCTLB 0xCF // Power control B
#define ILI9341_DRVTMGCTLA1 0xE8 // Driver timing control A
#define ILI9341_DRVTMGCTLA2 0xE9 //    "      "      "    "
#define ILI9341_DRVTMGCTLB 0xEA // Driver timing control B
#define ILI9341_POSCTL 0xED // Power on sequence control
#define ILI9341_EN3G 0xF2 // Enable 3G
#define ILI9341_PRCTL 0xF7 // Pump ratio control

esp_error_t init_display();

#endif