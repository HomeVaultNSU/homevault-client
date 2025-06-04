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

## How to use CLI
1. export needed env variables
```bash
export HV_FILE_HOSTNAME=http://localhost:8080
export HV_AUTH_HOSTNAME=http://localhost:8090
export HV_USERNAME=admin
export HV_PASSWORD=admin
```

2. register user with `register` command
```bash
homevault-cli register
```

3. now you can use `list`, `upload`, `download` commands
