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
#ifndef INCLUDE_TINYMSX_GW_H
#define INCLUDE_TINYMSX_GW_H
#include <stdio.h>
#include "tinymsx_def.h"

#ifdef __cplusplus
extern "C" {
#endif

void* tinymsx_create(int type, const void* rom, size_t romSize, size_t ramSize, int colorMode);
void tinymsx_destroy(const void* context);
void tinymsx_reset(const void* context);
void tinymsx_tick(const void* context, unsigned char pad1, unsigned char pad2);
unsigned short* tinymsx_display(const void* context);
void* tinymsx_sound(const void* context, size_t* size);
const void* tinymsx_save(const void* context, size_t* size);
void tinymsx_load(const void* context, const void* data, size_t size);
unsigned short tinymsx_backdrop(const void* context);
void tinymsx_load_bios_msx1_main(const void* context, void* bios, size_t size);
void tinymsx_load_bios_msx1_logo(const void* context, void* bios, size_t size);
void tinymsx_setup_special_key1(const void* context, unsigned char c, int isTenKey);
void tinymsx_setup_special_key2(const void* context, unsigned char c, int isTenKey);
unsigned char* tinymsx_get_vram(const void* context);
unsigned short tinymsx_get_nameTableAddr(const void* context);
unsigned char* tinymsx_get_ram(const void* context);

#ifdef __cplusplus
};
#endif

#endif
