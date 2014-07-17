''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
''FloatStringTest.spin
''Solve a floating point math problem and display the result with Parallax Serial
''Terminal.

CON
   
  _clkmode = xtal1 + pll16x
  _xinfreq = 5_000_000
   

OBJ
   
  pst     : "Parallax Serial Terminal"
  fMath   : "FloatMath"
  fString : "FloatString"
   
   
PUB TestFloat | a, b, c

  '' Solve a floating point math problem and display the result.
 
  pst.Start(115_200)

  a := 1.5
  b := pi

  c := fmath.FAdd(a, b)

  pst.Str(String("1.5 + Pi = "))

  pst.Str(fstring.FloatToString(c)) 
 
