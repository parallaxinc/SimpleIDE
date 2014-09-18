''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
''File: AddressBlinkerControlWithOffsets.spin
''Top File: TestBs2IoLiteObject.spin
''Turn P6 LED on for 1 s, then flash P5 LED at 5 Hz whenever the 
''P21 pushbutton is held down.

OBJ

    stamp : "Bs2IoLite"


PUB ButtonBlinkTime | time, index

    stamp.high(6)            ' Set P6 to output-high
    stamp.pause(1000)        ' Delay 1 s
    stamp.low(6)             ' Set P6 to output-low
    stamp.low(5)             ' Set P5 to output-low
    repeat                   ' Repeat (like DO...LOOP in PBASIC)
      if stamp.in(21)        ' If P21 pushbutton pressed
        stamp.toggle(5)      ' Toggle P5 output state
      else
        stamp.low(5)
      stamp.pause(100)       ' Delay 0.1 s before repeat