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
#include <shared.h>
#include <mem.h>
#include <libc.h>
#include <console.h>
#include <video.h>
#include <types.h>
#include <stdarg.h>
#include <bios.h>

/******************************************************************************/

static vbe_info_t* vbe_info;
static vesa_mode_info_t* vesa_mode_info;

/******************************************************************************/

void
video_init(void)
{
    /* set default video mode */

    rmode_ctx_t ctx;

    /* 12h: Graphic, 640x480, 8x16, 4-bit color */
    ctx.al = 0x12;
    ctx.ah = 0x00;

    ldr_bios_call(BIOS_SVC_VIDEO, &ctx);
}

/******************************************************************************/

void
vesa_init(void)
{
    vbe_info = NULL;
    vesa_mode_info = NULL;
}

bool_t
vesa_get_info(void)
{
    /* VESA BIOS Extensions (VBE) */

    rmode_ctx_t ctx;

    if(vbe_info == NULL)
    {
        vbe_info = (vbe_info_t *)malloc(sizeof(vbe_info_t));
    }
    
    /* check for VESA support */
    memcpy(vbe_info->signature, "VBE2", 4);

    ctx.ax = 0x4F00;
    ctx.es = (((uint32_t)vbe_info >> 4) & 0xF000); /* seg */
    ctx.di = (((uint32_t)vbe_info >> 0) & 0xFFFF); /* offs */

    ldr_bios_call(BIOS_SVC_VIDEO, &ctx);

#if 0

    printf(FG_LMAGENTA, "AX=%x\n", ctx.eax & 0xFFFF);
    printf(FG_LMAGENTA, "Version=%x\n", vbe_info->version);
    printf(FG_LMAGENTA, "%c%c%c%c\n", 
        vbe_info->signature[0], 
        vbe_info->signature[1],
        vbe_info->signature[2],
        vbe_info->signature[3] ); 

#endif

    return (ctx.ax == 0x4F && !memcmp(vbe_info->signature, "VESA", 4));
}

bool_t
vesa_get_mode_info(int mode)
{
    rmode_ctx_t ctx;

    if(vesa_mode_info == NULL)
    {
        vesa_mode_info = (vesa_mode_info_t *)malloc(sizeof(vesa_mode_info_t));
    }

    ctx.ax = 0x4F01;
    ctx.cx = mode;
    ctx.es = (((uint32_t)vesa_mode_info >> 4) & 0xF000); /* seg */
    ctx.di = (((uint32_t)vesa_mode_info >> 0) & 0xFFFF); /* offs */

    ldr_bios_call(BIOS_SVC_VIDEO, &ctx);

    return (ctx.ax == 0x4F);
}

bool_t
vesa_set_mem_window(int window, int position)
{
    /* Not needed if using VESA in LFB mode */

    rmode_ctx_t ctx;

    /*
        WinGranularity specifies the smallest boundary, in KB, 
        on which the window can be placed in the frame buffer memory
    */
    ctx.ax = 0x4F05;
    ctx.bh = 0;
    ctx.bl = window;
    ctx.dx = position;

    ldr_bios_call(BIOS_SVC_VIDEO, &ctx);

    return (ctx.ax == 0x4F);
}

