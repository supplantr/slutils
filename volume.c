#include <signal.h>
#include <stdbool.h>
#include <alsa/asoundlib.h>

#define FORMAT   "%s %i"
#define INTERVAL 3
#define SNDCARD  "default"
#define MIXER    "Master"

struct alsa_data {
	snd_mixer_t *handle;
	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *sid;
	long min, max, vol;
	int mute;
};

char *format = FORMAT;
char *sndcard = SNDCARD;
char *mixer = MIXER;
bool force = false;

void alsa_event(snd_mixer_elem_t *elem, unsigned int mask)
{
	if (mask&0x1) {
		struct alsa_data *data = (struct alsa_data *)snd_mixer_elem_get_callback_private(elem);

		if (snd_mixer_selem_get_playback_volume(elem, 0, &(data->vol)) != 0) {
			fprintf(stderr, "error: snd_mixer_selem_get_playback_volume failed\n");
			exit(EXIT_FAILURE);
		}

		if (snd_mixer_selem_get_playback_switch(elem, 0, &(data->mute)) != 0) {
			fprintf(stderr, "error: snd_mixer_selem_get_playback_switch failed\n");
			exit(EXIT_FAILURE);
		}

	}
}

void init_alsa_data(struct alsa_data *alsa)
{
	if (snd_mixer_open(&(alsa->handle), 0) != 0) {
		fprintf(stderr, "error: snd_mixer_open failed\n");
		exit(EXIT_FAILURE);
	}

	if (snd_mixer_attach(alsa->handle, sndcard) != 0) {
		fprintf(stderr, "error: snd_mixer_attach failed\n");
		exit(EXIT_FAILURE);
	}

	if (snd_mixer_selem_register(alsa->handle, NULL, NULL) != 0) {
		fprintf(stderr, "error: snd_mixer_selem_register failed\n");
		exit(EXIT_FAILURE);
	}

	if (snd_mixer_load(alsa->handle) != 0) {
		fprintf(stderr, "error: snd_mixer_load failed\n");
		exit(EXIT_FAILURE);
	}

	snd_mixer_selem_id_malloc(&(alsa->sid));
	snd_mixer_selem_id_set_index(alsa->sid, 0);
	snd_mixer_selem_id_set_name(alsa->sid, mixer);
	alsa->elem = snd_mixer_find_selem(alsa->handle, alsa->sid);

	if (alsa->elem == NULL) {
		fprintf(stderr, "error: snd_mixer_find_selem failed\n");
		exit(EXIT_FAILURE);
	}

	snd_mixer_elem_set_callback(alsa->elem, (snd_mixer_elem_callback_t)alsa_event);
	snd_mixer_elem_set_callback_private(alsa->elem, alsa);

	if (snd_mixer_selem_get_playback_volume_range(alsa->elem, &(alsa->min), &(alsa->max)) != 0) {
		fprintf(stderr, "error: couldn't get volume range\n");
		exit(EXIT_FAILURE);
	}

	alsa_event(alsa->elem, 1);
}

char* to_string(int i)
{
	char *s;
	(i) ? (s = "On") : (s = "Off");
	return s;
}

void put_info(struct alsa_data *alsa)
{
	snd_mixer_handle_events(alsa->handle);
	printf(format, to_string(alsa->mute), 100*(alsa->vol - alsa->min)/(alsa->max - alsa->min));
	printf("\n");
	fflush(stdout);
}

void sig_handle(int sig)
{
	if (sig == SIGUSR1)
		force = true;
}

int main(int argc, char *argv[])
{
	struct sigaction act;
	act.sa_handler = &sig_handle;
	sigaction(SIGUSR1, &act, 0);

	bool snoop = false;
	int interval = INTERVAL;

	char opt;
	while ((opt = getopt(argc, argv, "hsf:i:c:m:")) != -1) {
		switch (opt) {
			case 'h':
				printf("volume [-h|-s|-f FORMAT|-i INTERVAL|-c CARD|-m MIXER]\n");
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
			case 'c':
				sndcard = optarg;
				break;
			case 'm':
				mixer = optarg;
				break;
		}
	}

	struct alsa_data alsa;
	init_alsa_data(&alsa);

	if (snoop)
		while (1) {
			if (force) {
				force = false;
				continue;
			}
			put_info(&alsa);
			sleep(interval);
		}
	else
		put_info(&alsa);
}
