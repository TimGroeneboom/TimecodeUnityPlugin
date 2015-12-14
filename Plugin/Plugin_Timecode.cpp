#include "AudioPluginUtil.h"

extern "C"
{
#include "xwax\timecoder.h"
}

namespace Timecode
{
	enum Param
	{
		P_Pitch,
		P_NUM
	};

	struct EffectData
	{
		float p[P_NUM];
		HistoryBuffer history[8];
		float pitch;
		int numchannels;
	};

	//
	bool shortBufferInited;
	struct timecoder_t timecoder;
	short *shortBuffer;

	int InternalRegisterEffectDefinition(UnityAudioEffectDefinition& definition)
	{
		int numparams = P_NUM;
		definition.paramdefs = new UnityAudioParameterDefinition[numparams];
		return numparams;
	}

	UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK CreateCallback(UnityAudioEffectState* state)
	{
		EffectData* data = new EffectData;
		memset(data, 0, sizeof(EffectData));
		InitParametersFromDefinitions(InternalRegisterEffectDefinition, data->p);
		state->effectdata = data;
		for (int i = 0; i < 8; i++)
			data->history[i].Init(state->samplerate * 2);

		// TODO : make format selectable with GUI
		timecoder_init(&timecoder, "traktor_b", 1.0, state->samplerate);

		return UNITY_AUDIODSP_OK;
	}

	UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ReleaseCallback(UnityAudioEffectState* state)
	{
		EffectData* data = state->GetEffectData<EffectData>();
		delete data;

		//
		timecoder_clear(&timecoder); // class
		timecoder_free_lookup(); // static

		//
		delete shortBuffer;

		return UNITY_AUDIODSP_OK;
	}

	UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ProcessCallback(UnityAudioEffectState* state, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int outchannels)
	{
		EffectData* data = state->GetEffectData<EffectData>();

		memcpy(outbuffer, inbuffer, sizeof(float) * length * inchannels);

		// TODO : find out if this is the best way.
		if (!shortBufferInited)
		{
			shortBuffer = new short[length * inchannels];
			shortBufferInited = true;
		}

		// convert from -1 to 1 to a 16-byte signed short integer
		int bufferSize = length * inchannels;
		for (int i = 0; i < bufferSize; i++)
		{
			shortBuffer[i] = inbuffer[i] * (1 << 15);
		}

		//
		timecoder_submit(&timecoder, &shortBuffer[0], bufferSize);
		data->pitch = timecoder_get_pitch(&timecoder);

		return UNITY_AUDIODSP_OK;
	}

	UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK SetFloatParameterCallback(UnityAudioEffectState* state, int index, float value)
	{
		EffectData* data = state->GetEffectData<EffectData>();
		if (index >= P_NUM)
			return UNITY_AUDIODSP_ERR_UNSUPPORTED;
		data->p[index] = value;
		return UNITY_AUDIODSP_OK;
	}

	UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK GetFloatParameterCallback(UnityAudioEffectState* state, int index, float* value, char *valuestr)
	{
		EffectData* data = state->GetEffectData<EffectData>();
		if (value != NULL)
			*value = data->p[index];
		if (valuestr != NULL)
			valuestr[0] = 0;
		return UNITY_AUDIODSP_OK;
	}

	int UNITY_AUDIODSP_CALLBACK GetFloatBufferCallback(UnityAudioEffectState* state, const char* name, float* buffer, int numsamples)
	{
		EffectData* data = state->GetEffectData<EffectData>();
		HistoryBuffer& l = data->history[0];
		HistoryBuffer& r = data->history[1];
		int w1 = l.writeindex;
		int w2 = r.writeindex;
		for (int n = 0; n < numsamples / 2; n++)
		{
			buffer[n * 2 + 0] = l.data[w1];
			if (--w1 < 0)
				w1 = l.length - 1;
			if (n * 2 + 1 < numsamples)
				buffer[n * 2 + 1] = r.data[w2];
			if (--w2 < 0)
				w2 = r.length - 1;
		}
		return UNITY_AUDIODSP_OK;
	}
}
