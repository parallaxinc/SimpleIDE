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
   
PUB TestDatMessages | value, counter

  ''Send messages stored in the DAT block.
 
  pst.Start(115_200)
  
  repeat
     pst.Str(@MyString)
     pst.Dec(counter++)
     pst.Str(@MyOtherString)
     pst.Str(@BlankLine)
     waitcnt(clkfreq + cnt)

DAT
  MyString        byte    "This is test message number: ", 0  
  MyOtherString   byte    ", ", pst#NL, "...and this is another line of text.", 0 
  BlankLine       byte    pst#NL, pst#NL, 0
