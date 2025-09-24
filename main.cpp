#include <iostream>
#include <bits/stdc++.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>

using namespace std;

// Link with ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

mutex mtx;
vector<pair<int, bool>> results;

bool scanPort(const string& ip_addr, int port)
{
                        // ipv4   // tcp conn
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        cout << "Socket creation failed: " << WSAGetLastError() << "\n";
        return false;
    }

    // Non-blocking mode socket
    u_long mode = 1;
    if (ioctlsocket(sock, FIONBIO, &mode) == SOCKET_ERROR)
    {
        closesocket(sock);
        return false;
    }
    
    sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    inet_pton(AF_INET, ip_addr.c_str(), &target.sin_addr);

    // Estabilish connection
    int res = connect(sock, (sockaddr*)&target, sizeof(target));
    if (res == 0)
    {
        closesocket(sock);
        return true;
    }

    int err = WSAGetLastError();
    if (err != WSAEWOULDBLOCK)
    {
        closesocket(sock);
        return false;
    }

    fd_set writeSet;
    FD_ZERO(&writeSet);
    FD_SET(sock, &writeSet);

    TIMEVAL timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 300000;

    int selRes = select(0, nullptr, &writeSet, nullptr, &timeout);
    if (selRes <= 0)
    {
        closesocket(sock);
        return false;
    }

    int
        sockErr = 0,
        optLen = sizeof(sockErr);
    
    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&sockErr), &optLen) == SOCKET_ERROR)
    {
        closesocket(sock);
        return false;
    }

    closesocket(sock);
    return (sockErr == 0);
}

int main()
{
    int startPort, endPort;
    cout << "Enter start and end port to scan (e.g 1 1024): ";
    cin >> startPort >> endPort;

    cout << startPort << " " << endPort << "\n";
    
    string ip_addr;
    cout << "Enter IP address: ";
    cin >> ip_addr;

    WSAData wsaData;
    int startup = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (startup != 0)
    {
        cout << "Error: " << startup << "\n";
        return 1;
    }

    atomic<int> nextPort(startPort);
    unsigned nThreads = thread::hardware_concurrency();
    if (!nThreads)
        nThreads = 4; // Fallback

    vector<thread> threads;

    auto worker = [&]()
    {
        while (true)
        {
            int port = nextPort.fetch_add(1);
            if (port > endPort)
                break;

            bool open = scanPort(ip_addr, port);
            lock_guard<mutex> lg(mtx);
            results.emplace_back(port, open);
        }
    };

    for (unsigned i = 0; i < nThreads; i++)
        threads.emplace_back(worker);

    for (auto &t : threads)
        t.join();

    sort(results.begin(), results.end());
    for (auto &[port, open] : results)
    {
        if (open)
        {
            cout << "\x1B[32mPort [" << port << "] Open!\n";
        }
        else
        {
            cout << "\x1B[33mPort [" << port << "] is closed or filtered!\n";
        }
    }

    WSACleanup();
    
    system("pause");
    return 0;
}