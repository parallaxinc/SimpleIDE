/**
 * @file tv.h - tv cog start/stop control
 * Copyright (c) 2009, Steve Denson
 * See end of file for terms of use.
 */
#ifndef __TV_H__
#define __TV_H__

#include <stdint.h>

/**
 * TV struct for starting/controlling a TV cog
 */
typedef volatile struct TV_struct
{
  uint32_t status     ; // 0/1/2 = off/visible/invisible           read-only
  uint32_t enable     ; // 0/? = off/on                            write-only
  uint32_t pins       ; // %ppmmm = pins                           write-only
  uint32_t mode       ; // %ccinp = chroma,interlace,ntsc/pal,swap write-only
  uint32_t screen     ; // pointer to screen (words)               write-only
  uint32_t colors     ; // pointer to colors (longs)               write-only               
  uint32_t ht         ; // horizontal tiles                        write-only
  uint32_t vt         ; // vertical tiles                          write-only
  uint32_t hx         ; // horizontal cell expansion               write-only
  uint32_t vx         ; // vertical cell expansion                 write-only
  uint32_t ho         ; // horizontal offset                       write-only
  uint32_t vo         ; // vertical offset                         write-only
  uint32_t broadcast  ; // broadcast frequency (Hz)                write-only
  uint32_t auralcog   ; // aural fm cog                            write-only
} TV_t;

/**
 * TV_BASEBAND_PIN macro gets tv.pins definition based on pin
 */
#define TV_BASEBAND_PIN(pin) ((pin & 0x38) << 1) | (((pin & 4) == 4) ? 0x5 : 0) 

/**
 * start a TV cog
 * @param tvptr - pointer to tv startup array
 * @return zero on failure or non-zero cog number
 */
int  TV_start(TV_t* tvptr);

/**
 * stop previously registered tv cog
 */
void TV_stop(void);


