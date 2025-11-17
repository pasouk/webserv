#!/usr/bin/env python3
import os
#read CGI variables
script_name = os.environ.get("SCRIPT_NAME", "")
path_info = os.environ.get("PATH_INFO", "")
script_filename = os.environ.get("SCRIPT_FILENAME", "")
request_method = os.environ.get("REQUEST_METHOD", "")
query_string = os.environ.get("QUERY_STRING", "")
content_length = os.environ.get("CONTENT_LENGTH", "0")
server_protocol = os.environ.get("SERVER_PROTOCOL", "")
server_name = os.environ.get("SERVER_NAME", "")
server_port = os.environ.get("SERVER_PORT", "")

# Headers CGI
print("Content-Type: text/plain")
print()

print(f"SCRIPT_NAME={script_name}")
print(f"PATH_INFO={path_info}")
print(f"SCRIPT_FILENAME={script_filename}")
print(f"REQUEST_METHOD={request_method}")
print(f"QUERY_STRING={query_string}")
print(f"CONTENT_LENGTH={content_length}")
print(f"SERVER_PROTOCOL={server_protocol}")
print(f"SERVER_NAME={server_name}")
print(f"SERVER_PORT={server_port}")
print