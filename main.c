/*
Tomasz Stachowski
309675
*/

#include "error.h"
#include "transport.h"

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        fprintf(stderr, "Usage: ./transport ip port output_file bytes\n");
        exit(EXIT_FAILURE);
    }

    struct in_addr ip;
    Inet_pton(AF_INET, argv[1], &(ip.s_addr));

    int port = atoi(argv[2]);
    if (port > 65535)
    {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }

    FILE *file_ptr = Fopen(argv[3], "wb");

    int bytes = atoi(argv[4]);
    if (bytes <= 0)
    {
        fprintf(stderr, "Invalid number of bytes\n");
        exit(EXIT_FAILURE);
    }

    transport(ip.s_addr, port, file_ptr, bytes);

    Fclose(file_ptr);
    return 0;
}