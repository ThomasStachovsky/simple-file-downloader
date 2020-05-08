/*
Tomasz Stachowski
309675
*/

#include "error.h"
#include "transport.h"

#define WINDOW_SIZE (1024 + 512)
#define MAX_PACKET 1000
#define EMPTY 0
#define REQUESTED 1
#define TO_SAVE 2
#define DEFAULT_TIMER 50000
#define NONE -1
#define REQUEST_STRING_SIZE 50
#define NO_PACKETS -1
#define DUPLICATE -2
#define DONE 1
#define NOT_DONE 0

static in_addr_t ip;
static int port;
static FILE *file_ptr;
static int bytes;
static char window[WINDOW_SIZE][MAX_PACKET];
static suseconds_t window_time[WINDOW_SIZE];
static int window_status[WINDOW_SIZE];
static int window_packet_number[WINDOW_SIZE];
static int last_packet_window_index;
static int last_packet_size;
static int full_packets;
static int all_packets;
static int next_to_save;
static int next_to_get;
static int downloaded_packets;
static int sockfd;
static suseconds_t latest_time_elapsed;
static suseconds_t latest_min_time;

static void sendGET(int packet_number);
static int recvDATA();
static void set_up_socket();
static void initialize_variables(in_addr_t _ip, int _port, FILE *_file_ptr, int _bytes);
static int write_to_file();
static void send_timeout_requests();
static void send_new_requests();
static void recv_packets();

void transport(in_addr_t _ip, int _port, FILE *_file_ptr, int _bytes)
{
    initialize_variables(_ip, _port, _file_ptr, _bytes);
    set_up_socket();
    while (write_to_file() != DONE)
    {
        send_timeout_requests();
        send_new_requests();
        recv_packets();
    }
}

static void initialize_variables(in_addr_t _ip, int _port, FILE *_file_ptr, int _bytes)
{
    ip = _ip;
    port = _port;
    file_ptr = _file_ptr;
    bytes = _bytes;
    downloaded_packets = 0;
    full_packets = bytes / MAX_PACKET;
    last_packet_size = bytes - (full_packets * MAX_PACKET);
    if (last_packet_size == 0)
    {
        last_packet_size = MAX_PACKET;
        all_packets = full_packets;
    }
    else
        all_packets = full_packets + 1;
    last_packet_window_index = NONE;
    next_to_save = 0;
    next_to_get = 0;
    for (int i = 0; i < WINDOW_SIZE; i++)
        window_status[i] = EMPTY;
}

static void set_up_socket()
{
    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons((short)port);
    server_address.sin_addr.s_addr = ip;
    Connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address));
}

static void sendGET(int packet_number)
{
    int start = packet_number * MAX_PACKET;
    int length;
    char packetdata[REQUEST_STRING_SIZE] = {0};
    if (packet_number == all_packets - 1)
        length = last_packet_size;
    else
        length = MAX_PACKET;
    sprintf(packetdata, "GET %d %d\n", start, length);
    Send(sockfd, packetdata, strlen(packetdata), 0);
}

static int recvDATA()
{
    int start, length;
    int packet_number, window_index;
    char packetdata[REQUEST_STRING_SIZE + MAX_PACKET];
    char *next_number, *filedata;
    if (Recv(sockfd, packetdata, REQUEST_STRING_SIZE + MAX_PACKET, MSG_DONTWAIT) == -1)
        return NO_PACKETS; // Recvfrom returned so packet_len == -1 implies errno == EWOULDBLOCK
    /* No need to check if the ip and port are correct, because we used connect() and recv() */
    start = strtol(packetdata + 5, &next_number, 10);
    length = strtol(next_number, &filedata, 10);
    filedata++;
    packet_number = start / MAX_PACKET;
    window_index = packet_number % WINDOW_SIZE;
    if ((packet_number < next_to_save) || window_status[window_index] == TO_SAVE)
        return DUPLICATE;
    memcpy(&window[window_index][0], filedata, length);
    return window_index;
}

static int write_to_file()
{
    for (int i = next_to_save % WINDOW_SIZE, j = next_to_save; i != (j - 1 + WINDOW_SIZE) % WINDOW_SIZE; i = (i + 1) % WINDOW_SIZE)
    {
        if (window_status[i] != TO_SAVE)
            return NOT_DONE;
        if (i == last_packet_window_index)
        {
            Fwrite(&window[i][0], 1, last_packet_size, file_ptr);
            window_status[i] = EMPTY;
            next_to_save++;
            return DONE;
        }
        else
        {
            Fwrite(&window[i][0], 1, MAX_PACKET, file_ptr);
            window_status[i] = EMPTY;
            next_to_save++;
        }
    }
    return NOT_DONE;
}

static void send_timeout_requests()
{
    latest_min_time = 1000000;
    for (int i = 0; i < WINDOW_SIZE; i++)
    {
        if (window_status[i] == REQUESTED)
        {
            window_time[i] -= latest_time_elapsed;
            if (window_time[i] <= 0)
            {
                sendGET(window_packet_number[i]);
                window_time[i] = DEFAULT_TIMER;
            }
            if (latest_min_time > window_time[i])
                latest_min_time = window_time[i];
        }
    }
}

static void send_new_requests()
{
    while ((next_to_get - next_to_save < WINDOW_SIZE) && (next_to_get < all_packets))
    {
        int window_index = next_to_get % WINDOW_SIZE;
        sendGET(window_index);
        window_packet_number[window_index] = next_to_get;
        window_status[window_index] = REQUESTED;
        window_time[window_index] = DEFAULT_TIMER;
        next_to_get++;
    }
}

static void recv_packets()
{
    int ready;
    struct timeval tv = {.tv_sec = 0, .tv_usec = latest_min_time};
    fd_set descriptors;
    FD_ZERO(&descriptors);
    FD_SET(sockfd, &descriptors);
    ready = Select(sockfd + 1, &descriptors, NULL, NULL, &tv);
    latest_time_elapsed = latest_min_time - tv.tv_usec;
    if (ready > 0)
        for (;;)
        {
            int window_index = recvDATA();
            if (window_index == NO_PACKETS)
                break;
            else if (window_index == DUPLICATE)
                continue;
            else
            {
                window_status[window_index] = TO_SAVE;
                downloaded_packets++;
                printf("Done:%.3f%%\n", (float)(downloaded_packets)*100 / all_packets);
                if (window_packet_number[window_index] == all_packets - 1)
                    last_packet_window_index = window_index;
            }
        }
}