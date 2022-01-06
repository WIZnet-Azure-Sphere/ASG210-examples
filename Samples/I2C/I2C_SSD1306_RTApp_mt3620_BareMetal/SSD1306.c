/* Copyright (c) Codethink Ltd. All rights reserved.
   Licensed under the MIT License. */

#include "SSD1306.h"

typedef union __attribute__((__packed__))
{
    struct __attribute__((__packed__))
    {
        unsigned reserved : 6;
        bool     isData   : 1;
        bool     cont     : 1;
    };
    uint8_t mask;
} Ssd1306_I2CHeader;

typedef struct __attribute__((__packed__))
{
    Ssd1306_I2CHeader header;
    uint8_t           data;
} Ssd1306_Packet;

#define SSD1306_MAX_DATA_WRITE 1024
static bool Ssd1306_Write(i2c_num bus_num, bool isData, const void *data, uintptr_t size)
{
    static __attribute__((section(".sysram"))) Ssd1306_Packet cmd[SSD1306_MAX_DATA_WRITE];
    if (size > SSD1306_MAX_DATA_WRITE)
    {
        return false;
    }

    uintptr_t i;
    for (i = 0; i < size; i++)
    {
        cmd[i].header = (Ssd1306_I2CHeader)
        {
            .isData = isData, .cont = (i < (size - 1))
        };
        cmd[i].data   = ((uint8_t *)data)[i];
    }

    return (mtk_os_hal_i2c_write(bus_num, SSD1306_ADDRESS, cmd, (size * sizeof(cmd[0]))) == I2C_OK);
}

//Static functions for hardware configuration
static bool SSD1306_SetDisplayClockDiv(i2c_num bus_num, uint8_t value)
{
    uint8_t packet[] = { SSD1306_CMD_SETDISPLAYCLOCKDIV, value };
    return Ssd1306_Write(bus_num, false, packet, sizeof(packet));
}

static bool SSD1306_SetMultiplex(i2c_num bus_num, uint8_t value)
{
    uint8_t packet[] = { SSD1306_CMD_SETMULTIPLEX, value };
    return Ssd1306_Write(bus_num, false, packet, sizeof(packet));
}

static bool SSD1306_SetDisplayOffset(i2c_num bus_num, uint8_t value)
{
    uint8_t packet[] = { SSD1306_CMD_SETDISPLAYOFFSET, value };
    return Ssd1306_Write(bus_num, false, packet, sizeof(packet));
}


static bool SSD1306_SetChargePump(i2c_num bus_num, uint8_t value)
{
    uint8_t packet[] = { SSD1306_CMD_CHARGEPUMP, value };
    return Ssd1306_Write(bus_num, false, packet, sizeof(packet));
}

static bool SSD1306_SetStartLine(i2c_num bus_num, unsigned offset)
{
    if (offset >= SSD1306_HEIGHT)
    {
        return false;
    }

    uint8_t value = SSD1306_CMD_SETSTARTLINE + offset;
    return Ssd1306_Write(bus_num, false, &value, sizeof(value));
}

static bool SSD1306_SetMemoryMode(i2c_num bus_num, uint8_t value)
{
    uint8_t packet[] = { SSD1306_CMD_MEMORYMODE, value };
    return Ssd1306_Write(bus_num, false, packet, sizeof(packet));
}

static bool SSD1306_SetSegRemap(i2c_num bus_num, bool remapTrue)
{
    uint8_t value = SSD1306_CMD_SETSEGMENTREMAP | remapTrue;
    return Ssd1306_Write(bus_num, false, &value, sizeof(value));
}

static bool SSD1306_SetComScanDir(i2c_num bus_num, bool scanDirTrue)
{
    uint8_t value = SSD1306_CMD_COMSCANDIR | (scanDirTrue ? 8 : 0);
    return Ssd1306_Write(bus_num, false, &value, sizeof(value));
}

static bool SSD1306_SetComPins(i2c_num bus_num, uint8_t value)
{
    uint8_t packet[] = { SSD1306_CMD_SETCOMPINS, value };
    return Ssd1306_Write(bus_num, false, packet, sizeof(packet));
}

static bool SSD1306_SetPreCharge(i2c_num bus_num, uint8_t value)
{
    uint8_t packet[] = { SSD1306_CMD_SETPRECHARGE, value };
    return Ssd1306_Write(bus_num, false, packet, sizeof(packet));
}

