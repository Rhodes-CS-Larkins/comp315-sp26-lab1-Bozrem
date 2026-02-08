/*
 * ping.c - UDP ping/pong client code
 *          author: Remy Bozung
 */
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

int main(int argc, char **argv) {
  int ch, errors = 0;
  int nping = 1;                        // default packet count
  char *ponghost = strdup("localhost"); // default host
  char *pongport = strdup(PORTNO);      // default port
  int arraysize = 100;                  // default packet size

  while ((ch = getopt(argc, argv, "h:n:p:s:")) != -1) { // It was missing -s
    switch (ch) {
    case 'h':
      ponghost = strdup(optarg);
      break;
    case 'n':
      nping = atoi(optarg);
      break;
    case 'p':
      pongport = strdup(optarg);
      break;
    case 's':
      arraysize = atoi(optarg);
      break;
    default:
      fprintf(stderr,
              "usage: ping [-h host] [-n #pings] [-p port] [-s size]\n");
    }
  }

  struct addrinfo hints, *result, *res_ptr;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;      // IPv4
  hints.ai_socktype = SOCK_DGRAM; // UDP
  hints.ai_protocol = 0;          // Any protocol
  hints.ai_flags = 0;

  int s = getaddrinfo(ponghost, pongport, &hints, &result);
  if (s != 0) {
    perror("Failed getaddrinfo");
    exit(1);
  }

  int sockfd = -1;

  for (res_ptr = result; res_ptr != NULL; res_ptr = res_ptr->ai_next) {
    sockfd =
        socket(res_ptr->ai_family, res_ptr->ai_socktype, res_ptr->ai_protocol);

    if (sockfd != -1)
      break;
  }

  if (res_ptr == NULL) {
    fprintf(stderr, "Failed to create a socket to ping.\n");
    exit(1);
  }

  // Socket is set up, but not connected because udp. sendto() sends without
  // connection

  // Set up the datagram here
  char *dgram = malloc(arraysize * sizeof(char)); // TODO: Error check
  memset(dgram, 200, arraysize);

  char *res_dgram = calloc(arraysize, sizeof(char)); // TODO: Error Check:

  double total_comm_time = 0;

  for (int p = 0; p < nping; p++) {
    double stime = get_wctime();
    if (sendto(sockfd, dgram, arraysize, 0, res_ptr->ai_addr,
               res_ptr->ai_addrlen) != arraysize) {
      perror("Failed to send to socket. Skipping...");
      continue;
    }

    if (recvfrom(sockfd, res_dgram, arraysize, 0, res_ptr->ai_addr,
                 &res_ptr->ai_addrlen) != arraysize) {
      printf("Failed to receive: message length mismatch. Skipping...\n");
      continue;
    }

    int valid = 1;
    // Validate res
    for (int i = 0; i < arraysize; i++) {
      if ((int)res_dgram[i] != 201) {
        printf("Failed to validate res at index %d. Expected 201, got %d.\n", i,
               res_dgram[i]);
        valid = 0;
        break;
      }
    }

    if (!valid)
      break;

    double comm_time = get_wctime() - stime;
    total_comm_time += comm_time;

    printf("\nSuccess in %f seconds\n", comm_time);
  }

  // TODO: Stats printing

  printf("nping: %d arraysize: %d errors: %d ponghost: %s pongport: %s\n",
         nping, arraysize, errors, ponghost, pongport);

  free(dgram);
  free(res_dgram);
  freeaddrinfo(result);

  return 0;
}
