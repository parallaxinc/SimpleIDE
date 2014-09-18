''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
''File: MinuteSet.spin
''Emulates buttons that set alarm clock time.

PUB SetTimer | counter, divide

    dira[9..4]~~                             ' Set LED I/O pins to output

    repeat                                   ' Main loop

       'Delay for 1 ms.
       waitcnt(clkfreq/1000 + cnt)           ' Delay 1 ms

       {If a button is pressed...
       NOTE: Resetting the counter to -1 makes it possible to rapidly press
       and release the button and advance the minute display without the any
       apparent delay.}
       if ina[21] or ina[23]                 ' if a button is pressed
          counter++                          ' increment counter
       else                                  ' otherwise
          counter := -1                      ' set counter to -1
          
       'Reset minute overflows   
       if outa[9..4] == 63                   ' If 0 rolls over to 63
          outa[9..4] := 59                   ' reset to 59
       elseif outa[9..4] == 60               ' else if 59 increments to 60
          outa[9..4] := 0                    ' set to 0

       'Set counter ms time slice duration
       if counter > 2000                     ' If counter > 2000 (10 increments)
          divide := 50                       ' 50 ms between increments 
       else                                  ' otherwise
          divide := 200                      ' 200 ms between increments

       'If one of the ms time slices has elapsed   
       if counter // divide == 0             ' if a time slice has elapsed
          if ina[21]                         ' if P21 pushbutton is pressed
             outa[9..4]++                    ' increment outa[9..4]
          elseif ina[23]                     ' else if P23 pushbutton is pressed
             outa[9..4]--                    ' decrement outa[9..4]