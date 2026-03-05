*This project has been created as part of the 42 common core by fbuyl, magillet.*

# webserv

## Description

webserv is an HTTP/1.1 web server written , built from scratch as part of the 42 school common core. The goal is to understand how a real web server works under the hood 
from accepting TCP connections to parsing HTTP requests and sending back proper responses.

The server handles multiple clients simultaneously using poll() for non-blocking I/O (no threads, no fork per request). It supports:

- **GET, POST, DELETE** methods
- **CGI** execution (Python scripts, custom binaries) via fork/execve with pipes
- **Virtual hosting** — multiple servers on the same port, routed by the Host header
- **Chunked Transfer Encoding** and Content-Length body handling
- **File uploads** (multipart/form-data)
- **Custom error pages**, configurable per server or location
- **Autoindex** — directory listing when no index file is found
- **Location blocks** with method restrictions, aliases, and per-location body size limits

The configuration file syntax is inspired by nginx.

## Instructions

### Requirements

- g++ with C++98 support
- Linux 

### Compilation

```bash
make
```

This produces the `webserv` binary.

### Running

```bash
./webserv config_file
```

### Configuration file

 nginx-like syntax.  minimal example:

```nginx
http {
    keepalive_timeout 12;
    client_max_body_size 4096;

    server {
        listen 127.0.0.1:8080;
        server_name localhost;
        root ./html;
        index index.html;

        location / {
            limit_except GET {
                deny all;
            }
        }

        location *.py {
            cgi_pass /usr/bin/python3;
        }
    }
}
```

Key directives: `listen`, `server_name`, `root`, `index`, `alias`, `limit_except`, `cgi_pass`, `client_max_body_size`, `error_page`, `return`, `autoindex`.

### Testing

we mainly used the browser and 2 script tests:

```bash
# Official tester (provided by 42)
./webserv tests_confs/default_tester.conf &
yes "" | timeout 300 ./tester http://localhost:8080

# Our tester
bash my_tester/my_tester.sh
```

## Resources

### HTTP / Networking

- [RFC 7230 — HTTP/1.1 Message Syntax and Routing](https://datatracker.ietf.org/doc/html/rfc7230)
- [RFC 7231 — HTTP/1.1 Semantics and Content](https://datatracker.ietf.org/doc/html/rfc7231)
- [RFC 3875 — The Common Gateway Interface (CGI/1.1)](https://datatracker.ietf.org/doc/html/rfc3875)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [MDN Web Docs — HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP)
- [nginx documentation](https://nginx.org/en/docs/) — reference for config file syntax inspiration

### Use of AI

We used Claude (Anthropic) during this project, mainly for:
- **Understanding the required bases** - Http norm, sockets management, poll...
- **Understanding edge cases** — HTTP spec behavior for chunked encoding for exemple
- **Code review** — checking that our poll() usage respected the subject constraints fpr exemple
- **Stress testing analysis** — investigating why the 20-client × 100MB CGI POST test was failing for exemple

AI was not used to write the core architecture or the main logic of the server.
