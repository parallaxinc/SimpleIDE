''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' File: AddressBlinkerControlWithOffsets.spin
''
'' Another example cog manager that relies on an object that watches variables in its
'' parent object.
''
'' This one's start method only passes one variable address, but uses it as an anchor 
'' for two variables that are monitored by AddressBlinkerWithOffsets. 

CON
   
  _clkmode = xtal1 + pll16x
  _xinfreq = 5_000_000

VAR
   
  long pin, rateDelay
   
OBJ
   
  pst     : "Parallax Serial Terminal"
  AddrBlnk: "AddressBlinkerWithOffsets"
   
   
PUB TwoWayCom

  ''Send test messages and values to Parallax Serial Terminal.
 
  pst.Start(115_200)

  pin := 4
  rateDelay := 10_000_000

  AddrBlnk.start(@pin)

  dira[4..9]~~

  repeat

     pst.Str(String("Enter pin number: "))
     pin := pst.DecIn
     pst.Str(String("Enter delay ticks for 'rate':"))
     rateDelay := pst.DecIn
     pst.Char(pst#NL)

