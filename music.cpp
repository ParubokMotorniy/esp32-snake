#include "music.h"

#include "Audio.h"
#include "FS.h"
#include "SD_MMC.h"

#define SD_MMC_CMD 38
#define SD_MMC_CLK 39
#define SD_MMC_D0 40
#define I2S_BCLK 42
#define I2S_DOUT 41
#define I2S_LRC 14

namespace Music {
Audio audio;

void initAudio(int startVolume) {
  SD_MMC.end();
  SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
  if (!SD_MMC.begin("/sdcard", true, true, SDMMC_FREQ_DEFAULT, 5)) {
    Serial.println("Card Mount Failed");
    return;
  }

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  setVolume(startVolume);
  audio.connecttoFS(SD_MMC, "/music/playlist.mp3");
  audio.setFileLoop(true);
}

void updateMusic() {

  audio.loop();
}

void setVolume(int newVolume) {
  if (newVolume < 0 || newVolume > 21) {
    return;
  }

  audio.setVolume(newVolume);
}
}
