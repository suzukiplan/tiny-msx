
#ifndef INCLUDE_TMS9918A_HPP
#define INCLUDE_TMS9918A_HPP

#include "tinymsx_def.h"
#include <string.h>

class TMS9918A
{
  private:
    void* arg;
    void (*detectBlank)(void* arg);
    void (*detectBreak)(void* arg);

  public:
    unsigned short display[256 * 192];
    unsigned short palette[16];

    struct Context {
        int countH;
        int countV;
        unsigned char ram[0x4000];
        unsigned char reg[8];
        unsigned char tmpAddr[2];
        unsigned short addr;
        unsigned char stat;
        unsigned char latch;
        unsigned char readBuffer;
    } ctx;

    TMS9918A()
    {
        memset(palette, 0, sizeof(palette));
        this->reset();
    }

    void reset()
    {
        memset(display, 0, sizeof(display));
        memset(&ctx, 0, sizeof(ctx));
    }

    void initialize(int colorMode, void* arg, void (*detectBlank)(void*), void (*detectBreak)(void*))
    {
        this->arg = arg;
        this->detectBlank = detectBlank;
        this->detectBreak = detectBreak;
        unsigned int rgb[16] = {0x000000, 0x000000, 0x3EB849, 0x74D07D, 0x5955E0, 0x8076F1, 0xB95E51, 0x65DBEF, 0xDB6559, 0xFF897D, 0xCCC35E, 0xDED087, 0x3AA241, 0xB766B5, 0xCCCCCC, 0xFFFFFF};
        switch (colorMode) {
            case TINYMSX_COLOR_MODE_RGB555:
                for (int i = 0; i < 16; i++) {
                    this->palette[i] = 0;
                    this->palette[i] |= (rgb[i] & 0b111110000000000000000000) >> 9;
                    this->palette[i] |= (rgb[i] & 0b000000001111100000000000) >> 6;
                    this->palette[i] |= (rgb[i] & 0b000000000000000011111000) >> 3;
                }
                break;
            case TINYMSX_COLOR_MODE_RGB565:
                for (int i = 0; i < 16; i++) {
                    this->palette[i] = 0;
                    this->palette[i] |= (rgb[i] & 0b111110000000000000000000) >> 8;
                    this->palette[i] |= (rgb[i] & 0b000000001111110000000000) >> 5;
                    this->palette[i] |= (rgb[i] & 0b000000000000000011111000) >> 3;
                }
                break;
            default:
                memset(this->palette, 0, sizeof(this->palette));
        }
    }

    inline int getVideoMode()
    {
        if (ctx.reg[1] & 0b00010000) return 1; // Mode 1
        if (ctx.reg[0] & 0b00000010) return 2; // Mode 2
        if (ctx.reg[1] & 0b00001000) return 3; // Mode 3
        return 0;                              // Mode 0
    }

    inline unsigned short getBackdropColor()
    {
        return palette[ctx.reg[7] & 0b00001111];
    }

    inline void tick()
    {
        this->ctx.countH++;
        if (342 <= this->ctx.countH) {
            this->ctx.countH -= 342;
            this->drawScanline(this->ctx.countV++);
            if (262 <= this->ctx.countV) {
                this->ctx.countV -= 262;
                this->detectBreak(this->arg);
            }
        }
    }

    inline unsigned char readData()
    {
        unsigned char result = this->ctx.readBuffer;
        this->readVideoMemory();
        return result;
    }

    inline unsigned char readStatus()
    {
        unsigned char result = this->ctx.stat;
        this->ctx.stat &= 0b01011111;
        return result;
    }

    inline void writeData(unsigned char value)
    {
        this->ctx.addr &= 0x3FFF;
        this->ctx.readBuffer = value;
        this->ctx.ram[this->ctx.addr++] = this->ctx.readBuffer;
    }

