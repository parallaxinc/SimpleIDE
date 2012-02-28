/**
 * @file Graphics.c
 * Graphics driver ported from Chip Gracy's Graphics.spin
 * Users should comment out unused functions to save code space if necessary.
 * Copyright (c) 2009, Steve Denson
 * See end of file for MIT license terms.
 */
#include <stdio.h>
#include <string.h>
#include <propeller.h>
#include "Graphics.h"

/**
''
'' Theory of Operation:
''
'' A cog is launched with code contained in Graphics_Array[] which
'' processes commands sent by the Graph_* API wrapper functions.
''
'' Points, lines, arcs, sprites, text, and polygons are rasterized into
'' a specified stretch of memory which serves as a generic bitmap buffer.
''
'' The bitmap can be displayed by the TV or VGA driver.
'' To date, the C library version has been tested on TV only.
''
'' See GraphicsTest.c for usage example.
''
 */ 

/**
 * Graphics_Font is the character set
 * in separate GraphicsFont.c file
 */ 
extern uint16_t Graphics_Font[];

/**
 * Graphics interface control structure
 */
HUBDATA GraphCmd_ST gGraph;

/*
 * set command parameter array
 */
HUBDATA uint32_t pa[GRAPH_MAX_ARGS];

/**
 * Color codes
 */
HUBDATA uint32_t Graph_colors[] =
{
  0x00000000,
  0x55555555,
  0xAAAAAAAA,
  0xFFFFFFFF
};

/**
 * Round pixel recipes
 */
HUBDATA uint8_t Graph_pixels[] =
{
    0x00, 0x00, 0x00, 0x00,         //0,1,2,3
    0x00, 0x00, 0x02, 0x05,         //4,5,6,7
    0x0A, 0x0A, 0x1A, 0x1A,         //8,9,A,B
    0x34, 0x3a, 0x74, 0x74          //C,D,E,F
};


/**
 * start graphics driver
 * see header for API description.
 */
int	Graph_start(void)
{
    // spin uses fontptr := font. C has no such luxury
    // to achieve the same result, we pass font in setup
    int cog;
	extern uint32_t binary_Graphics_dat_start[];
#if defined(__PROPELLER_XMM__)
    extern uint32_t pasm[]; // defined in TV.c
	extern uint32_t binary_Graphics_dat_end[];
	int binsize = 	(binary_Graphics_dat_end - binary_Graphics_dat_start);
	memcpy(pasm, binary_Graphics_dat_start, binsize*4);
#endif
	gGraph.command = -1;

// Debug Only.
#if 0 // defined(__PROPELLER_XMM__) || defined(__PROPELLER_XMMC__)
	printf("Graph_start\n");
	printf("GraphCmd_ST %x\n", &gGraph);
	printf("%x\n", &gGraph.cog);
	printf("%x\n", &gGraph.command);
    printf("%x\n", &gGraph.pixel_width);  //pixel data width
    printf("%x\n", &gGraph.bitmap_ptr);   //bitmap data
    printf("%x\n", &gGraph.bitmap_len);
    printf("%x\n", &gGraph.bases);
    printf("%x\n", &gGraph.slices);
    printf("%x\n", &gGraph.text_xs); //text data (these 4 must be contiguous)
    printf("%x\n", &gGraph.text_ys);
    printf("%x\n", &gGraph.text_sp);
    printf("%x\n", &gGraph.text_just); 
	if(0) {
		int n;
		for(n = 0; n < 496; n++) {
			if(!(n % 8))
				printf("\n%03x ",n<<2);
			printf("0x%08x ",pasm[n]);
		}
		printf("\n");
	}
	//for(;;);
#endif
#if defined(__PROPELLER_XMM__)
    cog = cognew(pasm, (uint32_t)&gGraph.command)+1;
#else
    cog = cognew(binary_Graphics_dat_start, (uint32_t)&gGraph.command)+1;
#endif
	while(gGraph.command != 0)
		;
    //printf("Graph_start COG %d started 0x%08x\n", cog, gGraph.command);
    gGraph.cog = cog;

    return cog;
}

/**
 * stop graphics driver
 * see header for API description.
 */
void Graph_stop(void)
{
    GraphCmd_ST* gp = &gGraph;
    uint32_t cog = gp->cog-1;
    if(gp->cog > 0) {
        cogstop(cog);
    	gp->cog = 0;
	}
}

/**
 * set bitmap parameters
 * see header for API description.
 */
