''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' File: AddressBlinkerControlWithOffsets.spin
{{
Bs2IoLite.spin

This object features method calls similar to the PBASIC commands for the BASIC Stamp
2 microcontroller, such as high, low, in0 through in15, toggle, and pause.

}}

PUB high(pin)
''Make pin output-high.

    outa[pin]~~
    dira[pin]~~
    

PUB low(pin)
''Make pin output-low

    outa[pin]~
    dira[pin]~~
    

PUB in(pin) : state
{{Return the state of pin.
If pin is an output, state reflects the
output signal.  If pin is an input, state will be 1 if the voltage
applied to pin is above 1.65 V, or 0 if it is below.}}

    state := ina[pin]
    

PUB toggle(pin)
''Change pin's output state (high to low or low to high).
  
    !outa[pin]

PUB pause(ms) | time
''Make the program pause for a certain number of ms.  This applies to
''the cog making the call.  Other cogs will not be affected.

    time := ms * (clkfreq/1000)
    waitcnt(time + cnt)