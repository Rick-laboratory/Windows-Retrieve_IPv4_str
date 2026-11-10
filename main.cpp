#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif

namespace {
class WinsockSession {
public:
    WinsockSession() {
        WSADATA data{};
        const int error = WSAStartup(MAKEWORD(2, 2), &data);
        if (error != 0) {
            throw std::runtime_error("WSAStartup failed: " + std::to_string(error));
        }
    }
    ~WinsockSession() { WSACleanup(); }
    WinsockSession(const WinsockSession&) = delete;
    WinsockSession& operator=(const WinsockSession&) = delete;
};
} // namespace

// Resolve the local computer name and return the first IPv4 address.
// This does not determine the public Internet address or the preferred adapter.
std::string Retrieve_IPv4_str() {
    const WinsockSession winsock;

    std::array<char, MAX_COMPUTERNAME_LENGTH + 1> computer_name{};
    DWORD name_length = static_cast<DWORD>(computer_name.size());
    if (!GetComputerNameA(computer_name.data(), &name_length)) {
        throw std::runtime_error("GetComputerName failed: " +
                                 std::to_string(GetLastError()));
    }

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* raw_addresses = nullptr;
    const int error = getaddrinfo(computer_name.data(), nullptr, &hints, &raw_addresses);
    if (error != 0) {
        throw std::runtime_error("getaddrinfo failed: " + std::to_string(error));
    }
    const std::unique_ptr<addrinfo, decltype(&freeaddrinfo)>
        addresses(raw_addresses, &freeaddrinfo);

    for (const addrinfo* entry = addresses.get(); entry != nullptr; entry = entry->ai_next) {
        if (entry->ai_family != AF_INET || entry->ai_addr == nullptr ||
            entry->ai_addrlen < sizeof(sockaddr_in)) {
            continue;
        }
        const auto* address = reinterpret_cast<const sockaddr_in*>(entry->ai_addr);
        std::array<char, INET_ADDRSTRLEN> text{};
        if (inet_ntop(AF_INET, &address->sin_addr, text.data(), text.size()) == nullptr) {
            throw std::runtime_error("inet_ntop failed: " +
                                     std::to_string(WSAGetLastError()));
        }
        return text.data();
    }
    throw std::runtime_error("The local computer name has no IPv4 address.");
}

int main(int argc, char* argv[]) {
    if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        std::cout << "Usage: local-ipv4\n"
                     "Resolve the local computer name and print its first IPv4 address.\n";
        return 0;
    }
    if (argc != 1) {
        std::cerr << "Usage: local-ipv4 [--help]\n";
        return 2;
    }
    try {
        const std::string address = Retrieve_IPv4_str();
        std::cout << "IPv4: " << address << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
