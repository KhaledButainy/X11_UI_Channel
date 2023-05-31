# X11 Client-Server Application (Linux)

This is a simple client-server application that allows the client to send X11 events (mouse movements, button presses, key presses) to the server machine.

## Prerequisites

- Linux operating system
- C++ compiler (e.g., g++)
- X11 development libraries (Linux)
```
sudo apt update
```
```
sudo apt install libx11-dev libxext-dev libxtst-dev libxt-dev
```

## Installation

1. Clone the repository:
```
git clone <repository-url>
```

2. Change to the project directory:
```
cd <repo_directory>
```

3. Compile the server code:
```
g++ -o server server.cpp -lX11 -lpthread
```

4. Compile the client code:
```
g++ -o client client.cpp -lX11
```

## Usage

### Server

1. Run the server:
```
./server
```

2. The server will start listening for client connections on the announced IP and Port (e.g., 127.0.0.1:8888). Make a note of the server IP address and port.

### Client

1. Run the client:
```
./client
```

The client will prompt you to enter the server IP address and Port. Press enter to use the default settings.

2. Control the server machine:
- Move the mouse: The client will send the mouse movement events to the server, which will mimic the mouse movement on the server machine.
- Press/release mouse buttons: The client will send the button press/release events to the server.
- Press/release keyboard keys: The client will send the key press/release events to the server.

3. Press the `Esc` key on the client machine to close the connection and exit the client program.

## Notes

- Make sure you have the necessary permissions to run the server and client programs.
- If your client machine is not in the same network as your server machine, then you need to establish a VPN connection first.
- If you encounter any issues related to X11 libraries or missing headers, make sure you have the X11 development libraries installed on your system.