/*
''
''___
''VAR                   'TV parameters - 14 contiguous longs
''
''  long  tv_status     '0/1/2 = off/invisible/visible              read-only
''  long  tv_enable     '0/non-0 = off/on                           write-only
''  long  tv_pins       '%pppmmmm = pin group, pin group mode       write-only
''  long  tv_mode       '%tccip = tile,chroma,interlace,ntsc/pal    write-only
''  long  tv_screen     'pointer to screen (words)                  write-only      
''  long  tv_colors     'pointer to colors (longs)                  write-only                            
''  long  tv_ht         'horizontal tiles                           write-only                            
''  long  tv_vt         'vertical tiles                             write-only                            
''  long  tv_hx         'horizontal tile expansion                  write-only                            
''  long  tv_vx         'vertical tile expansion                    write-only                            
''  long  tv_ho         'horizontal offset                          write-only                            
''  long  tv_vo         'vertical offset                            write-only                            
''  long  tv_broadcast  'broadcast frequency (Hz)                   write-only                            
''  long  tv_auralcog   'aural fm cog                               write-only                            
''                                                                                              
''The preceding VAR section may be copied into your code.        
''After setting variables, do start(@tv_status) to start driver. 
''                                                               
''All parameters are reloaded each superframe, allowing you to make live
''changes. To minimize flicker, correlate changes with tv_status.
''
''Experimentation may be required to optimize some parameters.
''
''Parameter descriptions:
''  _________
''  tv_status
''
''    driver sets this to indicate status:
''      0: driver disabled (tv_enable = 0 or CLKFREQ < requirement)
''      1: currently outputting invisible sync data
''      2: currently outputting visible screen data
''  _________
''  tv_enable
''
''        0: disable (pins will be driven low, reduces power)
''    non-0: enable
''  _______
''  tv_pins
''
''    bits 6..4 select pin group:
''      %000: pins 7..0
''      %001: pins 15..8
''      %010: pins 23..16
''      %011: pins 31..24
''      %100: pins 39..32
''      %101: pins 47..40
''      %110: pins 55..48
''      %111: pins 63..56
''
''    bits 3..0 select pin group mode:
''      %0000: %0000_0111    -                    baseband
''      %0001: %0000_0111    -                    broadcast
''      %0010: %0000_1111    -                    baseband + chroma
''      %0011: %0000_1111    -                    broadcast + aural
''      %0100: %0111_0000    broadcast            -
''      %0101: %0111_0000    baseband             -
''      %0110: %1111_0000    broadcast + aural    -
''      %0111: %1111_0000    baseband + chroma    -
''      %1000: %0111_0111    broadcast            baseband
''      %1001: %0111_0111    baseband             broadcast
''      %1010: %0111_1111    broadcast            baseband + chroma
''      %1011: %0111_1111    baseband             broadcast + aural
''      %1100: %1111_0111    broadcast + aural    baseband
''      %1101: %1111_0111    baseband + chroma    broadcast
''      %1110: %1111_1111    broadcast + aural    baseband + chroma
''      %1111: %1111_1111    baseband + chroma    broadcast + aural
''      -----------------------------------------------------------
''            active pins    top nibble           bottom nibble
''
''      the baseband signal nibble is arranged as:
''        bit 3: chroma signal for s-video (attach via 560-ohm resistor)
''        bits 2..0: baseband video (sum 270/560/1100-ohm resistors to form 75-ohm 1V signal)
''
''      the broadcast signal nibble is arranged as:
''        bit 3: aural subcarrier (sum 560-ohm resistor into network below)
''        bits 2..0: visual carrier (sum 270/560/1100-ohm resistors to form 75-ohm 1V signal)
''  _______
''  tv_mode
''
''    bit 4 selects between 16x16 and 16x32 pixel tiles:
''      0: 16x16 pixel tiles (tileheight = 16)
''      1: 16x32 pixel tiles (tileheight = 32)
''
''    bit 3 controls chroma mixing into broadcast:
''      0: mix chroma into broadcast (color)
''      1: strip chroma from broadcast (black/white)
''
''    bit 2 controls chroma mixing into baseband:
''      0: mix chroma into baseband (composite color)
''      1: strip chroma from baseband (black/white or s-video)
''
''    bit 1 controls interlace:
''      0: progressive scan (243 display lines for NTSC, 286 for PAL)
''           less flicker, good for motion
''      1: interlaced scan (486 display lines for NTSC, 572 for PAL)
''           doubles the vertical display lines, good for text
''
''    bit 0 selects NTSC or PAL format
''      0: NTSC
''           3016 horizontal display ticks
''           243 or 486 (interlaced) vertical display lines
''           CLKFREQ must be at least 14_318_180 (4 * 3_579_545 Hz)*
''      1: PAL
''           3692 horizontal display ticks
''           286 or 572 (interlaced) vertical display lines
''           CLKFREQ must be at least 17_734_472 (4 * 4_433_618 Hz)*
''
''      * driver will disable itself while CLKFREQ is below requirement
''  _________
''  tv_screen
''
''    pointer to words which define screen contents (left-to-right, top-to-bottom)
''      number of words must be tv_ht * tv_vt
''      each word has two bitfields: a 6-bit colorset ptr and a 10-bit pixelgroup ptr
''        bits 15..10: select the colorset* for the associated pixel tile
''        bits 9..0: select the pixelgroup** address %ppppppppppcccc00 (p=address, c=0..15)
''
''       * colorsets are longs which each define four 8-bit colors
''
''      ** pixelgroups are <tileheight> longs which define (left-to-right, top-to-bottom) the 2-bit
''         (four color) pixels that make up a 16x16 or a 32x32 pixel tile
''  _________
''  tv_colors
''
''    pointer to longs which define colorsets
''      number of longs must be 1..64
''      each long has four 8-bit fields which define colors for 2-bit (four color) pixels
''      first long's bottom color is also used as the screen background color
''      8-bit color fields are as follows:
''        bits 7..4: chroma data (0..15 = blue..green..red..)*
''        bit 3: controls chroma modulation (0=off, 1=on)
''        bits 2..0: 3-bit luminance level:
''          values 0..1: reserved for sync - don't use
''          values 2..7: valid luminance range, modulation adds/subtracts 1 (beware of 7)
''          value 0 may be modulated to produce a saturated color toggling between levels 1 and 7
''
''      * because of TV's limitations, it doesn't look good when chroma changes abruptly -
''        rather, use luminance - change chroma only against a black or white background for
''        best appearance
''  _____
''  tv_ht
''
''    horizontal number pixel tiles - must be at least 1
''    practical limit is 40 for NTSC, 50 for PAL
''  _____
''  tv_vt
''
''    vertical number of pixel tiles - must be at least 1
''    practical limit is 13 for NTSC, 15 for PAL (26/30 max for interlaced NTSC/PAL)
''  _____
''  tv_hx
''
''    horizontal tile expansion factor - must be at least 3 for NTSC, 4 for PAL
''
''    make sure 16 * tv_ht * tv_hx + ||tv_ho + 32 is less than the horizontal display ticks
''  _____
''  tv_vx
''
''    vertical tile expansion factor - must be at least 1
''
''    make sure <tileheight> * tv_vt * tv_vx + ||tv_vo + 1 is less than the display lines
''  _____
''  tv_ho
''
''    horizontal offset in ticks - pos/neg value (0 for centered image)
''    shifts the display right/left
''  _____
''  tv_vo
''
''    vertical offset in lines - pos/neg value (0 for centered image)
''    shifts the display up/down
''  ____________
''  tv_broadcast
''
''    broadcast frequency expressed in Hz (ie channel 2 is 55_250_000)
''    if 0, modulator is turned off - saves power
''
''    broadcasting requires CLKFREQ to be at least 16_000_000
''    while CLKFREQ is below 16_000_000, modulator will be turned off
''  ___________
''  tv_auralcog
''
''    selects cog to supply aural fm signal - 0..7
''    uses ctra pll output from selected cog
''
''    in NTSC, the offset frequency must be 4.5MHz and the max bandwidth +-25KHz
''    in PAL, the offset frequency and max bandwidth vary by PAL type
*/

#endif
// __TV_H__

/*
 * Port of Chip Gracy's TV.spin
 *
'' ***************************************
'' *  TV Driver v1.1                     *
'' *  Author: Chip Gracey                *
'' *  Copyright (c) 2004 Parallax, Inc.  *               
'' *  See end of file for terms of use.  *               
'' ***************************************
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
