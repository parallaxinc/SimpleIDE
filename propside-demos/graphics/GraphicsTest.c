/**
 * @file GraphicsTest.c
 * Graphics test application ported from Chip Gracy's GraphicsDemo.spin
 * Copyright (c) 2009, Steve Denson
 * See end of file for MIT license terms.
 */
#include <stdint.h>
#include <stdlib.h>
#include <propeller.h>

#include "tv.h"
#include "Graphics.h"

void Start(void);
void grTest(void);

/**
 * graphics test main entry point
 */
int main(void)
{
    Start();
    grTest();    // never returns
    return 0;
}

#define X_TILES         12
#define Y_TILES         10
#define X_EXPANSION     14
#define X_ORIGIN        96
#define Y_ORIGIN        80
#define LINES           4
#define THICKNESS       3
#define MAX_COLORS      64

/*
 * TV and Display control variables
 */
HUBDATA TV_t  gTV;

HUBDATA volatile uint16_t gScreen[X_TILES * Y_TILES];
HUBDATA volatile uint32_t gColors[MAX_COLORS];

HUBDATA volatile uint32_t gDisplay[(X_TILES * Y_TILES << 4)+128];
HUBDATA volatile uint32_t gBitmap [(X_TILES * Y_TILES << 4)+128];

#define BITMAP_BASE (((int)gBitmap +256) & ~127)    // page align
#define DISPLY_BASE (((int)gDisplay+256) & ~127)

/*
 * demo lines and sprites
 */
uint8_t xa[LINES];
uint8_t ya[LINES];
uint8_t xsa[LINES];
uint8_t ysa[LINES];


/*
 * triangle
 */
uint16_t vecdef[] =
{
    0x4000+0x2000/3*0,      50,
    0x8000+0x2000/3*1+1,    50,
    0x8000+0x2000/3*2-1,    50,
    0x8000+0x2000/3*0,      50,
    0
};

/*
 * star
 */
uint16_t vecdef2[] =
{
    0x4000+0x2000/12*0,    50,
    0x8000+0x2000/12*1,    20,
    0x8000+0x2000/12*2,    50,
    0x8000+0x2000/12*3,    20,
    0x8000+0x2000/12*4,    50,
    0x8000+0x2000/12*5,    20,
    0x8000+0x2000/12*6,    50,
    0x8000+0x2000/12*7,    20,
    0x8000+0x2000/12*8,    50,
    0x8000+0x2000/12*9,    20,
    0x8000+0x2000/12*10,   50,
    0x8000+0x2000/12*11,   20,
    0x8000+0x2000/12*0,    50,
    0
};

/*
 * crosshair
 */
uint16_t pixdef[] =
{
    0x0702, 0x0000,     //byte    2,7,3,3  endianness ?
    0x0fc0, 0x0000,     //word    %%00333000,%%00000000
    0x3230, 0x0000,     //word    %%03020300,%%00000000
    0xc20c, 0x0000,     //word    %%30020030,%%00000000
    0xeaac, 0x0000,     //word    %%32222230,%%00000000
    0xc20c, 0x2000,     //word    %%30020030,%%02000000
    0x3230, 0xa800,     //word    %%03020300,%%22200000
    0x0fc0, 0x2000      //word    %%00333000,%%02000000
};

/*
 * dog
 */
uint16_t pixdef2[] =
{
    0x0401,
    0x0003,             //byte    1,4,0,3
    0x800a,             //word    %%20000022
    0x2aaa,             //word    %%02222222
    0x2aa0,             //word    %%02222200
    0x2020              //word    %%02000200
};

/*
 * start tv and graphics cogs
 */
static int tvstart(int pin)
{
    int n, dx, dy;
    int cog;
    TV_t* tp = &gTV;
    tp->enable = 1;
    tp->pins   = TV_BASEBAND_PIN(pin); 
    tp->mode   = 0;
    tp->screen = (uint32_t) &gScreen[0];     
    tp->colors = (uint32_t) &gColors[0];     
    tp->ht     = X_TILES;
    tp->vt     = Y_TILES;
    tp->hx     = X_EXPANSION;
    tp->vx     = 1;
    tp->ho     = 0;
    tp->vo     = 0;
    tp->broadcast = 0;
    tp->auralcog  = 0;

    cog = TV_start(tp);

    //setColorPalette(&gPalette[0]);

    // init colors
    for(n = 0; n < MAX_COLORS; n++)
        gColors[n] = 0x1010 * ((n+4) & 0xF) + 0x2B060C02;

    // init tile screen
    for (dx = 0; dx < tp->ht; dx++)
        for (dy = 0; dy < tp->vt; dy++)
            gScreen[dy * tp->ht + dx] = (DISPLY_BASE >> 6) + dy + dx * tp->vt + ((dy & 0x3F) << 10);

    return cog;
}


/*
 * Graphics test function
 */
char gstr[100];
#define MAXV 50

