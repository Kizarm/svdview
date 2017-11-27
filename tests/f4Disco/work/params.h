#ifndef PARAMS_H
#define PARAMS_H
static const double   AudioSampleRate = 44100.0;
static const unsigned AudioMidiDelay  = 44; // ~ AudioSampleRate / 1000
/// Počet generátorů.
static const unsigned int  maxGens  = 16;
/// Kladné maximum vzorku.
static const int           maxValue =  8191;
/// Záporné maximum vzorku.
static const int           minValue = -8192;
///
static const unsigned int  maxAmplt = (1U<<25);
#endif // PARAMS_H
