/**
 * @file Graphics_Font.c
 * Graphics vector font ported from Chip Gracey's Graphics.spin
 * Copyright (c) 2009, Steve Denson
 * See end of file for MIT license terms.
 */
#include <propeller.h>
#include "Graphics.h"

/**
 * Vector font - standard Graph_ascii characters ($21-$7E)
 */
HUBDATA uint16_t Graphics_Font[] = 
{
    Graph_fline | Graph_xa2 | Graph_yaC | Graph_xb2 | Graph_yb7 | Graph_more,            //!
    Graph_fline | Graph_xa2 | Graph_ya5 | Graph_xb2 | Graph_yb4,

    Graph_fline | Graph_xa1 | Graph_yaD | Graph_xb1 | Graph_ybC | Graph_more,            //"
    Graph_fline | Graph_xa3 | Graph_yaD | Graph_xb3 | Graph_ybC,

    Graph_fline | Graph_xa1 | Graph_yaA | Graph_xb1 | Graph_yb6 | Graph_more,            //#
    Graph_fline | Graph_xa3 | Graph_yaA | Graph_xb3 | Graph_yb6 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya9 | Graph_xb4 | Graph_yb9 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya7 | Graph_xb4 | Graph_yb7,

    Graph_farc | Graph_xa2 | Graph_ya9 | Graph_a9 | Graph_ax2 | Graph_ay1 | Graph_more,        //$
    Graph_farc | Graph_xa2 | Graph_ya7 | Graph_aB | Graph_ax2 | Graph_ay1 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya6 | Graph_xb2 | Graph_yb6 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_yaA | Graph_xb4 | Graph_ybA | Graph_more,
    Graph_fline | Graph_xa2 | Graph_yaA | Graph_xb2 | Graph_ybB | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya6 | Graph_xb2 | Graph_yb5,

    Graph_farc | Graph_xa1 | Graph_yaA | Graph_a0 | Graph_ax1 | Graph_ay1 | Graph_more,        //%
    Graph_farc | Graph_xa3 | Graph_ya6 | Graph_a0 | Graph_ax1 | Graph_ay1 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya6 | Graph_xb4 | Graph_ybA,

    Graph_farc | Graph_xa2 | Graph_yaA | Graph_a7 | Graph_ax1 | Graph_ay1 | Graph_more,        //&
    Graph_farc | Graph_xa2 | Graph_ya7 | Graph_a5 | Graph_ax2 | Graph_ay2 | Graph_more,
    Graph_fline | Graph_xa1 | Graph_yaA | Graph_xb4 | Graph_yb5,

    Graph_fline | Graph_xa2 | Graph_yaD | Graph_xb2 | Graph_ybC,                  //'

    Graph_farc | Graph_xa3 | Graph_ya9 | Graph_aD | Graph_ax1 | Graph_ay4 | Graph_more,        //(
    Graph_farc | Graph_xa3 | Graph_ya7 | Graph_aE | Graph_ax1 | Graph_ay4 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya7 | Graph_xb2 | Graph_yb9,

    Graph_farc | Graph_xa1 | Graph_ya9 | Graph_aC | Graph_ax1 | Graph_ay4 | Graph_more,        //)
    Graph_farc | Graph_xa1 | Graph_ya7 | Graph_aF | Graph_ax1 | Graph_ay4 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya7 | Graph_xb2 | Graph_yb9,

    Graph_fline | Graph_xa4 | Graph_ya6 | Graph_xb0 | Graph_ybA | Graph_more,            // star
    Graph_fline | Graph_xa0 | Graph_ya6 | Graph_xb4 | Graph_ybA | Graph_more,
    Graph_fline | Graph_xa2 | Graph_yaB | Graph_xb2 | Graph_yb5,

    Graph_fline | Graph_xa0 | Graph_ya8 | Graph_xb4 | Graph_yb8 | Graph_more,            //+
    Graph_fline | Graph_xa2 | Graph_yaA | Graph_xb2 | Graph_yb6,

    Graph_fline | Graph_xa2 | Graph_ya4 | Graph_xb1 | Graph_yb3,                  //,

    Graph_fline | Graph_xa0 | Graph_ya8 | Graph_xb4 | Graph_yb8,                  //-

    Graph_fline | Graph_xa2 | Graph_ya5 | Graph_xb2 | Graph_yb4,                  //.

    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb4 | Graph_ybC,                  //\/

    Graph_farc | Graph_xa2 | Graph_ya8 | Graph_a0 | Graph_ax2 | Graph_ay4,              //0

    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb4 | Graph_yb4 | Graph_more,            //1
    Graph_fline | Graph_xa2 | Graph_ya4 | Graph_xb2 | Graph_ybC | Graph_more,
    Graph_fline | Graph_xa0 | Graph_yaA | Graph_xb2 | Graph_ybC,

    Graph_farc | Graph_xa2 | Graph_yaA | Graph_a8 | Graph_ax2 | Graph_ay2 | Graph_more,        //2
    Graph_farc | Graph_xa2 | Graph_yaA | Graph_aF | Graph_ax2 | Graph_ay3 | Graph_more,
    Graph_farc | Graph_xa2 | Graph_ya4 | Graph_aD | Graph_ax2 | Graph_ay3 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb4 | Graph_yb4,

    Graph_farc | Graph_xa2 | Graph_yaA | Graph_a7 | Graph_ax2 | Graph_ay2 | Graph_more,        //3
    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_a6 | Graph_ax2 | Graph_ay2,

    Graph_fline | Graph_xa2 | Graph_yaC | Graph_xb0 | Graph_yb7 | Graph_more,            //4
    Graph_fline | Graph_xa0 | Graph_ya7 | Graph_xb4 | Graph_yb7 | Graph_more,
    Graph_fline | Graph_xa3 | Graph_ya4 | Graph_xb3 | Graph_yb8,

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_aB | Graph_ax2 | Graph_ay2 | Graph_more,        //5
    Graph_fline | Graph_xa4 | Graph_yaC | Graph_xb0 | Graph_ybC | Graph_more,
    Graph_fline | Graph_xa0 | Graph_yaC | Graph_xb0 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya8 | Graph_xb2 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb2 | Graph_yb4,

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_a0 | Graph_ax2 | Graph_ay2 | Graph_more,        //6
    Graph_farc | Graph_xa2 | Graph_ya8 | Graph_aD | Graph_ax2 | Graph_ay4 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya6 | Graph_xb0 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_yaC | Graph_xb3 | Graph_ybC,

    Graph_fline | Graph_xa0 | Graph_yaC | Graph_xb4 | Graph_ybC | Graph_more,            //7
    Graph_fline | Graph_xa1 | Graph_ya4 | Graph_xb4 | Graph_ybC,

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_a0 | Graph_ax2 | Graph_ay2 | Graph_more,        //8
    Graph_farc | Graph_xa2 | Graph_yaA | Graph_a0 | Graph_ax2 | Graph_ay2,

    Graph_farc | Graph_xa2 | Graph_yaA | Graph_a0 | Graph_ax2 | Graph_ay2 | Graph_more,        //9
    Graph_farc | Graph_xa2 | Graph_ya8 | Graph_aF | Graph_ax2 | Graph_ay4 | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya8 | Graph_xb4 | Graph_ybA | Graph_more,
    Graph_fline | Graph_xa1 | Graph_ya4 | Graph_xb2 | Graph_yb4,

    Graph_fline | Graph_xa2 | Graph_ya6 | Graph_xb2 | Graph_yb7 | Graph_more,            //:
    Graph_fline | Graph_xa2 | Graph_yaA | Graph_xb2 | Graph_yb9,

    Graph_fline | Graph_xa2 | Graph_ya4 | Graph_xb1 | Graph_yb3 | Graph_more,            //;
    Graph_fline | Graph_xa2 | Graph_ya8 | Graph_xb2 | Graph_yb7,

    Graph_fline | Graph_xa0 | Graph_ya8 | Graph_xb4 | Graph_ybA | Graph_more,            //<
    Graph_fline | Graph_xa0 | Graph_ya8 | Graph_xb4 | Graph_yb6,

    Graph_fline | Graph_xa0 | Graph_yaA | Graph_xb4 | Graph_ybA | Graph_more,            //=
    Graph_fline | Graph_xa0 | Graph_ya6 | Graph_xb4 | Graph_yb6,

    Graph_fline | Graph_xa4 | Graph_ya8 | Graph_xb0 | Graph_ybA | Graph_more,            //>
    Graph_fline | Graph_xa4 | Graph_ya8 | Graph_xb0 | Graph_yb6,

    Graph_farc | Graph_xa2 | Graph_yaB | Graph_a8 | Graph_ax2 | Graph_ay1 | Graph_more,        //?
    Graph_farc | Graph_xa3 | Graph_yaB | Graph_aF | Graph_ax1 | Graph_ay2 | Graph_more,
    Graph_farc | Graph_xa3 | Graph_ya7 | Graph_aD | Graph_ax1 | Graph_ay2 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya5 | Graph_xb2 | Graph_yb4,

    Graph_farc | Graph_xa2 | Graph_ya8 | Graph_a0 | Graph_ax1 | Graph_ay1 | Graph_more,        //@
    Graph_farc | Graph_xa2 | Graph_ya8 | Graph_a4 | Graph_ax2 | Graph_ay3 | Graph_more,
    Graph_farc | Graph_xa3 | Graph_ya8 | Graph_aF | Graph_ax1 | Graph_ay1 | Graph_more,
    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_aF | Graph_ax2 | Graph_ay1 | Graph_more,
    Graph_fline | Graph_xa3 | Graph_ya7 | Graph_xb3 | Graph_yb9,

    Graph_farc | Graph_xa2 | Graph_yaA | Graph_a8 | Graph_ax2 | Graph_ay2 | Graph_more,        //A
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_ybA | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya4 | Graph_xb4 | Graph_ybA | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya8 | Graph_xb4 | Graph_yb8,

    Graph_farc | Graph_xa2 | Graph_yaA | Graph_aB | Graph_ax2 | Graph_ay2 | Graph_more,        //B
    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_aB | Graph_ax2 | Graph_ay2 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_ybC | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb2 | Graph_yb4 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya8 | Graph_xb2 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_yaC | Graph_xb2 | Graph_ybC,

    Graph_farc | Graph_xa2 | Graph_yaA | Graph_a8 | Graph_ax2 | Graph_ay2 | Graph_more,        //C
    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_aA | Graph_ax2 | Graph_ay2 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya6 | Graph_xb0 | Graph_ybA,

    Graph_farc | Graph_xa2 | Graph_yaA | Graph_aC | Graph_ax2 | Graph_ay2 | Graph_more,        //D
    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_aF | Graph_ax2 | Graph_ay2 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_ybC | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya6 | Graph_xb4 | Graph_ybA | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb2 | Graph_yb4 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_yaC | Graph_xb2 | Graph_ybC,

    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_ybC | Graph_more,            //E
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb4 | Graph_yb4 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya8 | Graph_xb3 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_yaC | Graph_xb4 | Graph_ybC,

    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_ybC | Graph_more,            //F
    Graph_fline | Graph_xa0 | Graph_ya8 | Graph_xb3 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_yaC | Graph_xb4 | Graph_ybC,

    Graph_farc | Graph_xa2 | Graph_yaA | Graph_a8 | Graph_ax2 | Graph_ay2 | Graph_more,        //G
    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_aA | Graph_ax2 | Graph_ay2 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya6 | Graph_xb0 | Graph_ybA | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya4 | Graph_xb4 | Graph_yb7 | Graph_more,
    Graph_fline | Graph_xa3 | Graph_ya7 | Graph_xb4 | Graph_yb7,

    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_ybC | Graph_more,            //H
    Graph_fline | Graph_xa4 | Graph_ya4 | Graph_xb4 | Graph_ybC | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya8 | Graph_xb4 | Graph_yb8,

    Graph_fline | Graph_xa2 | Graph_ya4 | Graph_xb2 | Graph_ybC | Graph_more,            //I
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb4 | Graph_yb4 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_yaC | Graph_xb4 | Graph_ybC,

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_aA | Graph_ax2 | Graph_ay2 | Graph_more,        //J
    Graph_fline | Graph_xa4 | Graph_ya6 | Graph_xb4 | Graph_ybC,

    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_ybC | Graph_more,            //K
    Graph_fline | Graph_xa4 | Graph_yaC | Graph_xb0 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya4 | Graph_xb0 | Graph_yb8,

    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_ybC | Graph_more,            //L
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb4 | Graph_yb4,

    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_ybC | Graph_more,            //M
    Graph_fline | Graph_xa4 | Graph_ya4 | Graph_xb4 | Graph_ybC | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya8 | Graph_xb0 | Graph_ybC | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya8 | Graph_xb4 | Graph_ybC,

    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_ybC | Graph_more,            //N
    Graph_fline | Graph_xa4 | Graph_ya4 | Graph_xb4 | Graph_ybC | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya4 | Graph_xb0 | Graph_ybC,

    Graph_farc | Graph_xa2 | Graph_yaA | Graph_a8 | Graph_ax2 | Graph_ay2 | Graph_more,        //0
    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_aA | Graph_ax2 | Graph_ay2 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya6 | Graph_xb0 | Graph_ybA | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya6 | Graph_xb4 | Graph_ybA,

    Graph_farc | Graph_xa2 | Graph_yaA | Graph_aB | Graph_ax2 | Graph_ay2 | Graph_more,        //P
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_ybC | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya8 | Graph_xb2 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_yaC | Graph_xb2 | Graph_ybC,

    Graph_farc | Graph_xa2 | Graph_yaA | Graph_a8 | Graph_ax2 | Graph_ay2 | Graph_more,        //Q
    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_aA | Graph_ax2 | Graph_ay2 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya6 | Graph_xb0 | Graph_ybA | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya6 | Graph_xb4 | Graph_ybA | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya6 | Graph_xb4 | Graph_yb3,

    Graph_farc | Graph_xa2 | Graph_yaA | Graph_aB | Graph_ax2 | Graph_ay2 | Graph_more,        //R
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_ybC | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya8 | Graph_xb2 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_yaC | Graph_xb2 | Graph_ybC | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya4 | Graph_xb2 | Graph_yb8,

    Graph_farc | Graph_xa2 | Graph_yaA | Graph_a4 | Graph_ax2 | Graph_ay2 | Graph_more,        //S
    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_a6 | Graph_ax2 | Graph_ay2,

    Graph_fline | Graph_xa2 | Graph_ya4 | Graph_xb2 | Graph_ybC | Graph_more,            //T
    Graph_fline | Graph_xa0 | Graph_yaC | Graph_xb4 | Graph_ybC,

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_aA | Graph_ax2 | Graph_ay2 | Graph_more,        //U
    Graph_fline | Graph_xa0 | Graph_ya6 | Graph_xb0 | Graph_ybC | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya6 | Graph_xb4 | Graph_ybC,

    Graph_fline | Graph_xa2 | Graph_ya4 | Graph_xb0 | Graph_ybC | Graph_more,            //V
    Graph_fline | Graph_xa2 | Graph_ya4 | Graph_xb4 | Graph_ybC,

    Graph_fline | Graph_xa0 | Graph_yaC | Graph_xb0 | Graph_yb4 | Graph_more,            //W
    Graph_fline | Graph_xa4 | Graph_yaC | Graph_xb4 | Graph_yb4 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya8 | Graph_xb0 | Graph_yb4 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya8 | Graph_xb4 | Graph_yb4,

    Graph_fline | Graph_xa4 | Graph_ya4 | Graph_xb0 | Graph_ybC | Graph_more,            //X
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb4 | Graph_ybC,

    Graph_fline | Graph_xa0 | Graph_yaC | Graph_xb2 | Graph_yb8 | Graph_more,            //Y
    Graph_fline | Graph_xa4 | Graph_yaC | Graph_xb2 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya4 | Graph_xb2 | Graph_yb8,

    Graph_fline | Graph_xa0 | Graph_yaC | Graph_xb4 | Graph_ybC | Graph_more,            //Z
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb4 | Graph_ybC | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb4 | Graph_yb4,

    Graph_fline | Graph_xa2 | Graph_yaD | Graph_xb2 | Graph_yb3 | Graph_more,            //[
    Graph_fline | Graph_xa2 | Graph_yaD | Graph_xb4 | Graph_ybD | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya3 | Graph_xb4 | Graph_yb3,

    Graph_fline | Graph_xa4 | Graph_ya4 | Graph_xb0 | Graph_ybC,                  // backslash

    Graph_fline | Graph_xa2 | Graph_yaD | Graph_xb2 | Graph_yb3 | Graph_more,            //[
    Graph_fline | Graph_xa2 | Graph_yaD | Graph_xb0 | Graph_ybD | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya3 | Graph_xb0 | Graph_yb3,

    Graph_fline | Graph_xa2 | Graph_yaA | Graph_xb0 | Graph_yb6 | Graph_more,            //^
    Graph_fline | Graph_xa2 | Graph_yaA | Graph_xb4 | Graph_yb6,

    Graph_fline | Graph_xa0 | Graph_ya1 | Graph_xa4 | Graph_yb1,                  //_

    Graph_fline | Graph_xa1 | Graph_ya9 | Graph_xb3 | Graph_yb7,                  //`

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_a0 | Graph_ax2 | Graph_ay2 | Graph_more,        //a
    Graph_fline | Graph_xa4 | Graph_ya4 | Graph_xb4 | Graph_yb8,

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_a0 | Graph_ax2 | Graph_ay2 | Graph_more,        //b
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_ybC,

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_a9 | Graph_ax2 | Graph_ay2 | Graph_more,        //c
    Graph_fline | Graph_xa2 | Graph_ya4 | Graph_xb4 | Graph_yb4 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya8 | Graph_xb4 | Graph_yb8,

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_a0 | Graph_ax2 | Graph_ay2 | Graph_more,        //d
    Graph_fline | Graph_xa4 | Graph_ya4 | Graph_xb4 | Graph_ybC,

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_a4 | Graph_ax2 | Graph_ay2 | Graph_more,        //e
    Graph_fline | Graph_xa0 | Graph_ya6 | Graph_xb4 | Graph_yb6 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya4 | Graph_xb4 | Graph_yb4,

    Graph_farc | Graph_xa4 | Graph_yaA | Graph_aD | Graph_ax2 | Graph_ay2 | Graph_more,        //f
    Graph_fline | Graph_xa0 | Graph_ya8 | Graph_xb4 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya4 | Graph_xb2 | Graph_ybA,

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_a0 | Graph_ax2 | Graph_ay2 | Graph_more,        //g
    Graph_farc | Graph_xa2 | Graph_ya3 | Graph_aF | Graph_ax2 | Graph_ay2 | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya3 | Graph_xb4 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa1 | Graph_ya1 | Graph_xb2 | Graph_yb1,

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_a8 | Graph_ax2 | Graph_ay2 | Graph_more,        //h
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_ybC | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya4 | Graph_xb4 | Graph_yb6,

    Graph_fline | Graph_xa1 | Graph_ya4 | Graph_xb3 | Graph_yb4 | Graph_more,            //i
    Graph_fline | Graph_xa2 | Graph_ya4 | Graph_xb2 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa1 | Graph_ya8 | Graph_xb2 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_yaB | Graph_xb2 | Graph_ybA,

    Graph_farc | Graph_xa0 | Graph_ya3 | Graph_aF | Graph_ax2 | Graph_ay2 | Graph_more,        //j
    Graph_fline | Graph_xa2 | Graph_ya3 | Graph_xb2 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa1 | Graph_ya8 | Graph_xb2 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_yaB | Graph_xb2 | Graph_ybA,

    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_ybC | Graph_more,            //k
    Graph_fline | Graph_xa0 | Graph_ya6 | Graph_xb2 | Graph_yb6 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya6 | Graph_xb4 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya6 | Graph_xb4 | Graph_yb4,

    Graph_fline | Graph_xa1 | Graph_ya4 | Graph_xb3 | Graph_yb4 | Graph_more,            //l
    Graph_fline | Graph_xa2 | Graph_ya4 | Graph_xb2 | Graph_ybC | Graph_more,
    Graph_fline | Graph_xa1 | Graph_yaC | Graph_xb2 | Graph_ybC,

    Graph_farc | Graph_xa1 | Graph_ya7 | Graph_a8 | Graph_ax1 | Graph_ay1 | Graph_more,        //m
    Graph_farc | Graph_xa3 | Graph_ya7 | Graph_a8 | Graph_ax1 | Graph_ay1 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya4 | Graph_xb2 | Graph_yb7 | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya4 | Graph_xb4 | Graph_yb7,

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_a8 | Graph_ax2 | Graph_ay2 | Graph_more,        //n
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya4 | Graph_xb4 | Graph_yb6,

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_a0 | Graph_ax2 | Graph_ay2,              //o

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_a0 | Graph_ax2 | Graph_ay2 | Graph_more,        //p
    Graph_fline | Graph_xa0 | Graph_ya1 | Graph_xb0 | Graph_yb8,

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_a0 | Graph_ax2 | Graph_ay2 | Graph_more,        //q
    Graph_fline | Graph_xa4 | Graph_ya1 | Graph_xb4 | Graph_yb8,

    Graph_farc | Graph_xa2 | Graph_ya7 | Graph_a8 | Graph_ax2 | Graph_ay1 | Graph_more,        //r
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb0 | Graph_yb8,

    Graph_farc | Graph_xa2 | Graph_ya7 | Graph_a9 | Graph_ax2 | Graph_ay1 | Graph_more,        //s
    Graph_farc | Graph_xa2 | Graph_ya5 | Graph_aB | Graph_ax2 | Graph_ay1 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb2 | Graph_yb4 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya8 | Graph_xb4 | Graph_yb8,

    Graph_farc | Graph_xa4 | Graph_ya6 | Graph_aE | Graph_ax2 | Graph_ay2 | Graph_more,        //t
    Graph_fline | Graph_xa0 | Graph_ya8 | Graph_xb4 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya6 | Graph_xb2 | Graph_ybA,

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_aA | Graph_ax2 | Graph_ay2 | Graph_more,        //u
    Graph_fline | Graph_xa0 | Graph_ya6 | Graph_xb0 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya4 | Graph_xb4 | Graph_yb8,

    Graph_fline | Graph_xa0 | Graph_ya8 | Graph_xb2 | Graph_yb4 | Graph_more,            //v
    Graph_fline | Graph_xa4 | Graph_ya8 | Graph_xb2 | Graph_yb4,

    Graph_farc | Graph_xa1 | Graph_ya5 | Graph_aA | Graph_ax1 | Graph_ay1 | Graph_more,        //w
    Graph_farc | Graph_xa3 | Graph_ya5 | Graph_aA | Graph_ax1 | Graph_ay1 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya5 | Graph_xb0 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa2 | Graph_ya5 | Graph_xb2 | Graph_yb6 | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya5 | Graph_xb4 | Graph_yb8,

    Graph_fline | Graph_xa0 | Graph_ya8 | Graph_xb4 | Graph_yb4 | Graph_more,            //x
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb4 | Graph_yb8,

    Graph_farc | Graph_xa2 | Graph_ya6 | Graph_aA | Graph_ax2 | Graph_ay2 | Graph_more,        //y
    Graph_farc | Graph_xa2 | Graph_ya3 | Graph_aF | Graph_ax2 | Graph_ay2 | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya3 | Graph_xb4 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya6 | Graph_xb0 | Graph_yb8 | Graph_more,
    Graph_fline | Graph_xa1 | Graph_ya1 | Graph_xb2 | Graph_yb1,

    Graph_fline | Graph_xa0 | Graph_ya8 | Graph_xb4 | Graph_yb8 | Graph_more,            //z
    Graph_fline | Graph_xa4 | Graph_ya8 | Graph_xb0 | Graph_yb4 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb4 | Graph_yb4,

    Graph_farc | Graph_xa3 | Graph_yaA | Graph_aD | Graph_ax1 | Graph_ay3 | Graph_more,        //{
    Graph_farc | Graph_xa1 | Graph_ya6 | Graph_aC | Graph_ax1 | Graph_ay2 | Graph_more,
    Graph_farc | Graph_xa1 | Graph_yaA | Graph_aF | Graph_ax1 | Graph_ay2 | Graph_more,
    Graph_farc | Graph_xa3 | Graph_ya6 | Graph_aE | Graph_ax1 | Graph_ay3,

    Graph_fline | Graph_xa2 | Graph_ya3 | Graph_xb2 | Graph_ybD,                  //|

    Graph_farc | Graph_xa1 | Graph_yaA | Graph_aC | Graph_ax1 | Graph_ay3 | Graph_more,        //}
    Graph_farc | Graph_xa3 | Graph_ya6 | Graph_aD | Graph_ax1 | Graph_ay2 | Graph_more,
    Graph_farc | Graph_xa3 | Graph_yaA | Graph_aE | Graph_ax1 | Graph_ay2 | Graph_more,
    Graph_farc | Graph_xa1 | Graph_ya6 | Graph_aF | Graph_ax1 | Graph_ay3,

    Graph_farc | Graph_xa1 | Graph_ya8 | Graph_a8 | Graph_ax1 | Graph_ay1 | Graph_more,        //~
    Graph_farc | Graph_xa3 | Graph_ya8 | Graph_aA | Graph_ax1 | Graph_ay1,

/**
 * Vector font - custom characters ($7F+)
 */
    Graph_fline | Graph_xa2 | Graph_ya9 | Graph_xb0 | Graph_yb4 | Graph_more,            //delta
    Graph_fline | Graph_xa2 | Graph_ya9 | Graph_xb4 | Graph_yb4 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb4 | Graph_yb4,

    Graph_farc | Graph_xa2 | Graph_ya7 | Graph_a8 | Graph_ax2 | Graph_ay2 | Graph_more,        //omega
    Graph_farc | Graph_xa1 | Graph_ya7 | Graph_aE | Graph_ax1 | Graph_ay2 | Graph_more,
    Graph_farc | Graph_xa3 | Graph_ya7 | Graph_aF | Graph_ax1 | Graph_ay2 | Graph_more,
    Graph_fline | Graph_xa1 | Graph_ya5 | Graph_xb1 | Graph_yb4 | Graph_more,
    Graph_fline | Graph_xa3 | Graph_ya5 | Graph_xb3 | Graph_yb4 | Graph_more,
    Graph_fline | Graph_xa0 | Graph_ya4 | Graph_xb1 | Graph_yb4 | Graph_more,
    Graph_fline | Graph_xa4 | Graph_ya4 | Graph_xb3 | Graph_yb4,

    Graph_farc | Graph_xa2 | Graph_ya8 | Graph_a0 | Graph_ax1 | Graph_ay1,              //bullet

    0
};

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
