#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

// Link with ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

int main()
{
    int startPort, endPort;
    cout << "Enter start and end port to scan (e.g 1 1024): ";
    cin >> startPort >> endPort;

    cout << startPort << " " << endPort << "\n";
    
    string ip_adrr;
    cout << "Enter IP address: ";
    cin >> ip_adrr;

    WSAData wsaData;
    int startup = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (startup != 0)
    {
        cout << "Error: " << startup << "\n";
        return 1;
    }

    for (int port = startPort; port <= endPort; port++)
    {
                            // ipv4   // tcp conn
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET)
        {
            cout << "Socket creation failed: " << WSAGetLastError() << "\n";
            WSACleanup();
            return 1;
        }

        // Non-blocking mode socket
        u_long mode = 1;
        if (ioctlsocket(sock, FIONBIO, &mode) != 0)
        {
            cout << "ioctlsocket failed: " << WSAGetLastError() << "\n";
            closesocket(sock);
            WSACleanup();
            return 1;
        }
        
        sockaddr_in target;
        target.sin_family = AF_INET;
        target.sin_port = htons(port);
        inet_pton(AF_INET, ip_adrr.c_str(), &target.sin_addr);

        // Estabilish connection
        int res = connect(sock, (sockaddr*)&target, sizeof(target));
        if (res == SOCKET_ERROR)
        {  
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK)
            {
                fd_set writeSet;
                FD_ZERO(&writeSet);
                FD_SET(sock, &writeSet);

                TIMEVAL timeout;
                timeout.tv_sec = 0;
                timeout.tv_usec = 300000;

                int selRes = select(0, nullptr, &writeSet, nullptr, &timeout);
                if (selRes > 0 && FD_ISSET(sock, &writeSet))
                    cout << "Port [" << port << "] Open!\n";
                else
                    cout << "Port [" << port << "] is closed or filtered (err: " << err << ")!\n";
            }    
            
            else
                cout << "Connection failed due to an error: " << err << "\n";
        }
        closesocket(sock);
    }

    WSACleanup();
    return 0;
}