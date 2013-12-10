#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common.h"

#define INTERVAL  3
#define INTERFACE "wlp3s0"

char *interface = INTERFACE;

int put_info(void)
{
    FILE *fp;
    char buf[BUF_LEN];

    if ((fp = fopen("/proc/net/route", "r")) == NULL) {
        fprintf(stderr, "error: can't open /proc/net/route\n");
        exit(EXIT_FAILURE);
    }

    while (fgets(buf, BUF_LEN, fp) != NULL) {
        if (strncmp(interface, buf, strlen(interface)) == 0) {
            fclose(fp);
            printf("Online\n");
            fflush(stdout);
            return EXIT_SUCCESS;
        }
    }

    fclose(fp);
    printf("Offline\n");
    fflush(stdout);
    return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
    bool snoop = false;
    int interval = INTERVAL;

    char opt;
    while ((opt = getopt(argc, argv, "hsi:w:")) != -1) {
        switch (opt) {
            case 'h':
                printf("online [-h|-s|-i INTERVAL|-w INTERFACE]\n");
                exit(EXIT_SUCCESS);
                break;
            case 's':
                snoop = true;
                break;
            case 'i':
                interval = atoi(optarg);
                break;
            case 'w':
                interface = optarg;
                break;
        }
    }

    if (snoop)
        while (1) {
            put_info();
            sleep(interval);
        }
    else
        return put_info();
}
