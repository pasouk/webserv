#!/usr/bin/env python3
import os
import sys

print ("cgi1.py")
print("CONTENT_LENGTH: " + os.environ.get('CONTENT_LENGTH'))
n = int(os.environ.get('CONTENT_LENGTH'))
data = ""
for _ in range(n):
    c = sys.stdin.read(1)
    if not c:
        break
    data += c
print("Read on stdin: ", repr(data))   
#print (os.environ)