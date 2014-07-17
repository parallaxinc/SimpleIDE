''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' File: AddressBlinkerWithOffsets.spin
'' Example cog manager that watches variables in its parent object
'' Parent object should declare a long that stores the LED I/O pin number
'' followed by a long that stores the number of click ticks between each
'' LED state change.  It should pass the address of the long that stores
'' the LED I/O pin number to the Start method.

VAR
  long  stack[10]                      'Cog stack space
  byte  cog                            'Cog ID

PUB Start(baseAddress) : success
''Start new blinking process in new cog; return True if successful.
''
''baseAddress.......the address of the long variable that stores the LED pin number.
''baseAddress + 1...the address of the long variable that stores the blink rate delay.  

  Stop
  success := (cog := cognew(Blink(baseAddress), @stack) + 1)


PUB Stop
''Stop blinking process, if any.

  if Cog
    cogstop(Cog~ - 1)

PRI Blink(baseAddress) | pin, rate, pinOld, rateOld

    pin       :=   long[baseAddress][0]
    rate      :=   long[baseAddress][1]
    pinOld    :=   pin
    rateOld   :=   rate

    repeat
       pin := long[baseAddress][0]
       dira[pin]~~
       if pin <> pinOld
         dira[pinOld]~
       !outa[pin]
       pinOld := pin
       rate := long[baseAddress][1]
       waitcnt(rate/2 + cnt)