    inline void writeAddress(unsigned char value)
    {
        this->ctx.latch &= 1;
        this->ctx.tmpAddr[this->ctx.latch++] = value;
        if (2 == this->ctx.latch) {
            if (this->ctx.tmpAddr[1] & 0b10000000) {
                this->updateRegister();
            } else if (this->ctx.tmpAddr[1] & 0b01000000) {
                this->updateAddress();
            } else {
                this->updateAddress();
                this->readVideoMemory();
            }
        } else if (1 == this->ctx.latch) {
            this->ctx.addr &= 0xFF00;
            this->ctx.addr |= this->ctx.tmpAddr[0];
        }
    }

    inline void drawScanline(int lineNumber)
    {
        if (lineNumber < 192) {
            switch (this->getVideoMode()) {
                case 0: this->drawScanlineMode0(lineNumber); break;
                case 1: this->drawEmptyScanline(lineNumber); break;
                case 2: this->drawScanlineMode2(lineNumber); break;
                case 4: this->drawScanlineMode3(lineNumber); break;
                default: this->drawEmptyScanline(lineNumber);
            }
            if (191 == lineNumber) {
                this->ctx.stat |= 0x80;
                this->detectBlank(this->arg);
            }
        }
    }

  private:
    inline void updateAddress()
    {
        this->ctx.addr = this->ctx.tmpAddr[1];
        this->ctx.addr <<= 8;
        this->ctx.addr |= this->ctx.tmpAddr[0];
        this->ctx.addr &= 0x3FFF;
    }

    inline void readVideoMemory()
    {
        this->ctx.addr &= 0x3FFF;
        this->ctx.readBuffer = this->ctx.ram[this->ctx.addr++];
    }

    inline void updateRegister()
    {
        this->ctx.reg[this->ctx.tmpAddr[1] & 0b00001111] = this->ctx.tmpAddr[0];
    }

    inline void drawScanlineMode0(int lineNumber)
    {
        int pn = (this->ctx.reg[2] & 0b00001111) << 10;
        int ct = this->ctx.reg[3] << 6;
        int pg = (this->ctx.reg[4] & 0b00000111) << 11;
        int bd = this->ctx.reg[7] & 0b00001111;
        int pixelLine = lineNumber % 8;
        unsigned char* nam = &this->ctx.ram[pn + lineNumber / 8 * 32];
        int cur = lineNumber * 256;
        for (int i = 0; i < 32; i++) {
            unsigned char ptn = this->ctx.ram[pg + nam[i] * 8 + pixelLine];
            unsigned char c = this->ctx.ram[ct + nam[i] / 8];
            unsigned char cc[2];
            cc[1] = (c & 0xF0) >> 4;
            cc[1] = cc[1] ? cc[1] : bd;
            cc[0] = c & 0x0F;
            cc[0] = cc[0] ? cc[0] : bd;
            this->display[cur++] = this->palette[cc[(ptn & 0b10000000) >> 7]];
            this->display[cur++] = this->palette[cc[(ptn & 0b01000000) >> 6]];
            this->display[cur++] = this->palette[cc[(ptn & 0b00100000) >> 5]];
            this->display[cur++] = this->palette[cc[(ptn & 0b00010000) >> 4]];
            this->display[cur++] = this->palette[cc[(ptn & 0b00001000) >> 3]];
            this->display[cur++] = this->palette[cc[(ptn & 0b00000100) >> 2]];
            this->display[cur++] = this->palette[cc[(ptn & 0b00000010) >> 1]];
            this->display[cur++] = this->palette[cc[ptn & 0b00000001]];
        }
        drawSprites(lineNumber);
    }

