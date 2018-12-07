# hex.py - converts any file to a c-style array
# 2018 12 05  Maarten Pennings  v1  Created
import sys;
import datetime;
import os.path

# Is a filename passed?
if len(sys.argv)!=2:
    print("SYNTAX:")
    print("  python hex.py <file>")
    sys.exit(1)

# Does file exists?
name= sys.argv[1] # 'CCS811_FW_App_v2-0-0.bin'
if not os.path.isfile(name):
    print("File '"+name+"' does not exist")
    sys.exit(2)

# Process file
n= 0;
f= open(name, 'rb')
try:
    print( "// Hex dump of '"+name+"' created at " + str(datetime.datetime.now()) )
    print( "" )
    print( "#include <stdint.h>" )
    print( "" )
    print( "char * image_name=\""+name+"\";" )
    print( "uint8_t image_data[]= {" )
    byte= f.read(1)
    while byte:
        if( n%16==0 ): print( "  ", end="" )
        print( "0x{}, ".format(byte.hex()), end="" )
        if( n%16==7 ): print( "  ", end="" )
        if( n%16==15 ): print()
        byte= f.read(1)
        n= n+1
    print( "};" )
finally:
    f.close()