static bool SSD1306_SetVComDetect(i2c_num bus_num, uint8_t value)
{
    uint8_t packet[] = { SSD1306_CMD_SETVCOMDETECT, value };
    return Ssd1306_Write(bus_num, false, packet, sizeof(packet));
}
//End of hardware configuration functions

//Sets the start and end column address for the data that is being sent
static bool SSD1306_SetColumnAddress(i2c_num bus_num, uint8_t columnStart, uint8_t columnEnd)
{
    if ((columnStart >= SSD1306_WIDTH) || (columnEnd >= SSD1306_WIDTH))
    {
        return false;
    }
    uint8_t packet[] = { SSD1306_CMD_SETCOLUMNADDR, columnStart, columnEnd };
    return Ssd1306_Write(bus_num, false, packet, sizeof(packet));
}

bool SSD1306_WriteFullBuffer(i2c_num bus_num, const void *data, uintptr_t size)
{
    if (!SSD1306_SetColumnAddress(bus_num, 0, (SSD1306_WIDTH - 1)))
    {
        return false;
    }
    return Ssd1306_Write(bus_num, true, data, size);
}

bool SSD1306_SetDisplayOnOff(i2c_num bus_num, bool displayOnTrue)
{
    uint8_t value = SSD1306_CMD_DISPLAYONOFF | displayOnTrue;
    return Ssd1306_Write(bus_num, false, &value, sizeof(value));
}

bool SSD1306_SetContrast(i2c_num bus_num, uint8_t value)
{
    uint8_t packet[] = { SSD1306_CMD_SETCONTRAST, value };
    return Ssd1306_Write(bus_num, false, packet, sizeof(packet));
}

bool SSD1306_SetDisplayAllOn(i2c_num bus_num, bool displayAllOnTrue)
{
    int8_t value = SSD1306_CMD_DISPLAYALLON | displayAllOnTrue;
    return Ssd1306_Write(bus_num, false, &value, sizeof(value));
}

bool SSD1306_SetDisplayInverse(i2c_num bus_num, bool inverseTrue)
{
    uint8_t value = SSD1306_CMD_NORMALDISPLAY | inverseTrue;
    return Ssd1306_Write(bus_num, false, &value, sizeof(value));
}

bool SSD1306_ActivateScroll(i2c_num bus_num, bool activateScrollTrue)
{
    uint8_t value = SSD1306_CMD_SETSCROLL | activateScrollTrue;
    return Ssd1306_Write(bus_num, false, &value, sizeof(value));
}

bool Ssd1306_Init(i2c_num bus_num)
{
    bool success = true;

    success = success && SSD1306_SetDisplayOnOff(bus_num, false);
    success = success && SSD1306_SetDisplayClockDiv(bus_num, SSD1306_CTRL_DISPLAYCLOCKDIV);
    success = success && SSD1306_SetMultiplex(bus_num, SSD1306_CTRL_MULTIPLEX);
    success = success && SSD1306_SetDisplayOffset(bus_num, SSD1306_CTRL_DISPLAYOFFSET);
    success = success && SSD1306_SetStartLine(bus_num, SSD1306_CTRL_STARTLINE);
    success = success && SSD1306_SetChargePump(bus_num, SSD1306_CTRL_CHARGEPUMP);
    success = success && SSD1306_SetMemoryMode(bus_num, SSD1306_CTRL_MEMORYMODE);
    success = success && SSD1306_SetSegRemap(bus_num, SSD1306_CTRL_SEGREMAP);
    success = success && SSD1306_SetComScanDir(bus_num, SSD1306_CTRL_COMSCANDIR);
    success = success && SSD1306_SetComPins(bus_num, SSD1306_CTRL_COMPIMS);
    success = success && SSD1306_SetContrast(bus_num, SSD1306_CTRL_CONTRAST);
    success = success && SSD1306_SetPreCharge(bus_num, SSD1306_CTRL_PRECHARGE);
    success = success && SSD1306_SetVComDetect(bus_num, SSD1306_CTRL_VCOMDETECT);
    success = success && SSD1306_SetDisplayAllOn(bus_num, false);
    success = success && SSD1306_SetDisplayInverse(bus_num, SSD1306_CTRL_DISPLAYINVERSE);
    success = success && SSD1306_ActivateScroll(bus_num, SSD1306_CTRL_ACTIVATESCROLL);
    success = success && SSD1306_SetDisplayOnOff(bus_num, true);

    return success;
}