void Graph_setup(uint32_t x_tiles, uint32_t y_tiles, uint32_t x_origin, uint32_t y_origin, uint32_t base_ptr)
{
    GraphCmd_ST* gp = &gGraph;
    int n = 0;

	/* the simplest possible command */
    sendCommand(GraphCmd_loop, 0);

    for(n = 0; n < x_tiles && n < 32; n++)
        gp->bases[n] = base_ptr + ((n * y_tiles) << 6);

    y_tiles <<= 4;
    y_origin = y_tiles - y_origin - 1;

    pa[0] = x_tiles;
    pa[1] = y_tiles;
    pa[2] = x_origin;
    pa[3] = y_origin;
    pa[4] = (uint32_t)gp->bases; // driver ignores arg4
    pa[5] = (uint32_t)gp->bases;
    pa[6] = (uint32_t)gp->slices;
    pa[7] = (uint32_t)Graphics_Font;
	//printf("Graph_setup %d\n", GraphCmd_setup);
    sendCommand(GraphCmd_setup, pa);

    gp->bitmap_ptr = (uint32_t*)base_ptr;
    gp->bitmap_len = x_tiles * y_tiles;
    //printf("\nBP Longs 0x%x Bases 0x%x\n", gp->bitmap_ptr, gp->bases);
}

/**
 * clear bitmap
 * see header for API description.
 */
void Graph_clear(void)
{
    sendCommand(GraphCmd_loop, 0);
    memset(gGraph.bitmap_ptr, 0, gGraph.bitmap_len<<2);
}

/**
 * copy bitmap - use for double-buffered display
 * see header for API description.
 */
void Graph_copy(uint32_t dest_ptr)
{
    sendCommand(GraphCmd_loop, 0);
    memcpy((void*)dest_ptr, (void*)gGraph.bitmap_ptr, gGraph.bitmap_len<<2);
}

/**
 * set pixel color to two bit pattern
 * see header for API description.
 */
void Graph_color(uint32_t c)
{
    sendCommand(GraphCmd_color, (uint32_t*)&Graph_colors[c & 3]);
}

/**
 * set pixel width - actual width is w[3..0]+1
 * see header for API description.
 */
void Graph_width(uint32_t w)
{
    int  passes, r, n, pw;

    r = ~w & 0x10;
    w &= 0xf;
    gGraph.pixel_width = w;
    passes = (w >> 1) + 1;

    memset(pa, 0, GRAPH_MAX_ARGS<<2);
    pa[0] = w;
    pa[1] = passes;

    sendCommand(GraphCmd_width, pa);

    pw = w ^ 0xf;
    for(n = 0; n <= w >> 1; n++) {
        gGraph.slices[n] = (uint32_t)(0xffffffff >> (pw << 1)) << (pw & 0xE);
        if(r && (Graph_pixels[w] & (1 << n)))
            pw += 2;
        if(r && n == passes - 2)
            pw += 2;
    }
}

/**
 * set pixel color and width
 * see header for API description.
 */
void Graph_colorwidth(uint32_t c, uint32_t w)
{
    Graph_color(c);
    Graph_width(w);
}

/**
 * plot a pixel at point x,y
 * see header for API description.
 */
void Graph_plot(uint32_t x, uint32_t y)
{
    pa[0] = x;
    pa[1] = y;
    sendCommand(GraphCmd_plot, pa);
}

/**
 * draw a line to point x,y (from previous x,y?)
 * see header for API description.
 */
void Graph_line(uint32_t x, uint32_t y)
{
    pa[0] = x;
    pa[1] = y;
    sendCommand(GraphCmd_line, pa);
}

/**
 * draw an arc
 * see header for API description.
 */
void Graph_arc(uint32_t x, uint32_t y, uint32_t xr, uint32_t yr, int angle, int anglestep, int steps, int arcmode)
{
    pa[0] = x;
    pa[1] = y;
    pa[2] = xr;
    pa[3] = yr;
    pa[4] = angle;
    pa[5] = anglestep;
    pa[6] = steps;
    pa[7] = arcmode;
    sendCommand(GraphCmd_arc, pa);
}

/**
 * draw a vector
 * see header for API description.
 */
void Graph_vec(uint32_t x, uint32_t y, int vecscale, int vecangle, uint16_t* vecdef_ptr)
{
    pa[0] = x;
    pa[1] = y;
    pa[2] = vecscale;
    pa[3] = vecangle;
    pa[4] = (uint32_t)vecdef_ptr;
    sendCommand(GraphCmd_vec, pa);
}

/**
 * draw a vector arc
 * see header for API description.
 */
