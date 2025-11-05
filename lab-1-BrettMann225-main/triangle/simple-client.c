#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>

jack_port_t *input_port_left;
jack_port_t *output_port_left;
jack_port_t *input_port_right;
jack_port_t *output_port_right;
jack_client_t *client;

int indexleft = 0;
int indexright = 0;

int process (jack_nframes_t nframes, void *arg) {
  jack_default_audio_sample_t *outleft, *outright;

  outleft  = jack_port_get_buffer (output_port_left, nframes);
  outright = jack_port_get_buffer (output_port_right, nframes);


for (int i = 0; i < nframes; i++) {
 float phaseL = (float)indexleft / (float)(48); //48 samples per cycle, 24 samples for rising, 24 for falling. goes from 0-1 every 48 samples

 if (indexleft < 24) {
	outleft[i] = 2*phaseL - 0.5f;
 }  else {
	outleft[i] = (1-2*phaseL)+0.5f;
	}
 indexleft = (indexleft+1)%48;
}

for (int i = 0; i < nframes; i++) {
  float phaseR = (float)indexright / (float)(24);

  if (indexright < 12) {
        outright[i] = 2*phaseR - 0.5f;
 }  else {
        outright[i] = (1-2*phaseR)+0.5f;
        }
  indexright = (indexright+1)%24;
}

  // define a function that writes nframes of samples
  // to the left and right buffer according to
  // the specifications of the lab

  return 0;
}

void jack_shutdown (void *arg) {
        exit (1);
}

int main (int argc, char *argv[]) {
  const char **ports;
  const char *client_name = "simple";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t status;

  client = jack_client_open (client_name, options, &status, server_name);
  if (client == NULL) {
    fprintf (stderr, "jack_client_open() failed, status = 0x%2.0x\n", status);
    if (status & JackServerFailed) {
      fprintf (stderr, "Unable to connect to JACK server\n");
    }
    exit (1);
  }
  if (status & JackServerStarted) {
    fprintf (stderr, "JACK server started\n");
  }
  if (status & JackNameNotUnique) {
    client_name = jack_get_client_name(client);
    fprintf (stderr, "unique name `%s' assigned\n", client_name);
  }

  jack_set_process_callback (client, process, 0);
  jack_on_shutdown (client, jack_shutdown, 0);

  printf ("engine sample rate: %" PRIu32 "\n", jack_get_sample_rate (client));

  input_port_left = jack_port_register (client, "input_left",
                                   JACK_DEFAULT_AUDIO_TYPE,
                                   JackPortIsInput, 0);
  output_port_left = jack_port_register (client, "output_left",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput, 0);
  input_port_right = jack_port_register (client, "input_right",
                                   JACK_DEFAULT_AUDIO_TYPE,
                                   JackPortIsInput, 0);
  output_port_right = jack_port_register (client, "output_right",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput, 0);

  if ((input_port_left == NULL) || (output_port_left == NULL) ||
      (input_port_right == NULL) || (output_port_right == NULL)) {
    fprintf(stderr, "no more JACK ports available\n");
    exit (1);
  }

  if (jack_activate (client)) {
    fprintf (stderr, "cannot activate client");
    exit (1);
  }

  while (1)
    sleep(10);

  jack_client_close (client);
  exit (0);
}
