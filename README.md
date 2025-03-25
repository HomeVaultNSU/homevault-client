# homevault-client

## Build
Install dependencies:
```bash
apt install libcli11-dev libcurlpp-dev
```

Build:
```bash
cmake -B Build
cmake --build build
```

## Project structure
- `include` - public headers
- `cli` - CLI application
- `core` - core module, contains all requests to server
