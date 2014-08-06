{{ Blinker2.spin }}

CON
  MAXLEDS = 6                'Number of LED objects to use

OBJ
  LED[MAXLEDS] : "Output"


PUB Main
{Toggle pins at different rates, simultaneously}

  dira[16..23]~~                                 'Set pins to outputs
  LED[NextObject].Start(16, 3_000_000,   0)      'Blink LEDs
  LED[NextObject].Start(17, 2_000_000,   0)
  LED[NextObject].Start(18,   600_000, 300)
  LED[NextObject].Start(19, 6_000_000,  40)
  LED[NextObject].Start(20,   350_000, 300)
  LED[NextObject].Start(21, 1_250_000, 250)
  LED[NextObject].Start(22,   750_000, 200)      '<-Postponed 
  LED[NextObject].Start(23,   400_000, 160)      '<-Postponed
  LED[0].Start(20, 12_000_000, 0)                'Restart object 0
  repeat                                         'Loop endlessly


PUB NextObject : Index
{Scan LED objects and return index of next available LED object.
 Scanning continues until one is available.}

  repeat
    repeat Index from 0 to MAXLEDS-1
      if not LED[Index].Active
        quit
  while Index == MAXLEDS