void Graph_vecarc(uint32_t x, uint32_t y, uint32_t xr, uint32_t yr, int angle, int vecscale, int vecangle, uint16_t* vecdef_ptr)
{
    pa[0] = x;
    pa[1] = y;
    pa[2] = xr;
    pa[3] = yr;
    pa[4] = angle;
    pa[5] = vecscale;
    pa[6] = vecangle;
    pa[7] = (uint32_t)vecdef_ptr;
    sendCommand(GraphCmd_vecarc, pa);
}

/**
 * draw a pixel sprite
 * see header for API description.
 */
void Graph_pix(uint32_t x, uint32_t y, int pixrot, uint16_t* pixdef_ptr)
{
    pa[0] = x;
    pa[1] = y;
    pa[2] = pixrot;
    pa[3] = (long)pixdef_ptr;
    sendCommand(GraphCmd_pix, pa);
}

/**
 * draw a pixel sprite at an arc position
 * see header for API description.
 */
void Graph_pixarc(uint32_t x, uint32_t y, uint32_t xr, uint32_t yr, int angle, int pixrot, uint16_t* pixdef_ptr)
{
    pa[0] = x;
    pa[1] = y;
    pa[2] = xr;
    pa[3] = yr;
    pa[4] = angle;
    pa[5] = pixrot;
    pa[6] = (uint32_t)pixdef_ptr;
    sendCommand(GraphCmd_pixarc, pa);
}

/**
 * draw char at x,y
 * see header for API description.
 */
void Graph_char(uint32_t x, uint32_t y, char ch)
{
    char s[2] = { '\0', '\0' };
    s[0] = ch;
    Graph_text(x, y, s);
}

/**
 * draw text at x,y
 * see header for API description.
 */
void Graph_text(uint32_t x, uint32_t y, char* string_ptr)
{
    uint32_t just[2];

    justify(string_ptr, just);
    pa[0] = x;
    pa[1] = y;
    pa[2] = (uint32_t)string_ptr;
    pa[3] = just[0];
    pa[4] = just[1];
    sendCommand(GraphCmd_text, pa);
}

/**
 * draw text at an arc position
 * see header for API description.
 */
void Graph_textarc(uint32_t x, uint32_t y, uint32_t xr, uint32_t yr, int angle, char* string_ptr)
{
    uint32_t just[2];

    justify(string_ptr, just);
    pa[0] = x;
    pa[1] = y;
    pa[2] = xr;
    pa[3] = yr;
    pa[4] = angle;
    pa[5] = (uint32_t)string_ptr;
    pa[6] = just[0];
    pa[7] = just[1];
    sendCommand(GraphCmd_textarc, pa);
}

/**
 * set text size and justification
 * see header for API description.
 */
void Graph_textmode(uint32_t x_scale, uint32_t y_scale, uint32_t spacing, uint32_t justification)
{
    GraphCmd_ST* gp = &gGraph;
    pa[0] = gp->text_xs = x_scale;
    pa[1] = gp->text_ys = y_scale;
    pa[2] = gp->text_sp = spacing;
    pa[3] = gp->text_just = justification;
    sendCommand(GraphCmd_textmode, pa);
}

/**
 * draw a box with round/square corners according to pixel width
 * see header for API description.
 */
void Graph_box(int x, int y, int box_width, int box_height)
{
    uint32_t x2, y2, pmin, pmax;
    int width = gGraph.pixel_width;

    if((box_width > width) && (box_height > width))
    {
        pmin = width >> 1;
        pmax = width - pmin;
        x += pmin;
        y += pmin;
        x2 = x + box_width  - 1 - width;
        y2 = y + box_height - 1 - width;

        Graph_plot(x,y);
        Graph_plot(x,y2);
        Graph_plot(x2,y);
        Graph_plot(x2,y2);

        fill(x, y2 + pmax, 0, (x2 - x) << 16, 0, 0, pmax);  // fill gaps
        fill(x, y, 0, (x2 - x) << 16, 0, 0, pmin);
        fill(x - pmin, y2, 0, (x2 - x + width) << 16, 0, 0, y2 - y);
    }
}

/**
 * draw a solid quadrilateral
 * see header for API description.
 */
void Graph_quad(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3, uint32_t x4, uint32_t y4)
{
    Graph_tri(x1, y1, x2, y2, x3, y3);
    Graph_tri(x3, y3, x4, y4, x1, y1);
}

