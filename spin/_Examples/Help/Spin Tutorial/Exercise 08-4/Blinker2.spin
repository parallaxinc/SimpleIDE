{{ Blinker2.spin }}

CON
  _CLKMODE = XTAL1 + PLL4X     'Set to ext low-speed crystal, 4x PLL
  _XINFREQ = 5_000_000         'Frequency on XIN pin is 5 MHz
  MAXLEDS = 6                  'Number of LED objects to use


OBJ
  LED[MAXLEDS] : "Output"


PUB Main
{Toggle pins at different rates, simultaneously}

  dira[16..23]~~                                 'Set pins to outputs
  LED[NextObject].Start(16, 250,   0)            'Blink LEDs
  LED[NextObject].Start(17, 500,   0)
  LED[NextObject].Start(18,  50, 300)
  LED[NextObject].Start(19, 500,  40)
  LED[NextObject].Start(20,  29, 300)
  LED[NextObject].Start(21, 104, 250)
  LED[NextObject].Start(22,  63, 200)            '<-Postponed
  LED[NextObject].Start(23,  33, 160)            '<-Postponed
  LED[0].Start(20, 1000, 0)                      'Restart object 0
  repeat                                         'Loop endlessly

PUB NextObject : Index
{Scan LED objects and return index of next available LED object.
 Scanning continues until one is available.}

  repeat
    repeat Index from 0 to MAXLEDS-1
      if not LED[Index].Active
        quit
  while Index == MAXLEDS