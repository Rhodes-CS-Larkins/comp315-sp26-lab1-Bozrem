/*
 * pong.c - UDP ping/pong server code
 *          author: Remy Bozung
 */
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "util.h"

#define PORTNO "1266"
#define BUF_SIZE 1024

int main(int argc, char **argv) {
  int ch;
  int nping = 1;                   // default packet count
  char *pongport = strdup(PORTNO); // default port
  int verbose = 0;

  while ((ch = getopt(argc, argv, "n:p:v")) != -1) {
    switch (ch) {
    case 'n':
      nping = atoi(optarg);
      break;
    case 'p':
      pongport = strdup(optarg);
      break;
    case 'v':
      verbose = 1;
      break;
    default:
      fprintf(stderr, "usage: pong [-n #pings] [-p port] [-v]\n");
    }
  }

  struct addrinfo hints, *result, *res_ptr;
  memset(&hints, 0, sizeof(hints));
  // A lot of this follows the man page example pretty closely
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE; // TODO: Figure out what this actually does

  int s = getaddrinfo(NULL, pongport, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  if (verbose)
    printf("Got addrinfo\n");

  int sockfd;

  for (res_ptr = result; res_ptr != NULL; res_ptr = res_ptr->ai_next) {
    sockfd =
        socket(res_ptr->ai_family, res_ptr->ai_socktype, res_ptr->ai_protocol);
    if (sockfd == -1)
      continue;

    if (bind(sockfd, res_ptr->ai_addr, res_ptr->ai_addrlen) == 0)
      break; // Successful socket binding
  }

  if (verbose)
    printf("Established socket with sockfd of %d\n", sockfd);

  freeaddrinfo(result);

  if (res_ptr == NULL) {
    fprintf(stderr, "Failed to bind to a socket.\n");
    exit(1);
  }

  char dgram_buf[BUF_SIZE]; // TODO: Does this need to be dynamic or can I just
                            // set it reasonably large?

  struct sockaddr_storage peer_addr;
  socklen_t peer_addr_len;

  for (int i = 0; i < nping; i++) {
    // Use recv to get the dgram
    peer_addr_len = sizeof(peer_addr);
    int packet_size = recvfrom(sockfd, dgram_buf, BUF_SIZE, 0,
                               (struct sockaddr *)&peer_addr, &peer_addr_len);
    if (packet_size == -1)
      continue;

    // Add 1 to all
    for (int i = 0; i < packet_size; i++)
      dgram_buf[i]++;

    // send back
    char host[NI_MAXHOST], service[NI_MAXSERV];

    int s = getnameinfo((struct sockaddr *)&peer_addr, peer_addr_len, host,
                        NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
    if (s == 0)
      printf("pong[%d]: received packet from %s\n", i, host);
    else
      fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));

    if (sendto(sockfd, dgram_buf, packet_size, 0, (struct sockaddr *)&peer_addr,
               peer_addr_len) != packet_size)
      fprintf(stderr, "Did not send correct packet size on return.\n");

    nping--;
  }

  printf("nping: %d pongport: %s\n", nping, pongport);

  return 0;
}
