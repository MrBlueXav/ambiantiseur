# connect to gdb server
target extended-remote localhost:4242

# Vector table placed in Flash
set *0xE000ED08 = 0x08000000