    inline void drawScanlineMode2(int lineNumber)
    {
        int pn = (this->ctx.reg[2] & 0b00001111) << 10;
        int ct = (this->ctx.reg[3] & 0b10000000) << 6;
        int cmask = this->ctx.reg[3] & 0b01111111;
        cmask <<= 3;
        cmask |= 0x07;
        int pg = (this->ctx.reg[4] & 0b00000100) << 11;
        int pmask = this->ctx.reg[4] & 0b00000011;
        pmask <<= 8;
        pmask |= 0xFF;
        int bd = this->ctx.reg[7] & 0b00001111;
        int pixelLine = lineNumber % 8;
        unsigned char* nam = &this->ctx.ram[pn + lineNumber / 8 * 32];
        int cur = lineNumber * 256;
        int ci = (lineNumber / 64) * 256;
        for (int i = 0; i < 32; i++) {
            unsigned char ptn = this->ctx.ram[pg + ((nam[i] + ci) & pmask) * 8 + pixelLine];
            unsigned char c = this->ctx.ram[ct + ((nam[i] + ci) & cmask) * 8 + pixelLine];
            unsigned char cc[2];
            cc[1] = (c & 0xF0) >> 4;
            cc[1] = cc[1] ? cc[1] : bd;
            cc[0] = c & 0x0F;
            cc[0] = cc[0] ? cc[0] : bd;
            this->display[cur++] = this->palette[cc[(ptn & 0b10000000) >> 7]];
            this->display[cur++] = this->palette[cc[(ptn & 0b01000000) >> 6]];
            this->display[cur++] = this->palette[cc[(ptn & 0b00100000) >> 5]];
            this->display[cur++] = this->palette[cc[(ptn & 0b00010000) >> 4]];
            this->display[cur++] = this->palette[cc[(ptn & 0b00001000) >> 3]];
            this->display[cur++] = this->palette[cc[(ptn & 0b00000100) >> 2]];
            this->display[cur++] = this->palette[cc[(ptn & 0b00000010) >> 1]];
            this->display[cur++] = this->palette[cc[ptn & 0b00000001]];
        }
        drawSprites(lineNumber);
    }

    inline void drawScanlineMode3(int lineNumber)
    {
        // todo: draw Mode 3 characters
        drawSprites(lineNumber);
    }

    inline void drawEmptyScanline(int lineNumber)
    {
        int bd = this->ctx.reg[7] & 0b00001111;
        int cur = lineNumber * 256;
        for (int i = 0; i < 256; i++) {
            this->display[cur++] = palette[bd];
        }
    }

