
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cout << "Winsock initialization failed.\n";
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cout << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(serverSocket, (sockaddr*)&serverAddress,
             sizeof(serverAddress)) == SOCKET_ERROR) {
        cout << "Port 8080 is busy or bind failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        cout << "Listen failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "DCIMS server running at http://127.0.0.1:8080\n";
    cout << "Keep this terminal open. Press Ctrl+C to stop.\n";

    while (true) {
        SOCKET client = accept(serverSocket, nullptr, nullptr);
        if (client == INVALID_SOCKET) continue;

        char buffer[4096] = {};
        int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);

        string path = "/";
        if (bytes > 0) {
            string request(buffer, bytes);
            size_t firstSpace = request.find(' ');
            size_t secondSpace = request.find(' ', firstSpace + 1);
            if (firstSpace != string::npos &&
                secondSpace != string::npos) {
                path = request.substr(firstSpace + 1,
                                      secondSpace - firstSpace - 1);
            }
        }

        
string body;
string contentType = "text/html; charset=utf-8";
string filePath = "public/index.html";

if (path == "/api/cases") {
    ifstream dataFile("cases.txt");
    string id, type, location, priority, description, status;
    body = "[";
    bool first = true;

    while (getline(dataFile, id)) {
        if (!getline(dataFile, type) ||
            !getline(dataFile, location) ||
            !getline(dataFile, priority) ||
            !getline(dataFile, description) ||
            !getline(dataFile, status)) {
            break;
        }

        if (!first) body += ",";
        first = false;

        body += "{\"id\":\"" + id +
                "\",\"type\":\"" + type +
                "\",\"location\":\"" + location +
                "\",\"priority\":\"" + priority +
                "\",\"status\":\"" + status + "\"}";
    }

    body += "]";
    contentType = "application/json; charset=utf-8";
}

        if (path == "/style.css") {
            filePath = "public/style.css";
            contentType = "text/css; charset=utf-8";
        } else if (path == "/app.js") {
            filePath = "public/app.js";
            contentType = "application/javascript; charset=utf-8";
        }

        
if (path != "/api/cases") {
    ifstream file(filePath, ios::binary);

    if (file) {
        ostringstream content;
        content << file.rdbuf();
        body = content.str();
    } else {
        body = "<h1>DCIMS file not found</h1>";
    }
}

        string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: " + contentType + "\r\n"
            "Content-Length: " + to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n" + body;

        send(client, response.c_str(),
             static_cast<int>(response.size()), 0);
        closesocket(client);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
