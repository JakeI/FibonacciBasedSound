/*
* Copyright (c) 2014, Benny Bobaganoosh
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
*   this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED ANDON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

/*

Dependancies:

VC++ Directories > include C:\A\C_Programme\SDL\SDL2-devel-2.0.4-VC\SDL2-2.0.4\include

VC++ Directories > Libs > C:\A\C_Programme\SDL\SDL2-devel-2.0.4-VC\SDL2-2.0.4\lib\x86

Linker > Additional Dependancies > input > SDL2.lib;SDL2main.lib;

Linker > System > Subsystem > Windows

Copy "C:\A\C_Programme\SDL\SDL2-devel-2.0.4-VC\SDL2-2.0.4\lib\x86\SDL2.dll" to "C:\A\C_Programme\VC++\SoundTest\Debug"

*/

#include <iostream>
#include <SDL/SDL.h>
#include <conio.h>

#define FILE_PATH "mySound.wav"

struct AudioData
{
	Uint8* pos;
	Uint32 length;
};

void MyAudioCallback(void* userdata, Uint8* stream, int streamLength)
{
	AudioData* audio = (AudioData*)userdata;

	if (audio->length == 0)
	{
		return;
	}

	Uint32 length = (Uint32)streamLength;
	length = (length > audio->length ? audio->length : length);

	SDL_memcpy(stream, audio->pos, length);

	audio->pos += length;
	audio->length -= length;
}

inline double squereWave(double t) {
	return 1 - (2 * abs((int)(floor(t / M_PI)) % 2));
}

#define M_2_PI	2*M_PI
inline double triangleWave(double t) {
	double x = abs(t) / (M_2_PI);
	double m = x - floor(x);
	return m < M_PI ? m - 1 : 1 - m + M_PI;
}

struct wave {
	int frequencieID;
	float freqFactor;
	float weight;
	void set(float FreqFactor, float Weight, int FrequencieID = 0) {
		freqFactor = FreqFactor;
		weight = Weight;
		frequencieID = FrequencieID;
	}
};

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_AUDIO);

	SDL_AudioSpec wavSpec;
	Uint8* wavStart;
	Uint32 wavLength;

	/*if (SDL_LoadWAV(FILE_PATH, &wavSpec, &wavStart, &wavLength) == NULL)
	{
		// TODO: Proper error handling
		std::cerr << "Error: " << FILE_PATH
			<< " could not be loaded as an audio file" << std::endl;
		_getch();
		return 1;
	}*/

	wavLength = 18466204/4;
	wavStart = new Uint8[2 * wavLength];

	wavSpec.channels = 1;
	wavSpec.format = AUDIO_U8;
	wavSpec.freq = 44100;
	wavSpec.padding = 0;
	wavSpec.samples = 2048; // half the lenght of the sample stream in the callback function
	wavSpec.silence = 0;
	wavSpec.size = 0;

	wave waves[5] = { 0 };
	int wavesLen = sizeof(waves)/sizeof(wave);
	waves[0].set(1.0f, 2.0f);
	waves[1].set(2.0f, 1.0f);
	waves[2].set(3.0f, 0.5f);
	waves[3].set(5.0f, 0.25f);
	waves[4].set(8.0f, 0.125f);

	double constant = 2 * M_PI / wavSpec.freq;
	int twoAgo = 1, oneAgo = 1;
	double frequencys[2] = { 500, 600 };
	int noteLength = 22050;
	for (unsigned int i = 0; i < wavLength; i++) {
		if (i % noteLength == 0) {
			int next = twoAgo + oneAgo;
			frequencys[0] = 400 + (50 * (next % 7));
			//frequencys[1] = 800 + (50 * (next % 7));
			float fractionLength = pow(2, (next % 2) - 2);
			noteLength = wavSpec.freq * fractionLength;
			twoAgo = oneAgo;
			oneAgo = next;
		}

		float sample = 0.0f, totalWeight = 0.0f;
		for(int j = 0; j < wavesLen; j++) {
			sample += waves[j].weight*sin(constant*i*frequencys[waves[j].frequencieID]*waves[j].freqFactor);
			totalWeight += waves[j].weight;
		}
		wavStart[i] = (Uint8)(6*sample/totalWeight + 6);

		//wavStart[i] = (Uint8)(2 * sin(constant*i*frequency) + sin(constant*i*frequency*2) + sin(constant*i*frequency * 4) + 3);
		/*wavStart[i] = (Uint8)(3 * sin(constant*i*(300 * sin(constant*i*2) + 500)));*/
	}

	AudioData audio;
	audio.pos = wavStart;
	audio.length = wavLength;

	wavSpec.callback = MyAudioCallback;
	wavSpec.userdata = &audio;

	std::cout << "wav Spec:" << std::endl
		<< "wavSpec.channels = " << (int)wavSpec.channels << std::endl
		<< "wavSpec.format = " << wavSpec.format << std::endl
		<< "wavSpec.freq = " << wavSpec.freq << std::endl
		<< "wavSpec.padding = " << wavSpec.padding << std::endl
		<< "wavSpec.samples = " << wavSpec.samples << std::endl
		<< "wavSpec.silence = " << (int)wavSpec.silence << std::endl
		<< "wavSpec.size = " << wavSpec.size << std::endl;
		//<< "wavSpec." << wavSpec. << endl

	std::cout << std::endl << "audio.lenth = " << audio.length << std::endl;
		

	SDL_AudioDeviceID device = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL,
		SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (device == 0)
	{
		// TODO: Proper error handling
		std::cerr << "Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_PauseAudioDevice(device, 0);

	while (audio.length > 0 && _getch() != 27)
	{
		SDL_Delay(100);
	}

	SDL_CloseAudioDevice(device);
	SDL_FreeWAV(wavStart);
	delete[] wavStart;
	SDL_Quit();
	return 42;
}