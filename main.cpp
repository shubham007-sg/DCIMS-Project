#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

struct CrimeCase {
    int id;
    string crimeType;
    string location;
    string status;
    string priority;
};

class DCIMS {
private:
    vector<CrimeCase> cases;
    const string dataFile = "cases.csv";

    static string jsonEscape(const string& s) {
        string out;
        for (char c : s) {
            if (c == '"' || c == '\\') out += '\\';
            if (c == '\n' || c == '\r') continue;
            out += c;
        }
        return out;
    }

    static string getJsonValue(const string& body, const string& key) {
        string token = "\"" + key + "\"";
        size_t p = body.find(token);
        if (p == string::npos) return "";
        p = body.find(':', p + token.size());
        if (p == string::npos) return "";
        p++;
        while (p < body.size() && (body[p] == ' ' || body[p] == '\t')) p++;
        if (p >= body.size()) return "";
        if (body[p] == '"') {
            p++;
            string value;
            while (p < body.size()) {
                if (body[p] == '\\' && p + 1 < body.size()) {
                    value += body[p + 1];
                    p += 2;
                } else if (body[p] == '"') {
                    break;
                } else {
                    value += body[p++];
                }
            }
            return value;
        }
        size_t end = body.find_first_of(",}", p);
        return body.substr(p, end == string::npos ? string::npos : end - p);
    }

    static string readFile(const string& path) {
        ifstream f(path, ios::binary);
        if (!f) return "";
        ostringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    static string contentType(const string& path) {
        if (path.size() >= 5 && path.substr(path.size()-5) == ".html") return "text/html; charset=utf-8";
        if (path.size() >= 4 && path.substr(path.size()-4) == ".css") return "text/css; charset=utf-8";
        if (path.size() >= 3 && path.substr(path.size()-3) == ".js") return "application/javascript; charset=utf-8";
        return "text/plain; charset=utf-8";
    }

    void load() {
        cases.clear();
        ifstream file(dataFile);
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string id, type, location, status, priority;
            if (getline(ss, id, '|') && getline(ss, type, '|') &&
                getline(ss, location, '|') && getline(ss, status, '|') &&
                getline(ss, priority, '|')) {
                try {
                    CrimeCase c;
                    c.id = stoi(id);
                    c.crimeType = type;
                    c.location = location;
                    c.status = status;
                    c.priority = priority;
                    cases.push_back(c);
                } catch (...) {}
            }
        }
    }

    void save() {
        ofstream file(dataFile, ios::trunc);
        for (const auto& c : cases) {
            file << c.id << "|" << c.crimeType << "|" << c.location
                 << "|" << c.status << "|" << c.priority << "\n";
        }
    }

    string casesJson() const {
        ostringstream out;
        out << "[";
        for (size_t i = 0; i < cases.size(); ++i) {
            if (i) out << ",";
            const auto& c = cases[i];
            out << "{\"id\":" << c.id
                << ",\"crimeType\":\"" << jsonEscape(c.crimeType)
                << "\",\"location\":\"" << jsonEscape(c.location)
                << "\",\"status\":\"" << jsonEscape(c.status)
                << "\",\"priority\":\"" << jsonEscape(c.priority) << "\"}";
        }
        out << "]";
        return out.str();
    }

    static string response(const string& status, const string& type, const string& body) {
        ostringstream out;
        out << "HTTP/1.1 " << status << "\r\n"
            << "Content-Type: " << type << "\r\n"
            << "Content-Length: " << body.size() << "\r\n"
            << "Connection: close\r\n"
            << "Cache-Control: no-store\r\n\r\n" << body;
        return out.str();
    }

    void sendResponse(SOCKET client, const string& status, const string& type, const string& body) {
        string r = response(status, type, body);
        send(client, r.c_str(), (int)r.size(), 0);
    }

    void handleClient(SOCKET client) {
        char buffer[16384] = {0};
        int received = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) return;
        string request(buffer, received);
        size_t headerEnd = request.find("\r\n\r\n");
        if (headerEnd == string::npos) {
            sendResponse(client, "400 Bad Request", "application/json", "{\"error\":\"Bad request\"}");
            return;
        }

        string headers = request.substr(0, headerEnd);
        string body = request.substr(headerEnd + 4);
        istringstream firstLine(request);
        string method, path, version;
        firstLine >> method >> path >> version;

