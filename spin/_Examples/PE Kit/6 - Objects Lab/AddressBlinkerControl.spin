''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' AddressBlinkerControl.spin
'' Enter LED states into Parallax Serial Terminal and send to Propeller chip via 
'' Parallax Serial Terminal.

CON
   
  _clkmode = xtal1 + pll16x
  _xinfreq = 5_000_000
 
  
OBJ
   
  pst     : "Parallax Serial Terminal"
  AddrBlnk: "AddressBlinker"

   
VAR
  
  long pin, rateDelay
   

PUB UpdateVariables

  '' Update variables that get watched by AddressBlinker object.
 
  pst.Start(115_200)

  pin := 4
  rateDelay := 10_000_000

  AddrBlnk.Start(@pin, @rateDelay)

  dira[4..9]~~

  repeat

     pst.Str(String("Enter pin number: "))
     pin := pst.DecIn
     pst.Str(String("Enter delay clock ticks:"))
     rateDelay := pst.DecIn
     pst.Str(String(pst#NL))

