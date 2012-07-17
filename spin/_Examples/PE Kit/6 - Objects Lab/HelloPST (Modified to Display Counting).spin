''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
''HelloPST (Modified to Display Counting).spin
''Test message to Parallax Serial Terminal.  Modified to display the value of
''a variable as it is incremented within a repeat loop.

CON
   
  _clkmode = xtal1 + pll16x
  _xinfreq = 5_000_000
   
OBJ
   
  pst : "Parallax Serial Terminal"

PUB TestMessages | counter
  
  ''Send test messages to Parallax Serial Terminal.
 
  pst.Start(115_200)

  repeat
    pst.Str(String("counter = "))
    pst.Dec(counter++)
    pst.NewLine
    waitcnt(clkfreq/5 + cnt)
