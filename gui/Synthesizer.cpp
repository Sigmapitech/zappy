#include "Synthesizer.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

bool Synthesizer::Sample::
  modify(double pitchMult, double speedMult, double volumeMult)
{
  if (pitchMult <= 0 || speedMult <= 0 || volumeMult <= 0)
    return false;

  // Speed modifier
  if (speedMult != 1.0) {
    std::vector<int16_t> newData;
    Uint32 newLength = (1 / speedMult) * this->length;

    for (int i = 0; i < (int)newLength; i++)
      newData.push_back(this->data[(i * this->length) / (int)newLength]);
    this->data = newData;
  }

  // TODO: Pitch modifer
  (void)pitchMult;

  // TODO: Volume modifer
  (void)volumeMult;

  return true;
}

bool Synthesizer::init(const std::string &sampleDir)
{
  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    std::cerr << "Synthesizer: SDL_Init error: " << SDL_GetError() << "\n";
    return false;
  }

  SDL_AudioSpec desiredSpec = {};
  desiredSpec.freq = 44100;
  desiredSpec.format = AUDIO_S16SYS;
  desiredSpec.channels = 1;
  desiredSpec.samples = 1024;
  desiredSpec.callback = audioCallback;
  desiredSpec.userdata = this;

  _audioDevice = SDL_OpenAudioDevice(
    nullptr, 0, &desiredSpec, &_obtainedSpec, 0);
  if (!_audioDevice) {
    std::cerr
      << "Synthesizer: SDL_OpenAudioDevice error: " << SDL_GetError() << "\n";
    return false;
  }

  for (int i = 0; i < NB_SAMPLES; i++) {
    std::string filename = sampleDir + "/" + std::to_string(i) + ".wav";
    SDL_AudioSpec spec;
    Uint8 *buf = nullptr;
    Uint32 len = 0;

    if (!std::filesystem::exists(filename)) {
      std::cerr << "Synthesizer: Missing sample file no." << i << "\n";
      return false;
    } else if (SDL_LoadWAV(filename.c_str(), &spec, &buf, &len)) {
      if (spec.format != AUDIO_S16SYS || spec.channels != 1
          || spec.freq != 44100) {
        std::cerr << "Synthesizer: Unsupported format: " << filename << "\n";
        SDL_FreeWAV(buf);
        continue;
      }
      Sample s;
      s.length = len / sizeof(int16_t);
      s.data.assign((int16_t *)buf, (int16_t *)(buf + len));
      SDL_FreeWAV(buf);
      _samples[i] = std::move(s);
    }
  }

  SDL_PauseAudioDevice(_audioDevice, 0);
  _initialized = true;
  return true;
}

void Synthesizer::shutdown()
{
  if (_audioDevice) {
    SDL_CloseAudioDevice(_audioDevice);
    _audioDevice = 0;
  }
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
  _initialized = false;
}

void Synthesizer::speak(const std::string &message)
{
  std::lock_guard<std::mutex> lock(_voicesMutex);
  Voice voice;

  for (unsigned char c: message) {
    auto it = _samples.find(c);
    if (it != _samples.end()) {
      // modify sample locally if needed
      voice.sequence.push_back(&it->second);
    }
  }
  if (!voice.sequence.empty())
    _activeVoices.push_back(std::move(voice));
}

void Synthesizer::audioCallback(void *userdata, Uint8 *stream, int len)
{
  auto *player = static_cast<Synthesizer *>(userdata);
  player->mixAudio((int16_t *)stream, len / sizeof(int16_t));
}

void Synthesizer::mixAudio(int16_t *stream, int len)
{
  std::fill(stream, stream + len, 0);
  std::lock_guard<std::mutex> lock(_voicesMutex);

  for (auto it = _activeVoices.begin(); it != _activeVoices.end();) {
    Voice &voice = *it;
    int i = 0;

    while (i < len && voice.currentSampleIdx < voice.sequence.size()) {
      const Sample *sample = voice.sequence[voice.currentSampleIdx];
      Uint32 remaining = sample->length - voice.sampleOffset;
      Uint32 toCopy = std::min((Uint32)(len - i), remaining);

      for (Uint32 j = 0; j < toCopy; j++) {
        int sampleVal = stream[i + j] + sample->data[voice.sampleOffset + j];
        stream[i + j] = std::clamp(sampleVal, -32768, 32767);
      }
      voice.sampleOffset += toCopy;
      i += toCopy;
      if (voice.sampleOffset >= sample->length) {
        voice.sampleOffset = 0;
        voice.currentSampleIdx++;
      }
    }

    if (voice.currentSampleIdx >= voice.sequence.size())
      it = _activeVoices.erase(it);
    else
      it++;
  }
}
