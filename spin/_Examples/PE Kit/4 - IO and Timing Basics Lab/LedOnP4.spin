''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' File: LedOnP4.spin

PUB LedOn                          ' Method declaration
                                        
    dira[4] := 1                   ' Set P4 to output
    outa[4] := 1                   ' Set P4 high

    repeat                         ' Endless loop prevents program from ending