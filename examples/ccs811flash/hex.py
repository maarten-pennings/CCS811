# hex.py - converts any file to a c-style array
# 2020 05 17  Maarten Pennings  v3  Uses f-string, added extra NL
# 2019 01 15  Maarten Pennings  v2  Added const/PROGMEM to array
# 2018 12 05  Maarten Pennings  v1  Created
import sys;
import datetime;
import os.path

# This script needs Python 3.6

# Is a filename passed?
if len(sys.argv)!=2:
    print( "SYNTAX:" )
    print( "  python hex.py <file>" )
    sys.exit(1)

# Does file exists?
name= sys.argv[1] # 'CCS811_FW_App_v2-0-0.bin'
if not os.path.isfile(name):
    print( f"File '{name}' does not exist" )
    sys.exit(2)

# Process file
n= 0;
f= open(name, 'rb')
try:
    print( f"// Hex dump of '{name}' created at {datetime.datetime.now()}" )
    print( "" )
    print( "#include <stdint.h>" )
    print( "" )
    print( f"const char * image_name=\"{name}\";" )
    print( "const uint8_t image_data[] PROGMEM = {" )
    byte= f.read(1)
    while byte:
        if( n%16==0 ): print( "  ", end="" )
        print( f"0x{byte[0]:02x}, ", end="" )
        if( n%16==7 ): print( "  ", end="" )
        if( n%16==15 ): print()
        byte= f.read(1)
        n= n+1
    if( n%16!=0 ): print()
    print( "};" )
finally:
    f.close()

