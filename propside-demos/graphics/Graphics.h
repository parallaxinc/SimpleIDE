/**
 * @file Graphics.h
 * Graphics driver header & API descriptions.
 * Copyright (c) 2009, Steve Denson
 * See end of file for MIT license terms.
 */

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <stdint.h>

/**
 * Graphics driver interface command enumerations
 */
typedef enum Graph_enum
{
    GraphCmd_setup = 1,
    GraphCmd_color,
    GraphCmd_width,
    GraphCmd_plot,
    GraphCmd_line,
    GraphCmd_arc,
    GraphCmd_vec,
    GraphCmd_vecarc,
    GraphCmd_pix,
    GraphCmd_pixarc,
    GraphCmd_text,
    GraphCmd_textarc,
    GraphCmd_textmode,
    GraphCmd_fill,
    GraphCmd_loop
} GraphCmd_ET;


/**
 * Graphics driver interface structure
 */
typedef volatile struct GraphCmd_struct
{
    uint32_t cog;
    uint32_t command;
    uint32_t pixel_width;   //pixel data width
    uint32_t* bitmap_ptr;   //bitmap data
    uint32_t  bitmap_len;
    uint16_t bases[32];		// not sure why this is here
    uint32_t slices[8];
    uint32_t text_xs; 	    //text data (these 4 must be contiguous)
    uint32_t text_ys;
    uint32_t text_sp;
    uint32_t text_just; 
} GraphCmd_ST;

/**
 * maximum number of args to pass a command
 */
#define GRAPH_MAX_ARGS 9

/**
 * Vector Font Primitives
 */
#define Graph_xa0   0x0 << 0             //x line start / arc center
#define Graph_xa1   0x1 << 0
#define Graph_xa2   0x2 << 0
#define Graph_xa3   0x3 << 0
#define Graph_xa4   0x4 << 0
#define Graph_xa5   0x5 << 0
#define Graph_xa6   0x6 << 0
#define Graph_xa7   0x7 << 0

#define Graph_ya0   0x0 << 3            //y line start / arc center
#define Graph_ya1   0x1 << 3
#define Graph_ya2   0x2 << 3
#define Graph_ya3   0x3 << 3
#define Graph_ya4   0x4 << 3
#define Graph_ya5   0x5 << 3
#define Graph_ya6   0x6 << 3
#define Graph_ya7   0x7 << 3
#define Graph_ya8   0x8 << 3
#define Graph_ya9   0x9 << 3
#define Graph_yaA   0xA << 3
#define Graph_yaB   0xB << 3
#define Graph_yaC   0xC << 3
#define Graph_yaD   0xD << 3
#define Graph_yaE   0xE << 3
#define Graph_yaF   0xF << 3

#define Graph_xb0   0x0 << 7             //x line end
#define Graph_xb1   0x1 << 7
#define Graph_xb2   0x2 << 7
#define Graph_xb3   0x3 << 7
#define Graph_xb4   0x4 << 7
#define Graph_xb5   0x5 << 7
#define Graph_xb6   0x6 << 7
#define Graph_xb7   0x7 << 7

#define Graph_yb0   0x0 << 10           //y line end
#define Graph_yb1   0x1 << 10
#define Graph_yb2   0x2 << 10
#define Graph_yb3   0x3 << 10
#define Graph_yb4   0x4 << 10
#define Graph_yb5   0x5 << 10
#define Graph_yb6   0x6 << 10
#define Graph_yb7   0x7 << 10
#define Graph_yb8   0x8 << 10
#define Graph_yb9   0x9 << 10
#define Graph_ybA   0xA << 10
#define Graph_ybB   0xB << 10
#define Graph_ybC   0xC << 10
#define Graph_ybD   0xD << 10
#define Graph_ybE   0xE << 10
#define Graph_ybF   0xF << 10

#define Graph_ax1   0 << 7               //x arc radius
#define Graph_ax2   1 << 7

#define Graph_ay1   0 << 8              //y arc radius
#define Graph_ay2   1 << 8
#define Graph_ay3   2 << 8
#define Graph_ay4   3 << 8

#define Graph_a0    0x0 << 10           //arc start/length
#define Graph_a1    0x1 << 10           //bits[1..0] = start (0..3 = 0°, 90°, 180°, 270°)
#define Graph_a2    0x2 << 10           //bits[3..2] = length (0..3 = 360°, 270°, 180°, 90°)
#define Graph_a3    0x3 << 10
#define Graph_a4    0x4 << 10
#define Graph_a5    0x5 << 10
#define Graph_a6    0x6 << 10
#define Graph_a7    0x7 << 10
#define Graph_a8    0x8 << 10
#define Graph_a9    0x9 << 10
#define Graph_aA    0xA << 10
#define Graph_aB    0xB << 10
#define Graph_aC    0xC << 10
#define Graph_aD    0xD << 10
#define Graph_aE    0xE << 10
#define Graph_aF    0xF << 10

