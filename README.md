# Networks-Algos

## Router Simulation Program

### Description

The router simulation program allows users to create a network of routers and analyze routing tables to find the shortest path between routers using Dijkstra's algorithm.

### Prerequisites

- C++ compiler supporting C++11 standard or later

### How to Run

1. Clone the repository or download the source code files.
2. Compile the source code using a C++ compiler.
3. Run the compiled executable.

### Usage

Upon running the program, users are presented with a menu to perform various actions:

- **Create Router and Add Neighbors**: Create routers and specify incoming and outgoing neighbors.
- **Find Path**: Find the shortest path between two routers.
- **Print Routing Tables**: Display routing tables of all routers in the network.
- **Delete Router**: Delete a router from the network.
- **Exit**: Exit the program.

Follow the on-screen instructions to perform desired actions.

# DNS Resolver in C++

This is a basic **DNS Resolver** program written in C++. It allows the user to input a domain name and queries a DNS server to resolve it to an IP address (supports IPv4). The program also includes a simple caching mechanism to avoid repeated lookups.

---

## Features

- Sends custom DNS query packets over UDP  
- Parses DNS responses for IPv4 (A) records and CNAMEs  
- Retries on timeout (up to 3 attempts)  
- Maintains a local cache for faster repeated queries  

---

## Usage

1. **Compile the code:**

```bash
g++ -o dns_resolver dns_resolver.cpp
```

2. **Run the program:**

```bash
./dns_resolver
```

3. **Follow the prompt:**
Enter the hostname: google.com


4. **Type `exit` to quit:**
Enter the hostname: exit


---

## Requirements
- Linux environment (uses POSIX networking headers)
- g++ compiler (C++11 or higher)
- Internet connection to reach DNS servers

---

## Notes

- The DNS server is hardcoded as 172.17.1.1 in the code.
  You can replace it with a public DNS server like 8.8.8.8 (Google DNS).

- Only supports IPv4 (A) records and CNAMEs.

- Uses raw UDP sockets and constructs DNS packets manually.
  Does not require root access.
