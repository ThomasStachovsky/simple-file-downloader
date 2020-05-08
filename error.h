#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/ip_icmp.h>
#include <assert.h>
#include <sys/select.h>
#include <sys/time.h>

void Inet_pton(int __af, const char *__restrict__ __cp, void *__restrict__ __buf);
void Inet_ntop(int __af, const void *__restrict__ __cp, char *__restrict__ __buf, socklen_t __len);
int Socket(int __domain, int __type, int __protocol);
ssize_t Sendto(int __fd, const void *__buf, size_t __n, int __flags, const struct sockaddr *__addr, socklen_t __addr_len);
void Setsockopt(int __fd, int __level, int __optname, const void *__optval, socklen_t __optlen);
int Select(int __nfds, fd_set *__restrict__ __readfds, fd_set *__restrict__ __writefds, fd_set *__restrict__ __exceptfds, struct timeval *__restrict__ __timeout);
ssize_t Recvfrom(int __fd, void *__restrict__ __buf, size_t __n, int __flags, struct sockaddr *__restrict__ __addr, socklen_t *__restrict__ __addr_len);
void Close(int fd);
void Bind(int __fd, const struct sockaddr *__addr, socklen_t __len);
FILE *Fopen(const char *__restrict__ __filename, const char *__restrict__ __modes);
void Fclose(FILE *stream);
void Connect(int __fd, const struct sockaddr *__addr, socklen_t __len);
ssize_t Send(int __fd, const void *__buf, size_t __n, int __flags);
ssize_t Recv(int __fd, void *__buf, size_t __n, int __flags);
void Fwrite(const void *__restrict__ __ptr, size_t __size, size_t __n, FILE *__restrict__ __s);