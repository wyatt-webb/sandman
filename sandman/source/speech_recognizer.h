#pragma once

#include <sphinxbase/ad.h>
#include <sphinxbase/cont_ad.h>
#include <pocketsphinx.h>

#include <stdint.h>

#include "timer.h"

// Types
//

// Hides the details of recognizing speech.
//
class SpeechRecognizer
{
	public:

		// Initialize the recognizer.
		//
		// p_CaptureDeviceName:								The name of the audio capture device.
		// p_SampleRate:										The audio capture sample rate.
		// p_HMMFileName:										File name of the HMM the recognizer will use.
		// p_LanguageModelFileName:						File name of the language model the recognizer will use.
		// p_DictionaryFileName:							File name of the dictionary the recognizer will use.
		// p_LogFileName:										File name of the log for recognizer output.
		// p_UtteranceTrailingSilenceThresholdSec:	How long to wait with no new voice to end an 
		// 														utterance in seconds.
		//
		// returns:		True for success, false otherwise.
		//
		bool Initialize(char const* p_CaptureDeviceName, unsigned int p_SampleRate, 
			char const* p_HMMFileName,	char const* p_LanguageModelFileName, 
			char const* p_DictionaryFileName, char const* p_LogFileName,
			float p_UtteranceTrailingSilenceThresholdSec);

		// Uninitialize the recognizer.
		//
		void Uninitialize();

		// Process audio input in an attempt to recognize speech.
		//
		// p_RecognizedSpeech:	(Output) The speech recognized if any, or null if not.
		//
		// returns:				True for success, false if an error occurred.
		//
		bool Process(char const*& p_RecognizedSpeech);

	private:

		// Allows recording from an audio input device.
		ad_rec_t* m_AudioRecorder;

		// Detects voice activity vs. silence in the audio input.
		cont_ad_t* m_VoiceActivityDetector;

		// Decodes the audio input.
		ps_decoder_t* m_SpeechDecoder;

		// Track where an utterance is in progress.
		bool m_InUtterance;

		// A record of when the utterance began.
		Time m_UtteranceStartTime;

		// A counter tracking when we last saw voice activity.
		int m_LastVoiceSampleCount;
};
