/**
 * @file tv.c - tv cog start/stop control
 * Copyright (c) 2008, Steve Denson
 * See end of file for terms of use.
 */
#include <stdint.h>
#include <string.h>
#include <propeller.h>
#include "tv.h"

static int gcog;

#if defined(__PROPELLER_XMM__)
HUBDATA uint32_t pasm[496];
#endif

int  TV_start(TV_t* tvp)
{
	extern uint32_t binary_TV_dat_start[];
#if defined(__PROPELLER_XMM__)
	extern uint32_t binary_TV_dat_end[];
	int size = 	(binary_TV_dat_end - binary_TV_dat_start);
	memcpy(pasm, binary_TV_dat_start, size*4);
	gcog = cognew(pasm, tvp) + 1;
#else
	gcog = cognew(binary_TV_dat_start, tvp) + 1;
#endif
    return gcog;
}

#ifdef  TVTEST

#define X_TILES 10 // 44
#define Y_TILES 10 // 14

#define TV_TEXT_COLORTABLE_SIZE 64 // 8*2

HUBDATA TV_t gTV;

HUBDATA volatile uint16_t gScreen[X_TILES * Y_TILES];
HUBDATA volatile uint32_t gColors[TV_TEXT_COLORTABLE_SIZE];
HUBDATA volatile uint8_t  gpalette[TV_TEXT_COLORTABLE_SIZE] =     
{                   // fgRGB  bgRGB
    0x07, 0x0a,     // 0    white / dark blue
    0x07, 0xbb,     // 1   yellow / brown
    0x9e, 0x9b,     // 2  magenta / black
    0x04, 0x07,     // 3     grey / white
    0x3d, 0x3b,     // 4     cyan / dark cyan
    0x6b, 0x6e,     // 5    green / gray-green
    0xbb, 0xce,     // 6      red / pink
    0x3e, 0x0a      // 7     cyan / blue
};

static void setColorPalette(volatile uint8_t* ptr)
{
    int  ii = 0;
    int  mm = 0;
    int  fg = 0;
    int  bg = 0;
    for(ii = 0; ii < TV_TEXT_COLORTABLE_SIZE; ii += 2)
    {
        mm = ii + 1; // beta1 ICC has trouble with math in braces. use mm
        fg = ptr[ii] << 0;
        bg = ptr[mm] << 0;
        gColors[ii]  = (fg << 24) | (bg << 16) | (fg << 8) | bg;
        gColors[mm]  = (fg << 24) | (fg << 16) | (bg << 8) | bg;
   }        
}

int main(void)
{
    int dx, dy;

    gTV.enable = 1;
    gTV.pins   = TV_BASEBAND_PIN(12); 
    gTV.mode   = 0; //0x12;
    gTV.screen = (uint32_t) &gScreen[0];     
    gTV.colors = (uint32_t) &gColors[0];     
    gTV.ht     = X_TILES;
    gTV.vt     = Y_TILES;
    gTV.hx     = 10; // 4 if ht=44, vt=14
    gTV.vx     = 1;
    gTV.ho     = 0;
    gTV.vo     = 0;
    gTV.broadcast = 0;
    gTV.auralcog  = 0;

    TV_start(&gTV);
    setColorPalette(&gpalette[0]);

    // tile screen
    for (dx = 0; dx < gTV.ht; dx++) 
        for (dy = 0; dy < gTV.vt; dy++) 
            gScreen[dy * gTV.ht + dx] = (0x6000 >> 6) + dy + dx * gTV.vt + ((dy & 0x3F) << 10);

	return 0;
}
#endif

/*
+--------------------------------------------------------------------
|  TERMS OF USE: MIT License
+--------------------------------------------------------------------
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
+------------------------------------------------------------------
*/
