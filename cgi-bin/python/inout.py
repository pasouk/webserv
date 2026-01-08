#!/usr/bin/env python3
import os
import sys

n = int(os.environ.get('CONTENT_LENGTH'))
data = sys.stdin.read(n)  # récupère le body (pour POST)
print(f"{data}")

#if os.environ.get('REQUEST_METHOD') == 'POST':
#    n = int(os.environ.get('CONTENT_LENGTH'))
#    data = ""
#    for _ in range(n):
#        c = sys.stdin.read(1)
#        if not c:
#            break
#        data += c
#    print("Read on stdin: ", data)