    inline void drawSprites(int lineNumber)
    {
        static const unsigned char bit[8] = {
            0b10000000,
            0b01000000,
            0b00100000,
            0b00010000,
            0b00001000,
            0b00000100,
            0b00000010,
            0b00000001};
        bool si = this->ctx.reg[1] & 0b00000010 ? true : false;
        bool mag = this->ctx.reg[1] & 0b00000001 ? true : false;
        int sa = (this->ctx.reg[5] & 0b01111111) << 7;
        int sg = (this->ctx.reg[6] & 0b00000111) << 11;
        int sn = 0;
        unsigned char dlog[256];
        unsigned char wlog[256];
        memset(dlog, 0, sizeof(dlog));
        memset(wlog, 0, sizeof(wlog));
        for (int i = 0; i < 32; i++) {
            int cur = sa + i * 4;
            unsigned char y = this->ctx.ram[cur++];
            if (208 == y) break;
            unsigned char x = this->ctx.ram[cur++];
            unsigned char ptn = this->ctx.ram[cur++];
            unsigned char col = this->ctx.ram[cur++];
            if (col & 0x80) x -= 32;
            col &= 0b00001111;
            y++;
            if (mag) {
                if (si) {
                    // 16x16 x 2
                    if (y <= lineNumber && lineNumber < y + 32) {
                        sn++;
                        if (5 <= sn) {
                            this->ctx.stat &= 0b11100000;
                            this->ctx.stat |= 0b01000000 | i;
                            break;
                        } else {
                            this->ctx.stat &= 0b11100000;
                            this->ctx.stat |= i;
                        }
                        int pixelLine = lineNumber - y;
                        cur = sg + (ptn & 252) * 8 + pixelLine % 16 / 2 + (pixelLine < 16 ? 0 : 8);
                        int dcur = lineNumber * 256;
                        for (int j = 0; j < 16; j++, x++) {
                            if (wlog[x]) {
                                this->ctx.stat |= 0b00100000;
                            }
                            if (0 == dlog[x]) {
                                if (this->ctx.ram[cur] & bit[j / 2]) {
                                    this->display[dcur + x] = this->palette[col];
                                    dlog[x] = col;
                                    wlog[x] = 1;
                                }
                            }
                        }
                        cur += 8;
                        for (int j = 0; j < 16; j++, x++) {
                            if (wlog[x]) {
                                this->ctx.stat |= 0b00100000;
                            }
                            if (0 == dlog[x]) {
                                if (this->ctx.ram[cur] & bit[j / 2]) {
                                    this->display[dcur + x] = this->palette[col];
                                    dlog[x] = col;
                                    wlog[x] = 1;
                                }
                            }
                        }
                    }
                } else {
                    // 8x8 x 2
                    if (y <= lineNumber && lineNumber < y + 16) {
                        sn++;
                        if (5 <= sn) {
                            this->ctx.stat &= 0b11100000;
                            this->ctx.stat |= 0b01000000 | i;
                            break;
                        } else {
                            this->ctx.stat &= 0b11100000;
                            this->ctx.stat |= i;
                        }
                        cur = sg + ptn * 8 + lineNumber % 8;
                        int dcur = lineNumber * 256;
                        for (int j = 0; j < 16; j++, x++) {
                            if (wlog[x]) {
                                this->ctx.stat |= 0b00100000;
                            }
                            if (0 == dlog[x]) {
                                if (this->ctx.ram[cur] & bit[j / 2]) {
                                    this->display[dcur + x] = this->palette[col];
                                    dlog[x] = col;
                                    wlog[x] = 1;
                                }
                            }
                        }
                    }
                }
            } else {
                if (si) {
                    // 16x16 x 1
                    if (y <= lineNumber && lineNumber < y + 16) {
                        sn++;
                        if (5 <= sn) {
                            this->ctx.stat &= 0b11100000;
                            this->ctx.stat |= 0b01000000 | i;
                            break;
                        } else {
                            this->ctx.stat &= 0b11100000;
                            this->ctx.stat |= i;
                        }
                        int pixelLine = lineNumber - y;
                        cur = sg + (ptn & 252) * 8 + pixelLine % 8 + (pixelLine < 8 ? 0 : 8);
                        int dcur = lineNumber * 256;
                        for (int j = 0; j < 8; j++, x++) {
                            if (wlog[x]) {
                                this->ctx.stat |= 0b00100000;
                            }
                            if (0 == dlog[x]) {
                                if (this->ctx.ram[cur] & bit[j]) {
                                    this->display[dcur + x] = this->palette[col];
                                    dlog[x] = col;
                                    wlog[x] = 1;
                                }
                            }
                        }
                        cur += 16;
                        for (int j = 0; j < 8; j++, x++) {
                            if (wlog[x]) {
                                this->ctx.stat |= 0b00100000;
                            }
                            if (0 == dlog[x]) {
                                if (this->ctx.ram[cur] & bit[j]) {
                                    this->display[dcur + x] = this->palette[col];
                                    dlog[x] = col;
                                    wlog[x] = 1;
                                }
                            }
                        }
                    }
                } else {
                    // 8x8 x 1
                    if (y <= lineNumber && lineNumber < y + 8) {
                        sn++;
                        if (5 <= sn) {
                            this->ctx.stat &= 0b11100000;
                            this->ctx.stat |= 0b01000000 | i;
                            break;
                        } else {
                            this->ctx.stat &= 0b11100000;
                            this->ctx.stat |= i;
                        }
                        cur = sg + ptn * 8 + lineNumber % 8;
                        int dcur = lineNumber * 256;
                        for (int j = 0; j < 8; j++, x++) {
                            if (wlog[x]) {
                                this->ctx.stat |= 0b00100000;
                            }
                            if (0 == dlog[x]) {
                                if (this->ctx.ram[cur] & bit[j]) {
                                    this->display[dcur + x] = this->palette[col];
                                    dlog[x] = col;
                                    wlog[x] = 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
};

#endif // INCLUDE_TMS9918A_HPP