uint16_t
vesa_find_mode(int w, int h, int bpp)
{
    uint16_t last_mode = 0;

    if(!vesa_get_info())
    {
        return false;
    }

    uint16_t* modes = (uint16_t *)((
        (vbe_info->video_modes & 0xFFFF0000) >> 12) + 
        (vbe_info->video_modes & 0xFFFF));

    for(int i = 0; modes[i] != 0xFFFF; i++)
    {
        if(!vesa_get_mode_info(modes[i]))
        {
            continue;
        }

        /* check for color mode and linear frame buffer (LFB) support */
        if( (vesa_mode_info->attributes & 0x19) != 0x19 && 
            (vesa_mode_info->attributes & 0x90) != 0x90 )
        {
            continue;
        }

        /* check for packed pixel or direct color mode */
        if( vesa_mode_info->memory_model != 4 &&
            vesa_mode_info->memory_model != 6 )
        {
            /*
                00h = Text mode
                01h = CGA graphics
                02h = Hercules graphics
                03h = 4-plane planar
                04h = Packed pixel
                05h = Non-chain 4, 256 color
                06h = Direct Color
                07h = YUV
            */
            continue;
        }

        /* check for supported bits per pixel */
        if( vesa_mode_info->bpp != 8 &&
            vesa_mode_info->bpp != 16 && 
            vesa_mode_info->bpp != 32 )
        {
            continue;
        }

        last_mode = modes[i];
        
#if 0

        printf(FG_LRED, "%d | %dx%d %dbpp | memory model %d | %d %d\n", 
            i, 
            vesa_mode_info->x_res, 
            vesa_mode_info->y_res, 
            vesa_mode_info->bpp,
            vesa_mode_info->memory_model,
            vesa_mode_info->red_mask_size,
            vesa_mode_info->red_mask_pos);

        printf(FG_LCYAN, "A:%x, B:%x, WinFunc:%x, PhysBase:%x\n", 
            vesa_mode_info->win_a_segment, 
            vesa_mode_info->win_b_segment,
            vesa_mode_info->win_func_ptr,
            vesa_mode_info->phys_base_ptr );

        getch();

#endif

        if( vesa_mode_info->x_res == w && 
            vesa_mode_info->y_res == h &&
            vesa_mode_info->bpp == bpp )
        {
            return modes[i];
        }

    }

    return last_mode;
}

bool_t
vesa_set_mode(int w, int h, int bpp)
{
    rmode_ctx_t ctx;

    uint16_t mode;

    if((mode = vesa_find_mode(w, h, bpp)) == 0)
        return false;

    /* store the info on the right mode */
    vesa_get_mode_info(mode);

    ctx.ax = 0x4F02;
    ctx.bx = mode | 0x4000; /* enable linear framebuffer mode */

    ldr_bios_call(BIOS_SVC_VIDEO, &ctx);

    return (ctx.ax == 0x4F);
}

void
vesa_get_resolution(uint32_t* w, uint32_t* h)
{
    *w = vesa_mode_info->x_res;
    *h = vesa_mode_info->y_res;
}

void 
vesa_set_color(uint8_t* ptr, uint8_t bpp, uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t masks[] = 
    {
        0x00, 0x01, 0x03, 0x07,
        0x0F, 0x1F, 0x3F, 0x7F,
        0xFF
    };

    if(bpp == 32)
    {
        *(uint32_t *)ptr = 
            ((uint32_t)(r & masks[vesa_mode_info->red_mask_size]) << 
                vesa_mode_info->red_mask_pos) |
            ((uint32_t)(g & masks[vesa_mode_info->green_mask_size]) << 
                vesa_mode_info->green_mask_pos) |
            ((uint32_t)(b & masks[vesa_mode_info->blue_mask_size]) << 
                vesa_mode_info->blue_mask_pos);
    }
    else
    if(bpp == 16)
    {
        *(uint16_t *)ptr = 
            ((uint16_t)(r & masks[vesa_mode_info->red_mask_size]) << 
                vesa_mode_info->red_mask_pos) |
            ((uint16_t)(g & masks[vesa_mode_info->green_mask_size]) << 
                vesa_mode_info->green_mask_pos) |
            ((uint16_t)(b & masks[vesa_mode_info->blue_mask_size]) << 
                vesa_mode_info->blue_mask_pos);
    }
    else
    if(bpp == 8)
    {
        *(uint8_t *)ptr = 
            ((uint8_t)(r & masks[vesa_mode_info->red_mask_size]) << 
                vesa_mode_info->red_mask_pos) |
            ((uint8_t)(g & masks[vesa_mode_info->green_mask_size]) << 
                vesa_mode_info->green_mask_pos) |
            ((uint8_t)(b & masks[vesa_mode_info->blue_mask_size]) << 
                vesa_mode_info->blue_mask_pos);
    }
    else
    {
        /* ??? */
    }
}

