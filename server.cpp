#include <iostream>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <sstream>
#include <thread>
#include <vector>
#include <unistd.h>

#define SERVER_PORT 8888

std::string getIPv4Address()
{
    std::string ifconfigCmd = "ifconfig";
    std::string ifconfigResult;

    // Execute ifconfig command and capture the output
    FILE* pipe = popen(ifconfigCmd.c_str(), "r");
    if (pipe)
    {
        char buffer[128];
        while (!feof(pipe))
        {
            if (fgets(buffer, 128, pipe) != NULL)
            {
                ifconfigResult += buffer;
            }
        }
        pclose(pipe);
    }

    // Parse the output to get the IPv4 address
    std::string ipAddress;
    std::istringstream stream(ifconfigResult);
    std::string line;
    while (std::getline(stream, line))
    {
        if (line.find("inet ") != std::string::npos)
        {
            std::istringstream lineStream(line);
            std::string word;
            lineStream >> word; // Skip the "inet" keyword
            lineStream >> ipAddress;
            break;
        }
    }

    if (ipAddress.empty())
    {
        // Return a default address if no IPv4 address found
        ipAddress = "127.0.0.1";
    }

    return ipAddress;
}

void sendMouseEvent(Display* display, Window root, int x, int y, int button, bool isPress)
{
    XEvent event{};
    memset(&event, 0, sizeof(event));

    event.xbutton.root = root;
    event.xbutton.subwindow = None;
    event.xbutton.x = x;
    event.xbutton.y = y;
    event.xbutton.button = button;
    event.xbutton.same_screen = True;
    event.xbutton.time = CurrentTime;
    event.xbutton.state = isPress ? ButtonPressMask : ButtonReleaseMask;

    XQueryPointer(display, root, &event.xbutton.root, &event.xbutton.window,
                  &event.xbutton.x_root, &event.xbutton.y_root,
                  &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);

    event.type = isPress ? ButtonPress : ButtonRelease;
    event.xbutton.window = event.xbutton.subwindow;
    event.xbutton.subwindow = None;

    XSendEvent(display, PointerWindow, True, ButtonPressMask | ButtonReleaseMask, &event);
    XFlush(display);
}

void sendKeyEvent(Display* display, Window root, unsigned int keycode, bool isPress)
{
    XEvent event{};
    memset(&event, 0, sizeof(event));
    printf("Kello World!");
    event.xkey.display = display;
    event.xkey.window = root;
    event.xkey.root = root;
    event.xkey.subwindow = None;
    event.xkey.keycode = keycode;
    event.xkey.same_screen = True;
    event.xkey.time = CurrentTime;
    event.xkey.state = isPress ? 0 : KeyRelease;

    event.type = isPress ? KeyPress : KeyRelease;

    XSendEvent(display, root, True, KeyPressMask | KeyReleaseMask, &event);
    XFlush(display);
}

void handleClient(int clientSockfd)
{
    Display* display = XOpenDisplay(nullptr);
    Window root = DefaultRootWindow(display);

    XEvent event;
    XGrabPointer(display, root, False, PointerMotionMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | Button4MotionMask | Button5MotionMask | Button4Mask | Button5Mask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
    XGrabKeyboard(display, root, False, GrabModeAsync, GrabModeAsync, CurrentTime);

    while (true)
    {
        ssize_t bytesRead = recv(clientSockfd, &event, sizeof(event), 0);
        if (bytesRead <= 0)
        {
            break;
        }

        switch (event.type)
        {
            case MotionNotify:
                std::cout << "Mouse move: x=" << event.xmotion.x << ", y=" << event.xmotion.y << std::endl;
                sendMouseEvent(display, root, event.xmotion.x, event.xmotion.y, 0, true);
                break;
            case ButtonPress:
                std::cout << "Button press: " << event.xbutton.button << std::endl;
                sendMouseEvent(display, root, event.xbutton.x, event.xbutton.y, event.xbutton.button, true);
                break;
            case ButtonRelease:
                std::cout << "Button release: " << event.xbutton.button << std::endl;
                sendMouseEvent(display, root, event.xbutton.x, event.xbutton.y, event.xbutton.button, false);
                break;
            case KeyPress:
                std::cout << "Key press: keycode=" << event.xkey.keycode << ", state=" << event.xkey.state << std::endl;
                sendKeyEvent(display, root, event.xkey.keycode, true);
                break;
            case KeyRelease:
                std::cout << "Key release: keycode=" << event.xkey.keycode << ", state=" << event.xkey.state << std::endl;
                sendKeyEvent(display, root, event.xkey.keycode, false);
                break;
            default:
                break;
        }
    }

    XUngrabPointer(display, CurrentTime);
    XUngrabKeyboard(display, CurrentTime);
    XCloseDisplay(display);

    // Indicate that the client is disconnected
    std::cout << "Client disconnected" << std::endl;
}

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    if (bind(sockfd, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0)
    {
        std::cerr << "Failed to bind socket" << std::endl;
        close(sockfd);
        return 1;
    }

    if (listen(sockfd, 1) < 0)
    {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(sockfd);
        return 1;
    }

    std::cout << "Server listening on " << getIPv4Address() << ":" << SERVER_PORT << std::endl;

    std::vector<std::thread> clientThreads;

    while (true)
    {
        // Accept a client connection
        int clientSockfd = accept(sockfd, nullptr, nullptr);
        if (clientSockfd < 0)
        {
            std::cerr << "Failed to accept client connection" << std::endl;
            close(sockfd);
            return 1;
        }

        std::cout << "Client connected. Receiving events..." << std::endl;

        // Handle client connection in a separate thread
        std::thread clientThread(handleClient, clientSockfd);
        clientThreads.push_back(std::move(clientThread));
    }

    // Join all client threads
    for (auto& clientThread : clientThreads)
    {
        if (clientThread.joinable())
        {
            clientThread.join();
        }
    }

    return 0;
}
