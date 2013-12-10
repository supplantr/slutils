#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libmpd-1.0/libmpd/libmpd.h>

#define MPD_BUF_LEN 96

#define FORMAT      "%s %s %s %s"
#define INTERVAL    3
#define MPD_HOST    "localhost"
#define MPD_PORT    6600
#define MPD_PASSWD  NULL
#define MPD_TIMEOUT 2

struct mpd_data {
    char album[MPD_BUF_LEN];
    char artist[MPD_BUF_LEN];
    char title[MPD_BUF_LEN];
    MpdState state;
};

int mpd_conn = 0;
char *format = FORMAT;
char *passwd = MPD_PASSWD;

void set_mpd_data(MpdObj *mpd, ChangedStatusType st, struct mpd_data *data)
{
    if (st&MPD_CST_STATE || st&MPD_CST_SONGID) {

        mpd_Song *song = mpd_playlist_get_current_song(mpd);
        data->state = mpd_player_get_state(mpd);

        if (song) {
            if (song->album)
                mpd_song_markup(data->album, MPD_BUF_LEN, "%album%", song);
            else
                strcpy(data->album, "");
            if (song->artist)
                mpd_song_markup(data->artist, MPD_BUF_LEN, "%artist%", song);
            else
                strcpy(data->artist, "");
            if (song->title) {
                mpd_song_markup(data->title, MPD_BUF_LEN, "%title%", song);
            } else if (song->name) {
                mpd_song_markup(data->title, MPD_BUF_LEN, "%name%", song);
            } else if (song->file) {
                mpd_song_markup(data->title, MPD_BUF_LEN, "%file%", song);
            } else
                strcpy(data->title, "");
        }
    }
}

void mpd_conn_changed(MpdObj *mpd, int connect, int *mpd_conn)
{
    if (!connect) {
        mpd_disconnect(mpd);
    }
    *mpd_conn = connect;
}

void put_info(MpdObj *mpd, struct mpd_data *data)
{
    if (!mpd_conn) {
        freopen("/dev/null", "w", stderr);
        if (passwd)
            mpd_send_password(mpd);
        mpd_connect(mpd);
    }

    mpd_status_update(mpd);

    if (!mpd_conn) {
        printf(format, "Not connected", "", "", "");
    } else if (data->state == MPD_PLAYER_PAUSE) {
        printf(format, "Paused", data->title, data->artist, data->album);
    } else if (data->state == MPD_PLAYER_PLAY) {
        printf(format, "Playing", data->title, data->artist, data->album);
    } else if (data->state == MPD_PLAYER_STOP) {
        printf(format, "Stopped", "", "", "");
    }

    printf("\n");
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    bool snoop = false;
    int interval = INTERVAL;
    char *host = MPD_HOST;
    int port = MPD_PORT;
    int timeout = MPD_TIMEOUT;

    char opt;
    while ((opt = getopt(argc, argv, "hsf:i:o:p:w:t:")) != -1) {
        switch (opt) {
            case 'h':
                printf("music [-h|-s|-f FORMAT|-i INTERVAL|-o HOST|-p PORT|-w PASSWORD|-t TIMEOUT]\n");
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
            case 'o':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'w':
                passwd = optarg;
                break;
            case 't':
                timeout = atoi(optarg);
                break;
        }
    }

    MpdObj *mpd = mpd_new(host, port, passwd);
    struct mpd_data data;
    mpd_signal_connect_status_changed(mpd, (StatusChangedCallback)set_mpd_data, &data);
    mpd_signal_connect_connection_changed(mpd, (ConnectionChangedCallback)mpd_conn_changed, &mpd_conn);
    mpd_set_connection_timeout(mpd, timeout);

    if (snoop)
        while (1) {
            put_info(mpd, &data);
            sleep(interval);
        }
    else
        put_info(mpd, &data);
}
