{{Output.spin

Toggles Pin with Delay clock cycles of high/low time.}}

CON
  Pin   = 16                 { I/O pin to toggle on/off }
  Delay = 3_000_000          { On/Off Delay, in clock cycles}

PUB Toggle
''Toggle Pin forever
{Toggles I/O pin given by Pin and waits Delay system clock cycles in between each toggle.}

  dira[Pin]~~                'Set I/O pin to output direction
  repeat                     'Repeat following endlessly
    !outa[Pin]               '  Toggle I/O Pin
    waitcnt(Delay + cnt)     '  Wait for Delay cycles