void grTest()
{
    int n = 0;
    int numx = 0;
    int numchr = '0';
    int k = 8776434;
    int t = 0;
    int limit = Y_ORIGIN;
    int mousex = 0, mousey = 0;

    srand(k);
    for(n = 0; n < LINES; n++) {
        xa[n]  = (rand() % 32) - 16;
        ya[n]  = (rand() % 32) - 16;
        xsa[n] = (rand() % 16) - 8;
        ysa[n] = (rand() % 16) - 8;
    }

    for(;;)
    {
#if 0
        int delay = 100;
        waitcnt(((CLKFREQ*delay)/1000)+CNT);
        DIRA |= (1 << 15);
        OUTA ^= (1 << 15);
#endif
        Graph_clear();

        Graph_colorwidth(2,0);
        Graph_textmode(1,1,5,0);
        //sprintf(gstr,"Base %X Display %X", BITMAP_BASE, DISPLY_BASE);
        Graph_text(-limit*2 + t++ % (limit*3), limit-15, gstr);

        Graph_colorwidth(2,k>>3);
        if(!(k % 4)) {
            mousex = rand()%(limit/4);
            mousey = rand()%(limit/4);
        }
        Graph_pix(mousex, mousey, k>>4 & 0x7, pixdef);

        //draw spinning triangles
        Graph_colorwidth(3,0);
        for(n = 1; n <= 8; n++)
            Graph_vec(0, 0, ((k & 0x7F) << 3) + (n << 4), (k << 6) + (n << 8), vecdef);

        // init bouncing lines ... not even close to graphics.spin elegance
        if (!(k % 4)) {
            for(n = 0; n < LINES; n++) {
                if(rand()%100 > 80)
                    xsa[n] = -xsa[n];
                if(rand()%100 > 80)
                    ysa[n] = -ysa[n];
                xa[n] += xsa[n] % 10;
                ya[n] += ysa[n] % 10;
            }
        }
        
        Graph_colorwidth(1,THICKNESS);
        Graph_plot((xa[0]%MAXV)-(MAXV>>1), (ya[0]%MAXV)-(MAXV>>1));
        for(n = 1; n < LINES; n++)
            Graph_line((xa[n]%MAXV)-(MAXV>>1),(ya[n]%MAXV)-(MAXV>>1));
        Graph_line((xa[0]%MAXV)-(MAXV>>1), (ya[0]%MAXV)-(MAXV>>1));

        //draw small box with text
        Graph_colorwidth(1,14);
        Graph_box(6,-limit+3,53,20);
        Graph_colorwidth(2,0);
        Graph_textmode(1,1,5,0);
        Graph_text(10,-limit+5,"Propeller");

        // draw axis
        Graph_colorwidth(2,0);
        Graph_plot(0, 0);
        Graph_line(0, limit);
        Graph_plot(0, 0);
        Graph_line(limit, 0);

        Graph_plot(-limit, 0);
        Graph_line(limit, 0);
        Graph_plot(0, -limit);
        Graph_line(0, limit);

        Graph_colorwidth(4,0);
        Graph_pix(-limit + k%(limit*2), 2 + k%2, 0, pixdef2);

        //draw spinning stars and revolving crosshairs and dogs
        Graph_colorwidth(2,0);
        for(n = 0; n < 8; n++) {
            Graph_vecarc(limit/2, limit/2, 20, 20, -((n << 10) + (k << 6)), 0x40, -(k<<7), vecdef2);
            Graph_pixarc(-limit/2, -limit/2, 20, 20,  (n << 10) + (k << 6), k<<4, pixdef2);
            Graph_pixarc(-limit/2, -limit/2, 10, 10, -((n << 10) + (k << 6)), k<<4, pixdef);
        }

        // draw expanding pixel halo
        Graph_colorwidth(1,k);
        Graph_arc(0,0,limit,20,-k<<5,0x2000/9,9,0);

        // draw incrementing digit
        if((++numx % 7) == 0)
            numchr++;
        if(numchr < '0' || numchr > '9')
            numchr = '0';
        //Graph_textmode(8,8,6,5);
        Graph_textmode(3,3,5,0);
        Graph_colorwidth(1,20);
        Graph_char(-limit+5 + t % (limit*2), t*t % (limit/3) - limit/2, numchr);

        // copy bitmap to display
        if(BITMAP_BASE != DISPLY_BASE)
            Graph_copy(DISPLY_BASE);
        
        k++;
    }
}

void Start()
{
    tvstart(12);
    Graph_start();
    Graph_setup(X_TILES, Y_TILES, X_ORIGIN, Y_ORIGIN, BITMAP_BASE);
}

/*
+------------------------------------------------------------------------------------------------------------------------------+
¦                                                   TERMS OF USE: MIT License                                                  ¦                                                            
+------------------------------------------------------------------------------------------------------------------------------¦
¦Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation    ¦ 
¦files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,    ¦
¦modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software¦
¦is furnished to do so, subject to the following conditions:                                                                   ¦
¦                                                                                                                              ¦
¦The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.¦
¦                                                                                                                              ¦
¦THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE          ¦
¦WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR         ¦
¦COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,   ¦
¦ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                         ¦
+------------------------------------------------------------------------------------------------------------------------------+
*/

