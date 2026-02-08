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
  int verbose = 0;

  while ((ch = getopt(argc, argv, "h:n:p:s:v")) != -1) { // It was missing -s
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
    case 'v':
      verbose = 1;
      break;
    default:
      fprintf(stderr,
              "usage: ping [-h host] [-n #pings] [-p port] [-s size] [-v]\n");
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
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  if (verbose)
    printf("Got addrinfo\n");

  int sockfd = -1;

  for (res_ptr = result; res_ptr != NULL; res_ptr = res_ptr->ai_next) {
    sockfd =
        socket(res_ptr->ai_family, res_ptr->ai_socktype, res_ptr->ai_protocol);

    if (sockfd != -1)
      break;
  }

  if (verbose)
    printf("Exited sock loop with sockfd of %d\n", sockfd);

  if (res_ptr == NULL) {
    fprintf(stderr, "Failed to create a socket to ping.\n");
    exit(1);
  }

  // Socket is set up, but not connected because udp. sendto() sends without
  // connection

  // Set up the datagram here
  unsigned char *dgram = malloc(arraysize * sizeof(char)); // TODO: Error check
  memset(dgram, 200, arraysize);

  unsigned char *res_dgram =
      calloc(arraysize, sizeof(char)); // TODO: Error Check:

  if (verbose)
    printf("Allocated dgram and res_dgram\n");

  double total_comm_time = 0;

  for (int p = 0; p < nping; p++) {
    if (verbose)
      printf("Trying loop %d\n", p);

    double stime = get_wctime();
    if (sendto(sockfd, dgram, arraysize, 0, res_ptr->ai_addr,
               res_ptr->ai_addrlen) != arraysize) {
      perror("Failed to send to socket. Skipping...");
      continue;
    }

    if (verbose) {
      printf("Sucessfully sent packet:\n");
      for (int j = 0; j < arraysize; j++)
        printf("%d, ", dgram[j]);
      printf("\n");
    }

    if (recvfrom(sockfd, res_dgram, arraysize, 0, res_ptr->ai_addr,
                 &res_ptr->ai_addrlen) != arraysize) {
      printf("Failed to receive: message length mismatch. Skipping...\n");
      continue;
    }

    if (verbose) {
      printf("Sucessfully received packet:\n");
      for (int j = 0; j < arraysize; j++)
        printf("%d, ", res_dgram[j]);
      printf("\n");
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

    if (verbose)
      printf("Validated\n");

    double comm_time = get_wctime() - stime;
    total_comm_time += comm_time;

    printf("\nSuccess in %f seconds\n", comm_time);
  }

  printf("Sent %d packets of %d bytes in %f ms (%f ms / good packet)\n", nping,
         arraysize, total_comm_time,
         total_comm_time / arraysize); // TODO: only count REAL packets

  printf("nping: %d arraysize: %d errors: %d ponghost: %s pongport: %s\n",
         nping, arraysize, errors, ponghost, pongport);

  free(dgram);
  free(res_dgram);
  freeaddrinfo(result);

  return 0;
}
