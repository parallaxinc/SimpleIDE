''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
''File: DotNotationExample.spin

OBJ

  PbLed : "ButtonAndBlink"
    

PUB Main | time

    repeat

       time := PbLed.ButtonTime(23)

       PbLed.Blink(4, time, 20)