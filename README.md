# ComputerNetworks

## Description

This project implements a simple TCP Echo Server and Client in C. The server listens for connections from clients and echoes back any messages it receives.

## Files

- `server.c`: Source code for the TCP Echo Server
- `client.c`: Source code for the TCP Echo Client
- `server`: Compiled executable for the TCP Echo Server
- `client`: Compiled executable for the TCP Echo Client

## Features

- **TCP Echo Server**: Listens on port 25020 and echoes back messages from clients
- **TCP Echo Client**: Connects to the server and sends a predefined greeting message
- **Multi-client support**: Server can handle multiple client connections sequentially
- **Error handling**: Both programs include comprehensive error checking

## How to Use

### Prerequisites
- GCC compiler
- Unix/Linux environment (for socket programming)

### Compile (if needed)
To recompile the server and client from source:
```bash
gcc server.c -o server
gcc client.c -o client
```

### Run the Server
Start the server first (it will listen on localhost:25020):
```bash
./server
```
The server will display:
```
TCP ECHO SERVER.
SERVER: Listening for clients... Press Ctrl + C to stop the server.
```

### Run the Client
In a separate terminal, start the client:
```bash
./client
```
The client will:
1. Connect to the server
2. Send the message "===GOOD AFTERNOON==="
3. Receive and display the echoed message
4. Disconnect

## Configuration

Both programs use the following default settings:
- **IP Address**: `127.0.0.1` (localhost)
- **Port**: `25020`
- **Buffer Size**: 128 bytes

To modify these settings, edit the following variables in both `server.c` and `client.c`:
```c
unsigned short serv_port = 25020;  // Change port number
char serv_ip[] = "127.0.0.1";       // Change IP address
```

## Example Output

**Server Output:**
```
TCP ECHO SERVER.
SERVER: Listening for clients... Press Ctrl + C to stop the server.
SERVER: Connection from client 127.0.0.1 accepted.
SERVER: Echoed back ===GOOD AFTERNOON=== to 127.0.0.1.
```

**Client Output:**
```
TCP ECHO CLIENT
CLIENT: Connected to the server.
CLIENT: Message sent to echo server.
CLIENT: Message from the server: ===GOOD AFTERNOON===
```

## Notes

- Ensure the server is running before starting the client
- The server handles one client at a time and closes the connection after echoing the message
- Press `Ctrl + C` to stop the server
- This implementation is designed for educational purposes and local testing

## Author

Created as part of Computer Networks coursework.
