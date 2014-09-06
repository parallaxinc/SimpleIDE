{{ Display.spin }}

CON
  _clkmode = xtal1 + pll16x
  _xinfreq = 5_000_000

OBJ
  Num   :       "Numbers"
  TV    :       "TV_Terminal"
  
PUB Main | Temp
  Num.Init                                    'Initialize Numbers   
  TV.Start(12)                                'Start TV Terminal    
                                                                              
  Temp := 900 * 45 + 401                      'Evaluate expression  
  TV.Str(string("900 * 45 + 401 = "))         'then display it and  
  TV.Str(Num.ToStr(Temp, Num#DDEC))           'its result in decimal
  TV.Out(13)                                                                  
  TV.Str(string("In hexadecimal it's  = "))   'and in hexadecimal   
  TV.Str(Num.ToStr(Temp, Num#IHEX))                                           
  TV.Out(13)                                                                  
  TV.Out(13)                                                                  
                                                                              
  TV.Str(string("Counting by fives:"))        'Now count by fives   
  TV.Out(13)
  repeat Temp from 5 to 30 step 5
    TV.Str(Num.ToStr(Temp, Num#DEC))
    if Temp < 30
      TV.Out(",")