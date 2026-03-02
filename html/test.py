#!/usr/bin/env python3
import os
import sys
import datetime

body = ""
if os.environ.get("REQUEST_METHOD") == "POST":
    length = int(os.environ.get("CONTENT_LENGTH", 0))
    if length > 0:
        body = sys.stdin.read(length)

now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

print("Content-Type: text/html\r")
print("\r")
print("""<!DOCTYPE html>
<html><head>
<style>
body {{ font-family: monospace; max-width: 600px; margin: 40px auto; padding: 0 20px; background: #fafafa; color: #222; }}
h1 {{ font-size: 1.2em; border-bottom: 2px solid #222; padding-bottom: 8px; }}
section {{ background: #fff; border: 1px solid #ddd; border-radius: 4px; padding: 16px; margin-bottom: 16px; }}
h2 {{ font-size: 0.95em; color: #555; margin-bottom: 8px; }}
table {{ border-collapse: collapse; width: 100%; font-size: 0.85em; }}
td {{ padding: 4px 8px; border-bottom: 1px solid #eee; }}
td:first-child {{ font-weight: bold; white-space: nowrap; color: #555; }}
input[type=text], textarea {{ font-family: monospace; width: 100%; padding: 6px; border: 1px solid #ccc; border-radius: 3px; margin: 4px 0; }}
button {{ font-family: monospace; padding: 6px 16px; background: #222; color: #fff; border: none; border-radius: 3px; cursor: pointer; }}
.ok {{ color: #2a2; font-weight: bold; }}
.body-box {{ background: #f0f0f0; padding: 10px; border-radius: 3px; white-space: pre-wrap; word-break: break-all; }}
</style>
</head><body>
<h1>CGI test</h1>

<section>
<h2>Status</h2>
<p class="ok">CGI is working</p>
<table>
<tr><td>Time</td><td>{now}</td></tr>
<tr><td>Method</td><td>{method}</td></tr>
<tr><td>Python</td><td>{pyver}</td></tr>
</table>
</section>
""".format(
    now=now,
    method=os.environ.get("REQUEST_METHOD", "?"),
    pyver=sys.version.split()[0],
))

if body:
    print("""<section>
<h2>POST body received ({length} bytes)</h2>
<div class="body-box">{body}</div>
</section>""".format(length=len(body), body=body[:500]))

print("""<section>
<h2>Test POST</h2>
<form method="POST" action="/cgi/test.py">
<input type="text" name="message" placeholder="Type something...">
<button type="submit">Send via POST</button>
</form>
</section>

<section>
<h2>Environment</h2>
<table>""")

for key in sorted(os.environ):
    if key.startswith(("SERVER_", "REQUEST_", "QUERY_", "CONTENT_", "HTTP_", "PATH_", "SCRIPT_", "REMOTE_", "GATEWAY_")):
        print("<tr><td>{0}</td><td>{1}</td></tr>".format(key, os.environ[key]))

print("""</table>
</section>

<p><a href="/">Back to home</a></p>
</body></html>""")
