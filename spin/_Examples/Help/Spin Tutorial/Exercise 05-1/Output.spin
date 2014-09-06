{{ Output.spin }}

VAR
  long  Stack[9]             'Stack space for new cog

PUB Start(Pin, Delay, Count)
{{Start new toggling process in a new cog.}}

  cognew(Toggle(Pin, Delay, Count), @Stack)

PUB Toggle(Pin, Delay, Count)
{{Toggle Pin, Count times with Delay clock cycles in between.}}

  dira[Pin]~~                'Set I/O pin to output direction
  repeat Count               'Repeat for Count iterations
    !outa[Pin]               '  Toggle I/O Pin
    waitcnt(Delay + cnt)     '  Wait for Delay cycles
