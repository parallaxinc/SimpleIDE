''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' TestMessages.spin
'' Send text messages stored in the DAT block to Parallax Serial Terminal.

CON
  _clkmode = xtal1 + pll16x
  _xinfreq = 5_000_000

OBJ
  pst : "Parallax Serial Terminal"
   
PUB TestDatMessages | value, index

  pst.Start(115_200)
  waitcnt(clkfreq*2 + cnt)
  pst.Char(pst#CS)
  repeat
    repeat index from 0 to 4 
       pst.Str(@ValTxt)
       value := long[@valueList][index]
       pst.Dec(value)
       pst.Str(@ElNumTxt)
       pst.Dec(index)
       pst.Str(@BlankLine)
       waitcnt(clkfreq + cnt)

DAT

  ValTxt      byte   pst#NL, "The value is: ", 0  
  ElNumTxt    byte   ", ", pst#NL, "and its element #: ", 0 
  ValueList   long   98, 5282, 299_792_458, 254, 0
  BlankLine   byte   pst#NL, 0
