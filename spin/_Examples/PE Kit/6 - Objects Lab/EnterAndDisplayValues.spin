''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' File: EnterAndDisplayValues.spin
'' Messages to/from Propeller chip with Parallax Serial Terminal. Prompts you to enter a
'' value, and displays the value in decimal, binary, and hexadecimal formats.

CON

  _clkmode = xtal1 + pll16x
  _xinfreq = 5_000_000
   

OBJ
   
  pst : "Parallax Serial Terminal"
  
   
PUB TwoWayCom | value

  ''Test Parallax Serial Terminal number entry and display.
 
  pst.Start(115_200)
  pst.Clear

  repeat

     pst.Str(String("Enter a decimal value: "))
     value := pst.DecIn
     pst.Str(String(pst#NL, "You Entered", pst#NL, "--------------"))
     pst.Str(String(pst#NL, "Decimal: "))
     pst.Dec(value)
     pst.Str(String(pst#NL, "Hexadecimal: "))
     pst.Hex(value, 8)
     pst.Str(String(pst#NL, "Binary: "))
     pst.Bin(value, 32)
     repeat 2
        pst.NewLine     
   