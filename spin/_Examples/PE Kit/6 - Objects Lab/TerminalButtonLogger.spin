''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' TerminalButtonLogger.spin
'' Log times the button connected to P23 was pressed/released in 
'' Parallax Serial Terminal.

CON
  _clkmode = xtal1 + pll16x
  _xinfreq = 5_000_000
   

OBJ
  pst        : "Parallax Serial Terminal"
  Button     : "Button"
  Time       : "TickTock"

  
VAR
  long days, hours, minutes, seconds
  
                     
PUB TestDatMessages 
 
  pst.Start(115_200)                    ' Start Parallax Serial Terminal object.


  Time.Start(0, 0, 0, 0)                ' Start the TickTock object and initialize 
                                        ' the day, hour, minute, and second.
  pst.Str(@BtnPrompt)                   ' Display instructions in Parallax Serial
                                        ' Terminal
  repeat
  
    if Button.Time(23)                  ' If button pressed.
       ' Pass variables to TickTock object for update.
       Time.Get(@days, @hours, @minutes, @seconds)
       DisplayTime                      ' Display the current time.
       

PUB DisplayTime
      
      pst.Char(pst#NL)
      pst.Str(String("Day:"))
      pst.Dec(days)
      pst.Str(String("  Hour:"))
      pst.Dec(hours)
      pst.Str(String("  Minute:"))
      pst.Dec(minutes)
      pst.Str(String("  Second:"))
      pst.Dec(seconds)
      

DAT

BtnPrompt   byte    pst#CS, "Press/release P23 pushbutton periodically...", 0
  
