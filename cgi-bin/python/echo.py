#!/usr/bin/env python3
import sys, os

print("Content-Type: text/html")
print()

n = int(os.environ.get('CONTENT_LENGTH'))
data = sys.stdin.read(n)  # récupère le body (pour POST)
print("<html><body>")
print("<h2>Données reçues :</h2>")
print(f"<pre>{data}</pre>")
print("<hr>")
print("<h3>Environnement :</h3>")
for k, v in os.environ.items():
    print(f"{k}={v}<br>")
print("</body></html>")