set $sp = *0x00000000
set $pc = *0x00000004

break ResetHandler
break main
continue
