#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FORMAT    "%s"
#define INTERVAL  3
#define INTERFACE "wlp3s0"

char *format = FORMAT;
char *interface = INTERFACE;

int put_info(void)
{
	FILE *fp;
	char path[64], state[4];

	snprintf(path, sizeof(path), "/sys/class/net/%s/operstate", interface);

	if ((fp = fopen(path, "r")) == NULL) {
		fprintf(stderr, "error: can't open %s\n", path);
		exit(EXIT_FAILURE);
	}

	fscanf(fp, "%s", state);
	fclose(fp);

	if (strncmp(state, "up", 2) == 0) {
		printf(format, "Online");
		printf("\n");
		fflush(stdout);
		return EXIT_SUCCESS;
	}

	printf(format, "Offline");
	printf("\n");
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
				printf("online [-h|-s|-f FORMAT|-i INTERVAL|-w INTERFACE]\n");
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

	if (snoop)
		while (1) {
			put_info();
			sleep(interval);
		}
	else
		return put_info();
}
