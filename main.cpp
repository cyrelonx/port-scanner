#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

// Link with ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

int main()
{
    int port;
    cout << "Enter port number: ";
    cin >> port;
    
    string ip_adrr;
    cout << "Enter IP address: ";
    cin >> ip_adrr;
    cout << "Port: " << port << ", IP Address: " << ip_adrr << endl;

    WSAData wsaData;
    int startup = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (startup != 0)
    {
        cout << "Error: " << startup << "\n";
        return 1;
    }
                        // ipv4   // tcp conn
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        cout << "Socket creation failed: " << WSAGetLastError() << "\n";
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
        if (err == WSAECONNREFUSED || err == WSAETIMEDOUT)
            cout << "Port is closed or filtered (err: " << err << ")!\n";
        else
            cout << "Connection failed due to an error: " << err << "\n";
    }
    else
        cout << "Port is open!\n";

    closesocket(sock);
    WSACleanup();
    return 0;
}