/*
** Copyright 2008, The Android Open-Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef ANDROID_AUDIO_HARDWARE_H
#define ANDROID_AUDIO_HARDWARE_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/threads.h>
#include <utils/SortedVector.h>

#include <hardware_legacy/AudioHardwareBase.h>

extern "C" {
#include <linux/msm_audio.h>
#include <linux/msm_audio_voicememo.h>
}

namespace android {

// ----------------------------------------------------------------------------
// Kernel driver interface
//

#define SAMP_RATE_INDX_8000     0
#define SAMP_RATE_INDX_11025    1
#define SAMP_RATE_INDX_12000    2
#define SAMP_RATE_INDX_16000    3
#define SAMP_RATE_INDX_22050    4
#define SAMP_RATE_INDX_24000    5
#define SAMP_RATE_INDX_32000    6
#define SAMP_RATE_INDX_44100    7
#define SAMP_RATE_INDX_48000    8

#define EQ_MAX_BAND_NUM        12

#define ADRC_ENABLE        0x0001
#define ADRC_DISABLE       0xFFFE
#define EQ_ENABLE          0x0002
#define EQ_DISABLE         0xFFFD
#define RX_IIR_ENABLE      0x0004
#define RX_IIR_DISABLE     0xFFFB
#define MBADRC_ENABLE      0x0010
#define MBADRC_DISABLE     0xFFEF

#define AGC_ENABLE         0x0001
#define NS_ENABLE          0x0002
#define TX_IIR_ENABLE      0x0004

//Logical devices
#define PCM_OUT_DEVICE     "/dev/msm_pcm_out"
#define PCM_IN_DEVICE      "/dev/msm_pcm_in"
#define PCM_CTL_DEVICE     "/dev/msm_pcm_ctl"
#define PREPROC_CTL_DEVICE "/dev/msm_preproc_ctl"
#define VOICE_MEMO_DEVICE  "/dev/msm_voicememo"

//Sampling rates
const uint32_t inputSamplingRates[] = {
    8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000
};

//Audio hardware related parameters
#define CODEC_TYPE_PCM 0
#define AUDIO_HW_NUM_OUT_BUF 2                              // Number of buffers in audio driver for output
#define AUDIO_HW_OUT_LATENCY_MS 80                          // Additional latency introduced by audio DSP and hardware in ms

#define AUDIO_HW_IN_SAMPLERATE 8000                         // Default audio input sample rate
#define AUDIO_HW_IN_CHANNELS (AudioSystem::CHANNEL_IN_MONO) // Default audio input channel mask
#define AUDIO_HW_IN_BUFFERSIZE 2048                         // Default audio input buffer size
#define AUDIO_HW_IN_FORMAT (AudioSystem::PCM_16_BIT)        // Default audio input sample format

//Internal structures definition
struct eq_filter_type {
    int16_t  gain;
    uint16_t freq;
    uint16_t type;
    uint16_t qf;
};

struct eq_filter {
    uint16_t bands;
    uint16_t params[132];
};

struct rx_iir_filter {
    uint16_t num_bands;
    uint16_t iir_params[48];
};

struct adrc_filter {
    uint16_t adrc_params[8];
};

struct msm_audio_stats {
    uint32_t out_bytes;
    uint32_t unused[3];
};

struct tx_iir {
    uint16_t  cmd_id;
    uint16_t  active_flag;
    uint16_t  num_bands;
    uint16_t  iir_params[48];
};

struct ns {
    uint16_t  cmd_id;
    uint16_t  ec_mode_new;
    uint16_t  dens_gamma_n;
    uint16_t  dens_nfe_block_size;
    uint16_t  dens_limit_ns;
    uint16_t  dens_limit_ns_d;
    uint16_t  wb_gamma_e;
    uint16_t  wb_gamma_n;
};

struct tx_agc {
    uint16_t  cmd_id;
    uint16_t  tx_agc_param_mask;
    uint16_t  tx_agc_enable_flag;
    uint16_t  static_gain;
    int16_t   adaptive_gain_flag;
    uint16_t  agc_params[19];
};

struct adrc_config {
    uint16_t adrc_band_params[10];
};

struct adrc_ext_buf {
    int16_t buff[196];
};

struct mbadrc_filter {
    uint16_t num_bands;
    uint16_t down_samp_level;
    uint16_t adrc_delay;
    uint16_t ext_buf_size;
    uint16_t ext_partition;
    uint16_t ext_buf_msw;
    uint16_t ext_buf_lsw;
    adrc_config adrc_band[5];
    adrc_ext_buf  ext_buf;
};

enum tty_modes {
    TTY_OFF = 0,
    TTY_VCO = 1,
    TTY_HCO = 2,
    TTY_FULL = 3
};

class AudioHardware : public  AudioHardwareBase {
    class AudioStreamOutMSM72xx;
    class AudioStreamInMSM72xx;

    public:
                            AudioHardware();
        virtual             ~AudioHardware();
        virtual status_t    initCheck();

		virtual status_t    setVoiceVolume(float volume);
		virtual status_t    setMasterVolume(float volume);
#ifdef HAVE_FM_RADIO
		virtual status_t    setFmVolume(float volume);
#endif
		virtual status_t    setMode(int mode);

		// mic mute
		virtual status_t    setMicMute(bool state);
		virtual status_t    getMicMute(bool* state);

		virtual status_t    setParameters(const String8& keyValuePairs);
		virtual String8     getParameters(const String8& keys);

		// create I/O streams
		virtual AudioStreamOut* openOutputStream(
									uint32_t devices,
									int *format=0,
									uint32_t *channels=0,
									uint32_t *sampleRate=0,
									status_t *status=0);

		virtual AudioStreamIn* openInputStream(
									uint32_t devices,
									int *format,
									uint32_t *channels,
									uint32_t *sampleRate,
									status_t *status,
									AudioSystem::audio_in_acoustics acoustics);

		virtual	void        closeOutputStream(AudioStreamOut* out);
		virtual void        closeInputStream(AudioStreamIn* in);

		virtual size_t      getInputBufferSize(uint32_t sampleRate, int format, int channelCount);
				void        clearCurDevice() { mCurSndDevice = -1; }

	protected:
        virtual status_t    dump(int fd, const Vector<String16>& args);

	private:
        int          get_sound_endpoints(void);
        int          get_auto_volume_config(void);

        void         audpp_token_error();
        int          get_device_id(char device_code);
        int          get_sample_index(char sample_code);
        int          get_audpp_filter(void);
        int          check_and_set_audpp_parameters(char *buf, int size);

        int          msm72xx_enable_preproc(bool state);
        int          msm72xx_enable_postproc(bool state);
        unsigned int calculate_audpre_table_index(unsigned index);
        status_t     doAudioRouteOrMute(int32_t device);
        status_t     doRouting(AudioStreamInMSM72xx *input);
        status_t     setMicMute_nosync(bool state);
        status_t     checkMicMute();
        status_t     dumpInternals(int fd, const Vector<String16>& args);
        uint32_t     getInputSampleRate(uint32_t sampleRate);
        bool         checkOutputStandby();
        status_t     set_volume_rpc(int32_t device,uint32_t method,uint32_t volume);
        char *       get_sound_device(int32_t device);
        status_t     do_route_audio_rpc(int32_t device,bool ear_mute,bool mic_mute);
#ifdef HAVE_FM_RADIO
        status_t     setFmOnOff(bool onoff);
#endif
        AudioStreamInMSM72xx*   getActiveInput_l();

		class AudioStreamOutMSM72xx : public AudioStreamOut {
			public:
									AudioStreamOutMSM72xx();
				virtual             ~AudioStreamOutMSM72xx();
						status_t    set(AudioHardware* mHardware,
										uint32_t devices,
										int *pFormat,
										uint32_t *pChannels,
										uint32_t *pRate);
				virtual uint32_t    sampleRate() const { return 44100; }

				// must be 32-bit aligned - driver only seems to like 4800
				virtual size_t      bufferSize() const { return 4800; }
				virtual uint32_t    channels() const { return AudioSystem::CHANNEL_OUT_STEREO; }
				virtual int         format() const { return AudioSystem::PCM_16_BIT; }
				virtual uint32_t    latency() const { return (1000*AUDIO_HW_NUM_OUT_BUF*(bufferSize()/frameSize()))/sampleRate()+AUDIO_HW_OUT_LATENCY_MS; }
				virtual status_t    setVolume(float left, float right) { return INVALID_OPERATION; }
				virtual ssize_t     write(const void* buffer, size_t bytes);
				virtual status_t    standby();
				virtual status_t    dump(int fd, const Vector<String16>& args);
						bool        checkStandby();
				virtual status_t    setParameters(const String8& keyValuePairs);
				virtual String8     getParameters(const String8& keys);
						uint32_t    devices() { return mDevices; }
				virtual status_t    getRenderPosition(uint32_t *dspFrames);

            private:
                        void        close_stream(size_t count);

			private:
				AudioHardware* mHardware;
				int         mFd;
				int         mStartCount;
				int         mRetryCount;
				bool        mStandby;
				uint32_t    mDevices;
		};

		class AudioStreamInMSM72xx : public AudioStreamIn {
			public:
				enum input_state {
					AUDIO_INPUT_CLOSED,
					AUDIO_INPUT_OPENED,
					AUDIO_INPUT_STARTED
				};

									AudioStreamInMSM72xx();
				virtual             ~AudioStreamInMSM72xx();
						status_t    set(AudioHardware* mHardware,
										uint32_t devices,
										int *pFormat,
										uint32_t *pChannels,
										uint32_t *pRate,
										AudioSystem::audio_in_acoustics acoustics);
				virtual size_t      bufferSize() const { return mBufferSize; }
				virtual uint32_t    channels() const { return mChannels; }
				virtual int         format() const { return mFormat; }
				virtual uint32_t    sampleRate() const { return mSampleRate; }
				virtual status_t    setGain(float gain) { return INVALID_OPERATION; }
				virtual ssize_t     read(void* buffer, ssize_t bytes);
				virtual status_t    dump(int fd, const Vector<String16>& args);
				virtual status_t    standby();
				virtual status_t    setParameters(const String8& keyValuePairs);
				virtual String8     getParameters(const String8& keys);
				virtual unsigned int getInputFramesLost() const { return 0; }
						uint32_t    devices() { return mDevices; }
						int         state() const { return mState; }

            private:
                        void        close_stream(void);

			private:
				AudioHardware* mHardware;
				int            mFd;
				int            mState;
				int            mRetryCount;
				int            mFormat;
				uint32_t       mChannels;
				uint32_t       mSampleRate;
				size_t         mBufferSize;
				AudioSystem::audio_in_acoustics mAcoustics;
				uint32_t       mDevices;
				bool           mFirstread;
		};

	private:
		// Post processing parameters
		rx_iir_filter iir_cfg[3];
		adrc_filter   adrc_cfg[3];
		mbadrc_filter mbadrc_cfg[3];
		eq_filter     equalizer[3];
		uint16_t      adrc_flag[3];
		uint16_t      mbadrc_flag[3];
		uint16_t      eq_flag[3];
		uint16_t      rx_iir_flag[3];
		uint16_t      agc_flag[3];
		uint16_t      ns_flag[3];
		uint16_t      txiir_flag[3];
		bool          audpp_filter_inited;
		int           post_proc_feature_mask;
		bool          playback_in_progress;

		//Pre processing parameters
		tx_iir       tx_iir_cfg[9];
		ns           ns_cfg[9];
		tx_agc       tx_agc_cfg[9];
		int          enable_preproc_mask[9];

		//Sound devices
		int32_t      mActSndDevice;
		int32_t      mCurSndDevice;

		//Sound devices definition
		int32_t      SND_DEVICE_CURRENT;
		int32_t      SND_DEVICE_HANDSET;
		int32_t      SND_DEVICE_SPEAKER;
		int32_t      SND_DEVICE_MEDIA_SPEAKER;
		int32_t      SND_DEVICE_BT;
		int32_t      SND_DEVICE_BT_EC_OFF;
		int32_t      SND_DEVICE_HEADSET;
		int32_t      SND_DEVICE_HEADSET_AND_SPEAKER;
		int32_t      SND_DEVICE_IN_S_SADC_OUT_HANDSET;
		int32_t      SND_DEVICE_IN_S_SADC_OUT_SPEAKER_PHONE;
		int32_t      SND_DEVICE_TTY_HEADSET;
		int32_t      SND_DEVICE_TTY_HCO;
		int32_t      SND_DEVICE_TTY_VCO;
		int32_t      SND_DEVICE_CARKIT;
		int32_t      SND_DEVICE_FM_SPEAKER;
		int32_t      SND_DEVICE_FM_HEADSET;
		int32_t      SND_DEVICE_NO_MIC_HEADSET;

		//Low level audio driver handle
		int          m7xsnddriverfd;
		bool         mInit;
		bool         mBluetoothNrec;
		uint32_t     mBluetoothId;
		int          mFmRadioEnabled;
		int          mFmPrev;
		int          mFmVolume;
		int          mTtyMode;
		int          fmfd;
		bool         mMicMute;
		bool         mDualMicEnabled;
		bool         mBuiltinMicSelected;

		//Indexes
        int         tx_iir_index;
		int         audpre_index;

		AudioStreamOutMSM72xx*  mOutput;
		SortedVector <AudioStreamInMSM72xx*>   mInputs;

		msm_snd_endpoint *mSndEndpoints;
		int          mNumSndEndpoints;

	friend class AudioStreamInMSM72xx;
		Mutex       mLock;
};

// ----------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_AUDIO_HARDWARE_MSM72XX_H
