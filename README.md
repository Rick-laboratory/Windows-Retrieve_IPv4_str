# Local IPv4 Lookup for Windows

A compact C++ example that resolves the local computer name and returns its first IPv4 address as a `std::string`.

```text
IPv4: 192.0.2.10
```

The address above is illustrative. Actual results depend on the computer's name resolution and network configuration.

## Build and run

Requirements: Windows, CMake 3.16+, and a C++17 compiler with the Windows SDK or MinGW-w64 headers.

```sh
cmake -S . -B build
cmake --build build --config Release
```

Run `build/Release/local-ipv4.exe` with a multi-configuration generator. With Ninja or Makefiles, run `build/local-ipv4.exe`. For MinGW-w64 and Ninja, configure with `-G Ninja -DCMAKE_BUILD_TYPE=Release`.

The original Visual Studio 2019 project is also included; newer Visual Studio installations may need to retarget its v142 toolset. Both build paths link Winsock (`Ws2_32`).

```powershell
.\build\local-ipv4.exe
.\build\local-ipv4.exe --help
```

Exit codes: `0` on success, `1` if a Windows or resolver call fails, `2` for invalid arguments. The program exits immediately after printing its result, making it usable from scripts.

## How the example works

1. A scoped `WinsockSession` pairs `WSAStartup` with `WSACleanup`.
2. `GetComputerNameA` fills a correctly sized, checked buffer.
3. [`getaddrinfo`](https://learn.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-getaddrinfo) requests IPv4 results only. A smart pointer releases the returned list, including on exceptions.
4. [`inet_ntop`](https://learn.microsoft.com/en-us/windows/win32/api/ws2tcpip/nf-ws2tcpip-inet_ntop) formats the first IPv4 address into a caller-owned buffer.

The reusable function retains the name `Retrieve_IPv4_str()`. It returns an address or throws `std::runtime_error`; error text is never returned as an apparent address.

## Scope

This is a **hostname-resolution example**, not an adapter inventory or public-IP service. A machine can have several addresses; the resolver's first result need not belong to the preferred network interface. The example does not open a connection to a public IP lookup service. For adapter enumeration, use the Windows IP Helper API instead.

## License

This repository does not currently include a license file.
