''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
''HelloPST.spin
''Test message to Parallax Serial Terminal.

CON
   
  _clkmode = xtal1 + pll16x
  _xinfreq = 5_000_000
   
OBJ
   
  pst : "Parallax Serial Terminal"
   
PUB TestMessages

  ''Send test messages to Parallax Serial Terminal.
 
  pst.Start(115_200)

  repeat
    pst.Str(string("This is a test message!"))
    Pst.NewLine
    waitcnt(clkfreq + cnt)

