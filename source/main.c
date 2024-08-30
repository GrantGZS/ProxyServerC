#include "tcpproxy.h"
#include <stdio.h>

// show the usage
static void usage(void) {
    printf("Usage: tcpproxy remote_host remote_port proxy_server_port\n");
}

// the main function
int main(int argc, char **argv) {
    // check and get the argument
    if (argc != 4) {
        usage();
        exit(1);
    }

    char *remote_host = argv[1];
    char *remote_port = argv[2];
    char *proxy_port = argv[3];

    // start the proxy
    start_proxy(remote_host, remote_port, proxy_port);

    return 0;
}