void 
vesa_get_color(uint8_t* ptr, uint8_t bpp, uint8_t* r, uint8_t* g, uint8_t* b)
{
    uint32_t tmp;

    uint8_t masks[] = 
    {
        0x00, 0x01, 0x03, 0x07,
        0x0F, 0x1F, 0x3F, 0x7F,
        0xFF
    };

    if(bpp == 32)
    {
        tmp = *(uint32_t *)ptr;
    }
    else
    if(bpp == 16)
    {
        tmp = *(uint16_t *)ptr;
    }
    else
    if(bpp == 8)
    {
        tmp = *(uint8_t *)ptr;
    }
    else
    {
        tmp = 0;
    }

    *r = (tmp >> vesa_mode_info->red_mask_pos) & 
        masks[vesa_mode_info->red_mask_size];

    *g = (tmp >> vesa_mode_info->green_mask_pos) & 
        masks[vesa_mode_info->green_mask_size];

    *b = (tmp >> vesa_mode_info->blue_mask_pos) & 
        masks[vesa_mode_info->blue_mask_size];
}

void 
vesa_set_pixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t* lfb = (uint8_t *)vesa_mode_info->phys_base_ptr;

    /* check that the linear frame buffer is valid */
    if(lfb == NULL || lfb == (uint8_t *)(~0))
        return;

    if(x >= vesa_mode_info->x_res || y >= vesa_mode_info->y_res)
        return;

    lfb += x * (vesa_mode_info->bpp >> 3);
    lfb += y * vesa_mode_info->bytes_per_scanline;

    vesa_set_color(lfb, vesa_mode_info->bpp, r, g, b);
}

uint8_t*
vesa_get_font_8x8(void)
{
    rmode_ctx_t ctx;

    ctx.ax = 0x1130;
    ctx.bx = 0x0300; /* get addr of ROM 8x8 (8 bits x 8 bytes) font */

    ldr_bios_call(BIOS_SVC_VIDEO, &ctx);

    return  (uint8_t *)(((uintptr_t)(ctx.es) << 16) | (uintptr_t)(ctx.bp));
}

uint8_t*
vesa_get_font_8x16(void)
{
    rmode_ctx_t ctx;

    ctx.ax = 0x1130;
    ctx.bx = 0x0600; /* get addr of ROM 8x16 (8 bits x 16 bytes) font */

    ldr_bios_call(BIOS_SVC_VIDEO, &ctx);

    return  (uint8_t *)(((uintptr_t)(ctx.es) << 16) | (uintptr_t)(ctx.bp));
}

/******************************************************************************/

void
bmp_init(bmp_t* bmp, uint32_t width, uint32_t height, uint8_t bpp, uint8_t* ptr)
{
    bmp->width = width;
    bmp->height = height;
    bmp->bpp = bpp;
    bmp->ptr = ptr;
}

void 
font_init(font_t* font, uint32_t width, uint32_t height, uint8_t scale, uint8_t* ptr)
{
    font->width = width;
    font->height = height;
    font->scale = scale;
    font->ptr = ptr;
}

void
bmp_get_pixel(bmp_t* bmp, uint32_t x, uint32_t y, uint32_t* rgb)
{
    uint8_t r;
    uint8_t g; 
    uint8_t b;

    uint8_t* tmp = bmp->ptr;
    uint8_t pixel_size = bmp->bpp >> 3;

    if(x >= bmp->width || y >= bmp->height)
        return;

    tmp += x * pixel_size;
    tmp += y * (bmp->width * pixel_size);

    vesa_get_color(tmp, bmp->bpp, &r, &g, &b);

    *rgb = (uint32_t)(r << 16 | g << 8 | b);    
}

