''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
''File: TickTock.spin

VAR

  long stack[50]
  byte cog
  long days, hours, minutes, seconds  


PUB Start(setDay, setHour, setMinutes, setSeconds) : success
{{
Track time in another cog.

  Parameters - starting values for:
    setDay     - day
    setHour    - hour
    setMinutes - minute
    setSeconds - second
}}

  days := setDay
  hours := setHour
  minutes := setMinutes
  seconds := setSeconds
  
  Stop
  cog := cognew(GoodTimeCount, @stack)
  success := cog + 1


PUB Stop
''Stop counting time.

  if Cog
    cogstop(Cog~ - 1)
    

PUB Get(dayAddr, hourAddr, minAddr, secAddr) | time
{{
Get the current time.  Values are loaded into variables at the
addresses provided to the method parameters.

  Parameters:
    dayAddr  -  day variable address
    hourAddr -  hour variable address 
    minAddr  -  minute variable address
    secAddr  -  secondAddress
}}

  long[dayAddr]  := days
  long[hourAddr] := hours 
  long[minAddr]  := minutes
  long[secAddr]  := seconds
  

PRI GoodTimeCount | dT, T

  dT := clkfreq
  T  := cnt
  
  repeat
    
    T += dT
    waitcnt(T)
    seconds ++

    if seconds == 60
       seconds~
       minutes++
    if minutes == 60
       minutes~
       hours++
    if hours == 24
       hours~
       days++