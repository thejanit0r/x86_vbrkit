/*

    This file is part of x86_vbrkit.

    Copyright 2017 / the`janitor / < email: base64dec(dGhlLmphbml0b3JAcHJvdG9ubWFpbC5jb20=) >

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/
#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <types.h>

typedef struct 
{
    char        signature[4];
    uint16_t    version;
    uint32_t    oem;
    uint32_t    capabilities;
    uint32_t    video_modes;
    uint16_t    video_memory;
    uint16_t    software_rev;
    uint32_t    vendor;
    uint32_t    product_name;
    uint32_t    product_rev;    
    char        reserved[222];  
    char        oem_data[256];

} __attribute__ ((packed)) vbe_info_t;

typedef struct 
{
    uint16_t    attributes;
    uint8_t     win_a_attribs;
    uint8_t     win_b_attribs;
    uint16_t    win_granularity;
    uint16_t    win_size;
    uint16_t    win_a_segment;
    uint16_t    win_b_segment;
    uint32_t    win_func_ptr;
    uint16_t    bytes_per_scanline; 
    uint16_t    x_res;
    uint16_t    y_res;
    uint8_t     x_char_size;
    uint8_t     y_char_size;
    uint8_t     planes;
    uint8_t     bpp;
    uint8_t     banks;
    uint8_t     memory_model;
    uint8_t     bank_size;
    uint8_t     image_pages;
    uint8_t     reserved_page;
    uint8_t     red_mask_size;
    uint8_t     red_mask_pos;
    uint8_t     green_mask_size;
    uint8_t     green_mask_pos;
    uint8_t     blue_mask_size;
    uint8_t     blue_mask_pos;
    uint8_t     reserved_mask_size;
    uint8_t     reserved_mask_pos;
    uint8_t     direct_color_attributes;
    uint32_t    phys_base_ptr; 
    uint32_t    offscreen_mem_offset;
    uint16_t    offscreen_mem_size;

} __attribute__ ((packed)) vesa_mode_info_t;

typedef struct
{
    uint32_t    width;
    uint32_t    height;
    uint8_t     bpp;
    uint8_t*    ptr;

} bmp_t;

typedef struct
{
    uint32_t    width; /* bits */
    uint32_t    height; /* bytes */
    uint8_t     scale;
    uint8_t*    ptr;

} font_t;


/******************************************************************************/

void video_init(void);

void vesa_init(void);

bool_t vesa_get_info(void);

bool_t vesa_set_mem_window(int window, int position);

bool_t vesa_get_mode_info(int mode);

uint16_t vesa_find_mode(int w, int h, int bpp);

bool_t vesa_set_mode(int w, int h, int bpp);

void vesa_get_resolution(uint32_t* w, uint32_t* h);

void vesa_set_color(uint8_t* ptr, uint8_t bpp, uint8_t r, uint8_t g, uint8_t b);

void vesa_get_color(uint8_t* ptr, uint8_t bpp, uint8_t* r, uint8_t* g, uint8_t* b);

void vesa_set_pixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b);

void font_init(font_t* font, uint32_t width, uint32_t height, uint8_t scale, uint8_t* ptr);

void bmp_init(bmp_t* bmp, uint32_t width, uint32_t height, uint8_t bpp, uint8_t* ptr);

void bmp_get_pixel(bmp_t* bmp, uint32_t x, uint32_t y, uint32_t* rgb);

void bmp_set_pixel(bmp_t* bmp, uint32_t x, uint32_t y, uint32_t rgb);

void bmp_rect(bmp_t* bmp, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t rgb);

void bmp_puts(bmp_t* bmp, font_t* font, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg, char* str);

void bmp_bitblt(bmp_t* src, bmp_t* dst, uint32_t dst_x, uint32_t dst_y);

void bmp_to_framebuffer(bmp_t* bmp);

void bmp_from_ppm(bmp_t* bmp, uint8_t* ppm);

#endif //_VIDEO_H_ 


