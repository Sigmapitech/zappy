#pragma once

#include <SDL2/SDL.h>

#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#define NB_SAMPLES 256

class Synthesizer {
private:
  struct Sample {
    std::vector<int16_t> data;
    Uint32 length;

    bool modify(double pitchMult, double speedMult, double volumeMult);
  };

  struct Voice {
    std::vector<const Sample *> sequence;
    size_t currentSampleIdx = 0;
    Uint32 sampleOffset = 0;
  };

  static void audioCallback(void *userdata, Uint8 *stream, int len);
  void mixAudio(int16_t *stream, int len);

  std::unordered_map<uint8_t, Sample> _samples;
  std::vector<Voice> _activeVoices;
  std::mutex _voicesMutex;

  SDL_AudioDeviceID _audioDevice = 0;
  SDL_AudioSpec _obtainedSpec;
  bool _initialized = false;

public:
  Synthesizer() = default;

  ~Synthesizer()
  {
    shutdown();
  }

  bool init(const std::string &sampleDir);
  void speak(const std::string &message);
  void shutdown();
};