#define Graph_fline 0 << 14              //line command
#define Graph_farc  1 << 14              //arc command

#define Graph_more  1 << 15              //another arc/line

#define Graph_fx 3                       //number of custom characters

/**
 * start graphics driver - starts a cog
 * @return cog number or 0 if no cog available
 */
int  Graph_start(void);

/**
 * stop graphics driver - frees cog
 */
void Graph_stop(void);

/**
 * set bitmap parameters
 *
 * @param x_tiles        - number of x tiles (tiles are 16x16 pixels each)
 * @param y_tiles        - number of y tiles
 * @param x_origin       - relative-x center pixel
 * @param y_origin       - relative-y center pixel
 * @param base_ptr       - base address of bitmap
 */
void Graph_setup(uint32_t x_tiles, uint32_t y_tiles, uint32_t x_origin, uint32_t y_origin, uint32_t base_ptr);

/**
 * clear bitmap
 */
void Graph_clear(void);

/**
 * Copy bitmap
 * use for double-buffered display (flicker-free)
 *
 * @dest_ptr       - base address of destination bitmap
 */
void Graph_copy(uint32_t dest_ptr);

/**
 * Set pixel color to two-bit pattern
 *
 * @param c              - color code in bits[1..0]
 */
void Graph_color(uint32_t c);

/**
 * Set pixel width
 * actual width is w[3..0] + 1
 *
 * @param w              - 0..15 for round pixels, 16..31 for square pixels
 */
void Graph_width(uint32_t w);

/**
 * Set pixel color and width
 *
 * @param c - pixel color
 * @param c - pixel width
 */
void Graph_colorwidth(uint32_t c, uint32_t w);

/**
 * Plot point
 * x,y            - point
 *
 * @param x - x position (left to right)
 * @param y - y position (top to bottom)
 */
void Graph_plot(uint32_t x, uint32_t y);

/**
 * Draw a line to point
 *
 * x,y            - endpoint
 *
 * @param x - end point of line (left to right)
 * @param x - end point of line (top to bottom)
 */
void Graph_line(uint32_t x, uint32_t y);

/**
 * Draw an arc
 * x,y            - center of arc
 * xr,yr          - radii of arc
 *
 * @param x - center x position
 * @param y - center y position
 * @param xr -  x radius length
 * @param yr -  y radius length
 * @param angle          - initial angle in bits[12..0] (0..$1FFF = 0°..359.956°)
 * @param anglestep      - angle step in bits[12..0]
 * @param steps          - number of steps (0 just leaves (x,y) at initial arc position)
 * @param arcmode        - 0: plot point(s)
 *                         1: line to point(s)
 *                         2: line between points
 *                         3: line from point(s) to center
 */
void Graph_arc(uint32_t x, uint32_t y, uint32_t xr, uint32_t yr, int angle, int anglestep, int steps, int arcmode);

/**
 * Draw a vector sprite
 *
 * Vector sprite definition:
 *
 * word    $8000|$4000+angle       'vector mode + 13-bit angle (mode: $4000=plot, $8000=line)
 * word    length                  'vector length
 * ...                             'more vectors
 * ...
 * word    0                       'end of definition
 *
 * x,y          - center of vector sprite
 *
 * @param x     - center x position of vector sprite
 * @param y     - center y position of vector sprite
 * @param vecscale       - scale of vector sprite ($100 = 1x)
 * @param vecangle       - rotation angle of vector sprite in bits[12..0]
 * @param vecdef_ptr     - address of vector sprite definition
 */
void Graph_vec(uint32_t x, uint32_t y, int vecscale, int vecangle, uint16_t* vecdef_ptr);

/*
 * Draw a vector sprite at an arc position
 *
 * @param x              - x center of arc
 * @param y              - y center of arc
 * @param xr             - x radius of arc
 * @param yr             - y radius of arc
 * @param angle          - angle in bits[12..0] (0..$1FFF = 0°..359.956°)
 * @param vecscale       - scale of vector sprite ($100 = 1x)
 * @param vecangle       - rotation angle of vector sprite in bits[12..0]
 * @param vecdef_ptr     - address of vector sprite definition
 */
void Graph_vecarc(uint32_t x, uint32_t y, uint32_t xr, uint32_t yr, int angle, int vecscale, int vecangle, uint16_t* vecdef_ptr);

