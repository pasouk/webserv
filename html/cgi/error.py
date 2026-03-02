#!/usr/bin/env python3
# CGI script with a deliberate runtime error (division by zero)
import os

print("Content-Type: text/html\r")
print("\r")
print("<h1>This line prints before the error</h1>")

result = 1 / 0  # ZeroDivisionError

print("<p>This line is never reached</p>")
