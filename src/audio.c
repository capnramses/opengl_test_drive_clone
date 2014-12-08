#include "audio.h"
#include <stdio.h>
#include <assert.h>

irrklang::ISoundEngine* audio;
irrklang::ISound* engine_snd, *crash_snd;

bool init_audio () {
	printf ("init audio...\n");
	audio = irrklang::createIrrKlangDevice();
	if (!audio) {
		fprintf (stderr, "ERROR: could not start audio engine\n");
		return false;
	}
	// 3rd or 4th parms must be true to 'track' the sound for later mods
	engine_snd = audio->play2D("audio/engine_large.ogg", true, false, true);
	if (!engine_snd) {
		fprintf (stderr, "ERROR creating engine sound\n");
		return false;
	}
	engine_snd->setPlaybackSpeed (0.0f);
	engine_snd->setVolume (0.25f);
	audio->play2D("audio/pink.ogg", true, false, false);

	return true;
}

bool play_crash_snd () {
	// only play if not already playing
	if (audio->isCurrentlyPlaying ("audio/crash.ogg")) {
		return true;
	}
	crash_snd = audio->play2D("audio/crash.ogg", false, false, true);
	if (!crash_snd) {
		fprintf (stderr, "ERROR: playing crash sound\n");
		return false;
	}
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
