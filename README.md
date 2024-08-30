# My Proxy Server

A simple proxy server built using C to handle TCP connections.

## Features

- Supports basic TCP forwarding
- Configurable remote host and port
- Lightweight and efficient

## Installation

1. Clone the repository:

```bash
git clone https://github.com/GrantGZS/ProxyServerC.git

```
##Usage
1. cd to source
2. compile the proxy server with command: make
3. run program ./tcpproxy [host_ip] [host_port] [client_port]
4. in another terminal use telnet [client_ip] [client_port] command to proxy and GET / to fetch data through proxy server from host ip
   
