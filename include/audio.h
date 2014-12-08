#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <irrklang/irrKlang.h>

bool init_audio ();
bool play_crash_snd ();
void set_engine_speed (float fac);
void stop_driving_snds ();
bool free_audio ();

//export ISoundEngine* audio;

#endif
