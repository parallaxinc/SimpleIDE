''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
''HelloPST (Modified for 57.6 kpbs).spin
''Test message to Parallax Serial Terminal.  Modified for a baud rate of
''57.6 kbps.
''
''IMPORTANT: Make sure to adjust the Baud Rate dropdown menu in your
''           Parallax Serial Terminal software to 57600 so that it is
''           compatible with the baud rate this code makes the Propeller
''           chip use for communication.  
''
''           Also, before moving on to the next example program, set your
''           Parallax Serial Terminal software's Baud Rate dropdown menu
''           back to 115200.

CON
   
  _clkmode = xtal1 + pll16x
  _xinfreq = 5_000_000
   
OBJ
   
  pst : "Parallax Serial Terminal"
   
PUB TestMessages

  ''Send test messages to Parallax Serial Terminal.
 
  pst.Start(57_600)

  repeat
    pst.Str(string("This is a test message!"))
    Pst.NewLine
    waitcnt(clkfreq + cnt)
