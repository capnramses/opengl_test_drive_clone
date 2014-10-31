#include "audio.h"
#include <stdio.h>
#include <assert.h>

irrklang::ISoundEngine* audio;
irrklang::ISound* engine_snd;

bool init_audio () {
	printf ("init audio...\n");
	audio = irrklang::createIrrKlangDevice();
	if (!audio) {
		fprintf (stderr, "ERROR: could not start audio engine\n");
		return false;
	}
	// 3rd or 4th parms must be true to 'track' the sound for later mods
	engine_snd = audio->play2D("audio/engine_small.ogg", true, false, true);
	if (!engine_snd) {
		fprintf (stderr, "ERROR creating engine sound\n");
		return false;
	}
	engine_snd->setPlaybackSpeed (0.0f);

	return true;
}

void set_engine_speed (float fac) {
	assert (engine_snd);
	engine_snd->setPlaybackSpeed (fabs(fac) / 5.0f);
}

bool free_audio () {
	audio->drop ();
	return true;
}
