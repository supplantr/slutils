#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FORMAT   "%i"
#define INTERVAL 3
#define PATH     "/sys/devices/platform/coretemp.0/temp1_input"

char *format = FORMAT;
char *path = PATH;

int put_info(void)
{
	FILE *fp;
	int temp;

	if ((fp = fopen(path, "r")) == NULL) {
		fprintf(stderr, "error: can't open %s\n", path);
		exit(EXIT_FAILURE);
	}

	fscanf(fp, "%i", &temp);
	fclose(fp);
	printf(format, temp);
	printf("\n");
	fflush(stdout);
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	bool snoop = false;
	int interval = INTERVAL;

	char opt;
	while ((opt = getopt(argc, argv, "hsi:p:")) != -1) {
		switch (opt) {
			case 'h':
				printf("temp [-h|-s|-i INTERVAL|-p PATH]\n");
				exit(EXIT_SUCCESS);
				break;
			case 's':
				snoop = true;
				break;
			case 'i':
				interval = atoi(optarg);
				break;
			case 'p':
				path = optarg;
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
