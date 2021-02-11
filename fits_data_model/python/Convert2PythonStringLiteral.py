#!/usr/bin/env python3 

# Prints out a python string literal that is the properly escaped version
# of the input, to be used with .format()

import sys


T=str.maketrans(dict(
    (("\n","\\n\\\n"),("\"","\\\""),("\\","\\\\"),("{","{{"),("}","}}"),
 )))

out=["\"\\\n",]
for L in sys.stdin:
    out.append(L.translate(T))

out.append("\"")

print("".join(out))



