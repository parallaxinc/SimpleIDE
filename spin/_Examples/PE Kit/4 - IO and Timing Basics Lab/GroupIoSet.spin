''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' File: GroupIoSet.spin

PUB LedsOn

    dira[4..9] := %111111
    outa[4..9] := %101010

    repeat