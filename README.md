# homevault-client

## Build
Install dependencies:
```bash
apt install libcli11-dev libcurlpp-dev nlohmann-json3-dev
```

Build:
```bash
cmake -B build
cmake --build build
```

#### Build doxygen documentation
```
cmake --build build -t docs
```

## Project structure
- `include` - public headers
- `cli` - CLI application
- `core` - core module, contains all requests to server
