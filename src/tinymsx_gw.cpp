/**
 * SUZUKI PLAN - TinyMSX - wrapper library for C
 * -----------------------------------------------------------------------------
 * The MIT License (MIT)
 * 
 * Copyright (c) 2020 Yoji Suzuki.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * -----------------------------------------------------------------------------
 */
#include "tinymsx.h"
#include "tinymsx_gw.h"

void* tinymsx_create(int type, const void* rom, size_t romSize, size_t ramSize, int colorMode) { return new TinyMSX(type, rom, romSize, ramSize, colorMode); }
void tinymsx_destroy(const void* context) { delete (TinyMSX*)context; }
void tinymsx_reset(const void* context) { ((TinyMSX*)context)->reset(); }
void tinymsx_tick(const void* context, unsigned char pad1, unsigned char pad2) { ((TinyMSX*)context)->tick(pad1, pad2); }
unsigned short* tinymsx_display(const void* context) { return ((TinyMSX*)context)->getDisplayBuffer(); }
void* tinymsx_sound(const void* context, size_t* size) { return ((TinyMSX*)context)->getSoundBuffer(size); }
const void* tinymsx_save(const void* context, size_t* size) { return ((TinyMSX*)context)->saveState(size); }
void tinymsx_load(const void* context, const void* data, size_t size) { ((TinyMSX*)context)->loadState(data, size); }
unsigned short tinymsx_backdrop(const void* context) { return ((TinyMSX*)context)->getBackdropColor(); }
void tinymsx_load_bios_msx1_main(const void* context, void* bios, size_t size) { ((TinyMSX*)context)->loadBiosFromMemory(bios, size); }
void tinymsx_setup_special_key1(const void* context, unsigned char c, int isTenKey) { ((TinyMSX*)context)->setupSpecialKey1(c, isTenKey); }
void tinymsx_setup_special_key2(const void* context, unsigned char c, int isTenKey) { ((TinyMSX*)context)->setupSpecialKey2(c, isTenKey); }
unsigned char* tinymsx_get_vram(const void* context) { return ((TinyMSX*)context)->tms9918->ctx.ram; }
unsigned short tinymsx_get_nameTableAddr(const void* context) { return ((TinyMSX*)context)->tms9918->ctx.reg[2] << 10; }
unsigned char* tinymsx_get_ram(const void* context) { return ((TinyMSX*)context)->ram; }
