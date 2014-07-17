{{Output.spin

Toggles two pins, one after another.}}

PUB Main
  Toggle(16, 3_000_000, 10)  'Toggle P16 ten times, 1/4 s each
  Toggle(17, 2_000_000, 20)  'Toggle P17 twenty times, 1/6 s each


PUB Toggle(Pin, Delay, Count)
{{Toggle Pin, Count times with Delay clock cycles in between.}}

  dira[Pin]~~                'Set I/O pin to output direction
  repeat Count               'Repeat for Count iterations
    !outa[Pin]               '  Toggle I/O Pin
    waitcnt(Delay + cnt)     '  Wait for Delay cycles
