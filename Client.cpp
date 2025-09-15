#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#define PACKET_SIZE 1024
#define BUFFER_SIZE 1020
#define TIMEOUT 2000

using namespace std;

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "Russian");

    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        cout << "Не удалось открыть сокет" << endl;
        return -1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Не удалось открыть сокет" << endl;
        WSACleanup();
        return -1;
    }

    sockaddr_in clientAddress;
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = 0;
    clientAddress.sin_addr.s_addr = INADDR_ANY;
    if (bind(clientSocket, (sockaddr*)&clientAddress, sizeof(sockaddr)) == SOCKET_ERROR) {
        cout << "Не удалось связать сокет" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return -2;
    }
    int clientAddressLength = sizeof(sockaddr_in); 
    if (getsockname(clientSocket, (sockaddr*) &clientAddress, &clientAddressLength) == SOCKET_ERROR) {
        cout << "Не удалось получить адрес" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return -3;
    } 

    cout << "Клиент работает на порте № " << ntohs(clientAddress.sin_port) << endl;

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(atoi(argv[2]));
    InetPtonA(AF_INET, argv[1], &serverAddress.sin_addr.s_addr);
    int serverAddressLength = sizeof(sockaddr_in);

    int timeout = TIMEOUT;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(int)) == SOCKET_ERROR) {
        cout << "Не удалось настроить сокет" << endl;
        return -4;
    }

    ifstream file(string(argv[3]) + ".txt", ios::in | ios::binary);
    if (!file.is_open()) {
        cout << "Не удалось открыть файл" <<  string(argv[3]) << ".txt" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return -5;
    }
    char packet[PACKET_SIZE], buffer[BUFFER_SIZE];
    
    int clientIndex = atoi(argv[3]);

    while (true) {
        file.read(buffer, BUFFER_SIZE);
        int bytesRead = file.gcount();
        if (bytesRead <= 0) {
            cout << "Передача завершена" << endl;
            break;
        }
        cout.write(buffer, bytesRead);
        cout << endl;
        memcpy(packet, &clientIndex, sizeof(int));
        memcpy(packet + 4, buffer, bytesRead);
        int bytesSent, receivedBytes, attempts = 0;
        do {
            Sleep(100);
            attempts += 1;
            if (attempts > 1) {
                cout << "Попытка отправки № " << attempts << endl;
            }
            bytesSent = sendto(clientSocket, packet, bytesRead + 4, 0, (sockaddr*)&serverAddress, serverAddressLength);
            receivedBytes = recvfrom(clientSocket, packet, 3, 0, (sockaddr*)&serverAddress, &serverAddressLength);
            if (receivedBytes != SOCKET_ERROR) {
                cout << "Получено подтверждение" << endl;
            } else if (attempts == 5) {
                cout << "Пропуск" << endl;
            }
        } while (receivedBytes == SOCKET_ERROR && attempts <= 5);
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}