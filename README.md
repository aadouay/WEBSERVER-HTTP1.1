# WEBSERVER-HTTP1.1

> A lightweight, high-performance HTTP/1.1 web server built from scratch in C++98, featuring asynchronous I/O with epoll, CGI support, and JSON-based configuration.

## About

This project is a fully functional HTTP/1.1 web server implemented in C++98. It handles concurrent client connections using Linux's epoll for asynchronous I/O multiplexing, serves static files, supports dynamic routing, URL redirects, file uploads, CGI script execution, and user authentication — all configured through a simple JSON file.

## Features

- **Asynchronous I/O** — Non-blocking event loop powered by Linux epoll
- **HTTP Methods** — GET, POST, and DELETE support
- **Static File Serving** — Serve HTML, images, videos, and other static assets
- **Directory Listing** — Auto-generated directory indexes
- **URL Redirects** — Internal and external redirect support
- **CGI Execution** — Run dynamic scripts (e.g. Python) via CGI
- **File Uploads** — Upload files with configurable body size limits
- **User Authentication** — Built-in login and registration system
- **JSON Configuration** — Simple, readable server configuration
- **Custom Error Pages** — Configurable error responses
- **Concurrent Connections** — Handle multiple clients simultaneously

## Requirements

- Linux (uses epoll)
- g++ with C++98 support
- Python 3 (for CGI scripts)
- Make

## Build

```bash
make        # Build the webserver binary
make clean  # Remove object files
make fclean # Remove object files and the binary
make re     # Clean rebuild
```

## Usage

```bash
./webserver
```

The server reads its configuration from `.server/.config/example.json` by default.

## Configuration

The server is configured via a JSON file. Here is an example:

```json
{
  "servers": [
    {
      "port": "3000",
      "name": "my-app",
      "version": "0.1.0",
      "index": "index.html",
      "root": "./",
      "body_limit": "10485760",
      "routes": [
        { "path": "/upload", "source": "upload.html" },
        { "path": "/delete", "source": "delete/deleteme.html", "method": ["GET", "DELETE"] },
        { "path": "/list", "dictlist": "true" },
        { "path": "/redirect1", "redirect": "/redirect2" },
        {
          "path": "/cgi",
          "cgi_script": "python.py",
          "cgi_interpreter": "/usr/bin/python3"
        }
      ]
    }
  ]
}
```

### Configuration Options

| Option | Description |
|---|---|
| `port` | Port number the server listens on |
| `name` | Server name (used to locate the app directory) |
| `version` | Application version |
| `index` | Default index file |
| `root` | Root directory for serving files |
| `body_limit` | Maximum request body size in bytes |

### Route Options

| Option | Description |
|---|---|
| `path` | URL path for the route |
| `source` | Static file to serve |
| `method` | Allowed HTTP methods (defaults to GET) |
| `redirect` | URL to redirect to |
| `dictlist` | Enable directory listing (`"true"`) |
| `cgi_script` | CGI script filename |
| `cgi_interpreter` | Path to the CGI interpreter |

## Project Structure

```
.server/
├── main/           # Entry point
├── lib/            # Core headers (server, request, response, client, etc.)
├── parse/          # Configuration and JSON parsing
├── run/            # Event loop, epoll handling, and HTTP methods
│   ├── method/     # GET, POST, DELETE, and CGI handlers
│   └── utils/      # Helper utilities
└── .config/        # Server configuration files

app/
└── my-app/         # Sample application with static pages
```