        size_t cl = headers.find("Content-Length:");
        if (cl != string::npos) {
            size_t start = cl + 15;
            while (start < headers.size() && headers[start] == ' ') start++;
            size_t end = headers.find("\r\n", start);
            int expected = 0;
            try { expected = stoi(headers.substr(start, end - start)); } catch (...) {}
            while ((int)body.size() < expected && received > 0) {
                char more[4096];
                int n = recv(client, more, sizeof(more), 0);
                if (n <= 0) break;
                body.append(more, n);
            }
        }

        if (method == "GET" && (path == "/" || path == "/index.html")) {
            sendResponse(client, "200 OK", "text/html; charset=utf-8", readFile("public/index.html"));
        } else if (method == "GET" && path == "/style.css") {
            sendResponse(client, "200 OK", "text/css; charset=utf-8", readFile("public/style.css"));
        } else if (method == "GET" && path == "/app.js") {
            sendResponse(client, "200 OK", "application/javascript; charset=utf-8", readFile("public/app.js"));
        } else if (method == "GET" && path == "/api/cases") {
            load();
            sendResponse(client, "200 OK", "application/json; charset=utf-8", casesJson());
        } else if (method == "POST" && path == "/api/cases") {
            load();
            string idText = getJsonValue(body, "id");
            string type = getJsonValue(body, "crimeType");
            string location = getJsonValue(body, "location");
            string priority = getJsonValue(body, "priority");
            int id = 0;
            try { id = stoi(idText); } catch (...) {}
            if (id <= 0 || type.empty() || location.empty()) {
                sendResponse(client, "400 Bad Request", "application/json", "{\"error\":\"Please provide valid ID, crime type and location.\"}");
            } else {
                bool exists = false;
                for (const auto& c : cases) if (c.id == id) exists = true;
                if (exists) {
                    sendResponse(client, "409 Conflict", "application/json", "{\"error\":\"That Case ID already exists.\"}");
                } else {
                    CrimeCase c{id, type, location, "Pending", priority.empty() ? "Medium" : priority};
                    cases.push_back(c);
                    save();
                    sendResponse(client, "201 Created", "application/json", "{\"message\":\"Case created\"}");
                }
            }
        } else if (method == "PATCH" && path.rfind("/api/cases/", 0) == 0) {
            load();
            int id = 0;
            try { id = stoi(path.substr(string("/api/cases/").size())); } catch (...) {}
            string status = getJsonValue(body, "status");
            bool updated = false;
            for (auto& c : cases) {
                if (c.id == id) {
                    c.status = status.empty() ? c.status : status;
                    updated = true;
                    break;
                }
            }
            if (updated) {
                save();
                sendResponse(client, "200 OK", "application/json", "{\"message\":\"Status updated\"}");
            } else {
                sendResponse(client, "404 Not Found", "application/json", "{\"error\":\"Case not found\"}");
            }
        } else if (method == "DELETE" && path.rfind("/api/cases/", 0) == 0) {
            load();
            int id = 0;
            try { id = stoi(path.substr(string("/api/cases/").size())); } catch (...) {}
            auto oldSize = cases.size();
            cases.erase(remove_if(cases.begin(), cases.end(), [id](const CrimeCase& c) { return c.id == id; }), cases.end());
            if (cases.size() != oldSize) {
                save();
                sendResponse(client, "200 OK", "application/json", "{\"message\":\"Case deleted\"}");
            } else {
                sendResponse(client, "404 Not Found", "application/json", "{\"error\":\"Case not found\"}");
            }
        } else {
            sendResponse(client, "404 Not Found", "text/plain; charset=utf-8", "Not found");
        }
    }

public:
    void run() {
        load();
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            cerr << "Could not initialize Windows networking.\n";
            return;
        }
        SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server == INVALID_SOCKET) {
            cerr << "Could not create server socket.\n";
            WSACleanup();
            return;
        }
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = htons(8080);

        if (bind(server, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            cerr << "Could not use port 8080. Close any other server using this port and try again.\n";
            closesocket(server);
            WSACleanup();
            return;
        }
        listen(server, SOMAXCONN);
        cout << "\nDCIMS server is running!\n";
        cout << "Open this address in your browser: http://127.0.0.1:8080\n";
        cout << "Keep this terminal open while using the app.\n";
        cout << "Press Ctrl+C here to stop the server.\n\n";

        while (true) {
            SOCKET client = accept(server, nullptr, nullptr);
            if (client == INVALID_SOCKET) continue;
            handleClient(client);
            shutdown(client, SD_SEND);
            closesocket(client);
        }
        closesocket(server);
        WSACleanup();
    }
};

int main() {
    DCIMS app;
    app.run();
    return 0;
}
