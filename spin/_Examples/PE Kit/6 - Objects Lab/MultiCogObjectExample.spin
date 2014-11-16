''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
''Top File: MultiCogObjectExample.spin

OBJ

    Blinker[6] : "Blinker"
    Button     : "Button"


PUB ButtonBlinkTime | time, index

    repeat
    
       repeat index from 0 to 5
          time := Button.Time(23)
          Blinker[index].Start(index + 4, time, 1_000_000)
       
       repeat index from 5 to 0
          Button.Time(23)
          Blinker[index].Stop