void Graph_tri(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3)
{
    int xy[2];
    int mycase = ((y1 >= y2) ? 4 : 0) | ((y2 >= y3) ? 2 : 0) | ((y1 >= y3) ? 1 : 0);

    switch(mycase) {
        case 0:
            xy[0] = x1; 
            xy[1] = y1; // 2 longs 
            x1 = x3;
            y1 = y3;    // 2 longs
            x3 = xy[0];
            y3 = xy[1]; // 2 longs
            break;
        case 2:
            xy[0] = x1; 
            xy[1] = y1; // 2 longs
            x1 = x2;
            y1 = y2;
            x2 = x3;
            y2 = y3;    // 4 longs
            x3 = xy[0];
            y3 = xy[1]; // 2 longs
            break;
        case 3:
            xy[0] = x1; 
            xy[1] = y1; // 2 longs
            x1 = x2;
            y1 = y2;    // 2 longs
            x2 = xy[0];
            y2 = xy[1]; // 2 longs
            break;
        case 4:
            xy[0] = x3; 
            xy[1] = y3; // 2 longs
            x2 = x1;
            y2 = y1;
            x3 = x2;
            y3 = y2;    // 4 longs
            x1 = xy[0];
            y1 = xy[1]; // 2 longs
            break;
        case 5:
            xy[0] = x2; 
            xy[1] = y2; // 2 longs
            x2 = x3;
            y2 = y3;    // 2 longs
            x3 = xy[0];
            y3 = xy[1]; // 2 longs
            break;
        default:
            break;
    }

    fill(x1, y1, ((x3 - x1) << 16) / (y1 - y3 + 1), ((x2 - x1) << 16) / (y1 - y2 + 1), ((x3 - x2) << 16) / (y2 - y3 + 1), y1 - y2, y1 - y3);
}

/**
 * make sure last command is done
 */
void Graph_finish(void)
{
    sendCommand(GraphCmd_loop, 0);
}


/**
 * fill blocks
 */
void fill(int x, int y, int da, int db, int db2, int linechange, int lines_minus_1)
{
    pa[0] = x;
    pa[1] = y;
    pa[2] = da;
    pa[3] = db;
    pa[4] = db2;
    pa[5] = linechange;
    pa[6] = lines_minus_1;
    sendCommand(GraphCmd_fill, pa);
}

/**
 * justify apparently aligns string edges
 */
void justify(char* string_ptr, uint32_t* justptr)
{
    GraphCmd_ST* gp = &gGraph;
    uint32_t x = (strlen(string_ptr) - 1) * gp->text_xs * gp->text_sp + gp->text_xs * 5 - 1;
    uint32_t ja1[4];
    uint32_t ja2[4];
    
    // p1 is an alias of ja1
    ja1[0] = 0;
    ja1[1] = x >> 1;
    ja1[2] = x;
    ja1[3] = 0;
    justptr[0] = -ja1[gp->text_just]; // -lookupz(text_just >> 2 & 3: 0, x >> 1, x, 0)

    // p2 is an alias of ja1
    ja2[0] = 0;
    ja2[1] = gp->text_ys << 3;
    ja2[2] = gp->text_ys << 4;
    ja2[3] = 0;
    justptr[1] = -ja2[gp->text_just]; // -lookupz(text_just & 3: 0, text_ys << 3, text_ys << 4, 0)
}

#if DEBUG_CMD
char* cmds[] = {
    "None",  "Setup",
    "Color", "Width",
    "Plot",  "Line", 
    "Arc",   "Vec", "VecArc",
    "Pix",   "PixArc",
    "Text",  "TextArc", "TextMode",
    "Fill",  "Loop",
    0 };
#endif

/**
 * send command to driver
 * @param cmd    - GraphCmd_ET command
 * @param argptr - arguments pointer
 */
void sendCommand(GraphCmd_ET cmd, uint32_t* argptr)
{
    //write command and pointer
    gGraph.command = ((cmd << 16) | ((int)argptr & 0xffff));
#if DEBUG_CMD
    printf("\nsendCommand(0x%x, 0x%x): 0x%x ... %s", cmd, (int)argptr, gGraph.command, cmds[cmd]);
    fflush(stdout);
#endif
    // wait for command to be cleared, signifying receipt
    while(gGraph.command != 0)
		;
}



/*
 * Ported from Graphics.spin
 *
''***************************************
''*  Graphics Driver v1.0               *
''*  Author: Chip Gracey                *
''*  Copyright (c) 2005 Parallax, Inc.  *               
''*  See end of file for terms of use.  *               
''***************************************
 * 
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