void
bmp_set_pixel(bmp_t* bmp, uint32_t x, uint32_t y, uint32_t rgb)
{
    uint8_t* tmp = bmp->ptr;
    uint8_t pixel_size = bmp->bpp >> 3;

    if(x >= bmp->width || y >= bmp->height)
        return;

    tmp += x * pixel_size;
    tmp += y * (bmp->width * pixel_size);

    vesa_set_color(tmp, bmp->bpp, 
        (rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF); 
}

void 
bmp_puts(bmp_t* bmp, font_t* font, uint32_t x, uint32_t y, 
    uint32_t fg, uint32_t bg, char* str)
{
    int len = strlen(str);

    uint32_t color;
    uint32_t tmp_x = x;
    uint32_t tmp_y = y;

    int font_width_scaled = font->width * font->scale;
    int font_height_scaled = font->height * font->scale;

    for(int i = 0; i < len; i++)
    {
        if(str[i] == '\n')
        {
            y += font_height_scaled;

            tmp_x = x;
            tmp_y = y;
        }
        else
        if(str[i] == '\t')
        {
            tmp_x += (font_width_scaled * 4) - 
                (tmp_x % (font_width_scaled * 4));
        }
        else
        {
            uint8_t* glyph = font->ptr + ((uint32_t)str[i] * font->height);

            for(int dy = 0; dy < font_height_scaled; dy++)
            {
                for(int dx = 0; dx < font_width_scaled; dx++)
                {
                    if((glyph[dy / font->scale] & 
                        (0x80 >> (dx / font->scale))) != 0)
                    {
                        color = fg;
                    }
                    else
                    {
                        color = bg;
                    }

                    /* skip pixel on alpha channel set */
                    if((color & 0xFF000000) == 0)
                    {
                        bmp_set_pixel(bmp, tmp_x + dx, tmp_y + dy, color);
                    }
                }
            }

            tmp_x += font_width_scaled;
        }
    }
}

void
bmp_bitblt(bmp_t* src, bmp_t* dst, uint32_t dst_x, uint32_t dst_y)
{
    uint32_t rgb;

    /* bit block transfer */
    for(int y = 0; y < src->height; y++)
    {
        for(int x = 0; x < src->width; x++)
        {
            bmp_get_pixel(src, x, y, &rgb);
            bmp_set_pixel(dst, dst_x + x, dst_y + y, rgb);
        }
    }
}

void 
bmp_rect(bmp_t* bmp, uint32_t x, uint32_t y, uint32_t width, 
    uint32_t height, uint32_t rgb)
{
   for(int i = y; i < y + height; i++) 
   {
        for(int j = x; j < x + width; j++)
        {
            bmp_set_pixel(bmp, j, i, rgb);
        }
    }
}

void
bmp_from_ppm(bmp_t* bmp, uint8_t* ppm)
{
    char* end;
    int tmp = 0;

    if(ppm[0] != 'P' || ppm[1] != '6')
        return;

    /* skip the magic number */
    ppm += 3;

    /* consume all the comments */
    while(true)
    {
        if(*ppm == '#')
            tmp = true;
        
        if(tmp)
        {
            if(*ppm == '\n' || *ppm == '\r')
                tmp = false;

            ppm++;
        }
        else
        {
            break;
        }
    } 

    bmp->width = atoi((char *)ppm, &end); 
    bmp->height = atoi(end + 1, &end); 

    /* maximum value for each color (1 or 2 bytes) */
    tmp = atoi(end + 1, &end);
    ppm = (uint8_t *)end + 1;

    /* translate the rgb triplets to the bmp format */
    for(int y = 0; y < bmp->height; y++)
    {
        for(int x = 0; x < bmp->width; x++)
        {
            bmp_set_pixel(bmp, x, y, 
                (uint32_t)ppm[0] << 16 | 
                (uint32_t)ppm[1] << 8 | 
                (uint32_t)ppm[2]);

            ppm += 3;
        }
    }
}

void
bmp_to_framebuffer(bmp_t* bmp)
{
    uint32_t rgb;

    /* bit block transfer */
    for(int y = 0; y < bmp->height; y++)
    {
        for(int x = 0; x < bmp->width; x++)
        {
            bmp_get_pixel(bmp, x, y, &rgb);

            /* not really efficient... */
            vesa_set_pixel(x, y, (rgb >> 16) & 0xFF, 
                (rgb >> 8) & 0xFF, rgb & 0xFF);
        }
    }
}

