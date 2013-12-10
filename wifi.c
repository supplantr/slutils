#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/wireless.h>
#include "common.h"

#define OFFSET(X) ((6-X) > 0 ? (6-X) : 0)

#define FORMAT    "%i %s"
#define INTERVAL  3
#define INTERFACE "wlp3s0"

char *format = FORMAT;
char *interface = INTERFACE;
char name[IW_ESSID_MAX_SIZE + 1] = {0};

int put_info(int fd, struct iwreq *rqt)
{
    FILE *fp;
    char buf[BUF_LEN];
    int len = strlen(interface);
    int ss;

    if ((fp = fopen("/proc/net/wireless", "r")) == NULL) {
        fprintf(stderr, "error: can't open /proc/net/wireless\n");
        exit(EXIT_FAILURE);
    }

    while (fgets(buf, BUF_LEN, fp) != NULL) {
        if (strncmp(interface, buf + OFFSET(len), len) == 0) {
            fclose(fp);
            sscanf(buf + OFFSET(len) + len + 1, "%*d %d", &ss);

            rqt->u.essid.pointer = name;
            rqt->u.essid.length = IW_ESSID_MAX_SIZE + 1;
            if (ioctl(fd, SIOCGIWESSID, rqt) == -1) {
                perror("ioctl");
                exit(EXIT_FAILURE);
            }

            printf(format, 100*ss/70, name);
            printf("\n");
            fflush(stdout);
            return EXIT_SUCCESS;
        }
    }

    fclose(fp);
    printf("N/A\n");
    fflush(stdout);
    return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
    bool snoop = false;
    int interval = INTERVAL;

    char opt;
    while ((opt = getopt(argc, argv, "hsf:i:w:")) != -1) {
        switch (opt) {
            case 'h':
                printf("wifi [-h|-s|-f FORMAT|-i INTERVAL|-w INTERFACE]\n");
                exit(EXIT_SUCCESS);
                break;
            case 's':
                snoop = true;
                break;
            case 'f':
                format = optarg;
                break;
            case 'i':
                interval = atoi(optarg);
                break;
            case 'w':
                interface = optarg;
                break;
        }
    }

    struct iwreq request;
    int sock_fd;
    memset(&request, 0, sizeof(struct iwreq));
    sprintf(request.ifr_name, interface);

    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int exit_code;

    if (snoop)
        while (1) {
            put_info(sock_fd, &request);
            sleep(interval);
            name[0] = '\0';
        }
    else
        exit_code = put_info(sock_fd, &request);

    close(sock_fd);

    return exit_code;
}
