#!/usr/bin/env python3
# CGI script with an infinite loop - server should not crash
# The server will keep this connection alive but remain responsive to others
import os

# Infinite loop - never outputs headers, never exits
while True:
    pass