/**
 * Draw a pixel sprite
 *
 * Pixel sprite definition:
 *
 *  word    'word align, express dimensions and center, define pixels
 *  byte    xwords, ywords, xorigin, yorigin
 *  word    %%xxxxxxxx,%%xxxxxxxx
 *  word    %%xxxxxxxx,%%xxxxxxxx
 *  word    %%xxxxxxxx,%%xxxxxxxx
 *  ...
 *
 * @param x            - x center of vector sprite
 * @param y            - y center of vector sprite
 * @param pixrot         - 0: 0°, 1: 90°, 2: 180°, 3: 270°, +4: mirror
 * @param pixdef_ptr     - address of pixel sprite definition
 */
void Graph_pix(uint32_t x, uint32_t y, int pixrot, uint16_t* pixdef_ptr);

/**
 * Draw a pixel sprite at an arc position
 *
 * @param x              - x center of arc
 * @param y              - y center of arc
 * @param xr             - x radius of arc
 * @param yr             - y radius of arc
 * @param angle          - angle in bits[12..0] (0..$1FFF = 0°..359.956°)
 * @param pixrot         - 0: 0°, 1: 90°, 2: 180°, 3: 270°, +4: mirror
 * @param pixdef_ptr     - address of pixel sprite definition
 */
void Graph_pixarc(uint32_t x, uint32_t y, uint32_t xr, uint32_t yr, int angle, int pixrot, uint16_t* pixdef_ptr);

/**
 * Draw char
 *
 * @param x            - text position x (see textmode for sizing and justification)
 * @param y            - text position y (see textmode for sizing and justification)
 * @param ch           - char to draw
 */
void Graph_char(uint32_t x, uint32_t y, char ch);

/**
 * Draw text
 *
 * @param x            - text position x (see textmode for sizing and justification)
 * @param y            - text position y (see textmode for sizing and justification)
 * @param string_ptr   - address of zero-terminated string (it may be necessary to call .finish
 *                       immediately afterwards to prevent subsequent code from clobbering the
 *                       string as it is being drawn
 */
void Graph_text(uint32_t x, uint32_t y, char* string_ptr);

/**
 * Draw text at an arc position
 *
 * @param x              - x center of arc
 * @param y              - y center of arc
 * @param xr             - x radius of arc
 * @param yr             - y radius of arc
 * @param angle          - angle in bits[12..0] (0..$1FFF = 0°..359.956°)
 * @param string_ptr     - address of zero-terminated string (it may be necessary to call .finish
 *                         immediately afterwards to prevent subsequent code from clobbering the
 *                         string as it is being drawn
 */
void Graph_textarc(uint32_t x, uint32_t y, uint32_t xr, uint32_t yr, int angle, char* string_ptr);

/**
 * Set text size and justification
 *
 * @param x_scale        - x character scale, should be 1+
 * @param y_scale        - y character scale, should be 1+
 * @param spacing        - character spacing, 6 is normal
 * @param justification  - bits[1..0]: 0..3 = left, center, right, left
 *                         bits[3..2]: 0..3 = bottom, center, top, bottom
 */
void Graph_textmode(uint32_t x_scale, uint32_t y_scale, uint32_t spacing, uint32_t justification);

/**
 * Draw a box with round/square corners, according to pixel width
 *
 * @param x            - box left
 * @param y            - box box bottom
 * @param box_width    - box left
 * @param box_height   - box box bottom
 */
void Graph_box(int x, int y, int box_width, int box_height);

/**
 * Draw a solid quadrilateral
 * vertices must be ordered clockwise or counter-clockwise
 *
 * @param x1           - vertex x1
 * @param y1           - vertex y1
 * @param x2           - vertex x2
 * @param y2           - vertex y2
 * @param x3           - vertex x3
 * @param y3           - vertex y3
 * @param x4           - vertex x4
 * @param y4           - vertex y4
 */
void Graph_quad(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3, uint32_t x4, uint32_t y4);

/**
 * Draw a solid triangle
 * vertices must be ordered clockwise or counter-clockwise
 *
 * @param x1           - vertex x1
 * @param y1           - vertex y1
 * @param x2           - vertex x2
 * @param y2           - vertex y2
 * @param x3           - vertex x3
 * @param y3           - vertex y3
 */
void Graph_tri(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3);

/**
 * Wait for any current graphics command to finish
 * use this to insure that it is safe to manually manipulate the bitmap
 */
void Graph_finish(void);

/**
 * private functions
 */
void fill(int x, int y, int da, int db, int db2, int linechange, int lines_minus_1);
void justify(char* string_ptr, uint32_t* justptr);
void sendCommand(GraphCmd_ET cmd, uint32_t* argptr);

#endif
// __GRAPHICS_H__

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
