''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' File: AddressBlinker.spin
'' Example cog manager that watches variables in its parent object 


VAR
  long  stack[10]                      'Cog stack space
  byte  cog                            'Cog ID


PUB Start(pinAddress, rateAddress) : success
''Start new blinking process in new cog; return True if successful.
''Parameters: pinAddress - long address of the variable that stores the I/O pin
''            rateAddress - long address of the variable that stores the rate

  Stop
  success := (cog := cognew(Blink(pinAddress, rateAddress), @stack) + 1)


PUB Stop
''Stop blinking process, if any.

  if Cog
    cogstop(Cog~ - 1)


PRI Blink(pinAddress, rateAddress) | pin, rate, pinOld, rateOld

    pin       :=   long[pinAddress]
    rate      :=   long[rateAddress]
    pinOld    :=   pin
    rateOld   :=   rate

    repeat
       pin := long[pinAddress]
       dira[pin]~~
       if pin <> pinOld
         dira[pinOld]~
       !outa[pin]
       pinOld := pin
       rate := long[rateAddress]
       waitcnt(rate/2 + cnt)