set $sp = *0x20000000
set $pc = *0x20000004

break ResetHandler
break main
continue
