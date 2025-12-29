#!/usr/bin/env python3
import os
import urllib.parse

# read query string (ex: "a=5&b=3")
query = os.environ.get("QUERY_STRING", "")
# parameters parsed
params = urllib.parse.parse_qs(query)
a = int(params.get("a", ["0"])[0])
b = int(params.get("b", ["0"])[0])
result = a + b
# Headers HTTP
print("Content-Type: text/html")
print()
# Body
print(f"""
<html>
<head><title>Résultat</title></head>
<body>
<p>Résultat : {result}</p>
</body>
</html>
""")
