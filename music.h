#ifndef MUSIC_H
#define MUSIC_H
namespace Music {
void initAudio(int startVolume);

//play music on
void updateMusic();

//between 0-21
void setVolume(int newVolume);
}

#endif
