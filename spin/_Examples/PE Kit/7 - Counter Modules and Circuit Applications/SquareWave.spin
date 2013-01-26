''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' SquareWave.spin
'' Can be used to make either or both of a given cog's counter modules transmit square
'' waves.

PUB Freq(Module, Pin, Frequency) | s, d, ctr

'' Determine CTR settings for synthesis of 0..128 MHz in 1 Hz steps
''
'' in:    Pin = pin to output frequency on
''        Freq = actual Hz to synthesize
''
'' out:   ctr and frq hold ctra/ctrb and frqa/frqb values
''
''   Uses NCO mode %00100 for 0..499_999 Hz
''   Uses PLL mode %00010 for 500_000..128_000_000 Hz
''

  Frequency := Frequency #> 0 <# 128_000_000     'limit frequency range
  
  if Frequency < 500_000               'if 0 to 499_999 Hz,
    ctr := constant(%00100 << 26)      '..set NCO mode
    s := 1                             '..shift = 1

  else                                 'if 500_000 to 128_000_000 Hz,
    ctr := constant(%00010 << 26)      '..set PLL mode
    d := >|((Frequency - 1) / 1_000_000)    'determine PLLDIV
    s := 4 - d                         'determine shift
    ctr |= d << 23                     'set PLLDIV
    
  spr[10 + module] := fraction(Frequency, CLKFREQ, s)    'Compute frqa/frqb value
  ctr |= Pin                           'set PINA to complete ctra/ctrb value
  spr[8 + module] := ctr

  dira[pin]~~
  

PUB NcoFrqReg(frequency) : frqReg
{{
Returns frqReg = frequency × (2³² ÷ clkfreq) calculated with binary long
division.  This is faster than the floating point library, and takes less
code space.  This method is an adaptation of the CTR object's fraction
method.
}}
  frqReg := fraction(frequency, clkfreq, 1)


PRI fraction(a, b, shift) : f

  if shift > 0                         'if shift, pre-shift a or b left
    a <<= shift                        'to maintain significant bits while 
  if shift < 0                         'insuring proper result
    b <<= -shift
 
  repeat 32                            'perform long division of a/b
    f <<= 1
    if a => b
      a -= b
      f++           
    a <<= 1