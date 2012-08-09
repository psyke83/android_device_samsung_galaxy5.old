/*
** Copyright 2008, The Android Open-Source Project
** Copyright (c) 2010-2011, Code Aurora Forum. All rights reserved.
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

#include <cutils/properties.h>
#include <math.h>

//#define LOG_NDEBUG  0
#include <utils/Log.h>
#include <utils/String8.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <fcntl.h>

// hardware specific functions
#include "AudioHardware.h"
#include <media/AudioRecord.h>

#define COMBO_DEVICE_SUPPORTED 1 // Headset speaker combo device is supported on this target

#define DUALMIC_KEY           "dualmic_enabled"
#define TTY_MODE_KEY          "tty_mode"

#undef  LOG_TAG
#define LOG_TAG               "AudioHardwareMSM72XX"

#define LOG_SND_RPC 0          // Set to 1 to log sound RPC's

namespace android {

// ----------------------------------------------------------------------------

AudioHardware::AudioHardware() {
    //Internal structures initialization
    memset(iir_cfg,0,sizeof(iir_cfg));
    memset(adrc_cfg,0,sizeof(adrc_cfg));
    memset(mbadrc_cfg,0,sizeof(mbadrc_cfg));
    memset(equalizer,0,sizeof(equalizer));
    memset(adrc_flag,0,sizeof(adrc_flag));
    memset(mbadrc_flag,0,sizeof(mbadrc_flag));
    memset(eq_flag,0,sizeof(eq_flag));
    memset(rx_iir_flag,0,sizeof(rx_iir_flag));
    memset(agc_flag,0,sizeof(agc_flag));
    memset(ns_flag,0,sizeof(ns_flag));
    memset(txiir_flag,0,sizeof(txiir_flag));

    //Internal flags
    audpp_filter_inited = false;
    playback_in_progress = false;
    post_proc_feature_mask = (ADRC_ENABLE | EQ_ENABLE | RX_IIR_ENABLE | MBADRC_ENABLE);

    //Pre processing parameters
    memset(tx_iir_cfg,0,sizeof(tx_iir_cfg));
    memset(ns_cfg,0,sizeof(ns_cfg));
    memset(tx_agc_cfg,0,sizeof(tx_agc_cfg));
    memset(enable_preproc_mask,0,sizeof(enable_preproc_mask));

    //Current sound devices
    mActSndDevice = -1;
    mCurSndDevice = -1;

    //Sound devices definition
    SND_DEVICE_CURRENT = -1;
    SND_DEVICE_HANDSET = -1;
    SND_DEVICE_SPEAKER = -1;
    SND_DEVICE_MEDIA_SPEAKER = -1;
    SND_DEVICE_BT = -1;
    SND_DEVICE_BT_EC_OFF = 32; // really BT_NSEC_OFF
    SND_DEVICE_HEADSET = -1;
    SND_DEVICE_HEADSET_AND_SPEAKER = 26; // really FORCE_SPEAKER
    SND_DEVICE_IN_S_SADC_OUT_HANDSET = -1;
    SND_DEVICE_IN_S_SADC_OUT_SPEAKER_PHONE = -1;
    SND_DEVICE_TTY_HEADSET = -1;
    SND_DEVICE_TTY_HCO = -1;
    SND_DEVICE_TTY_VCO = -1;
    SND_DEVICE_CARKIT = -1;
    SND_DEVICE_FM_SPEAKER = -1;
    SND_DEVICE_FM_HEADSET = 17; // really IN_S_SADC_OUT_HEADSET
    SND_DEVICE_NO_MIC_HEADSET = -1;

    //Internal status flags
    mInit = false;
    mMicMute = true;
    mBluetoothNrec = true;
    mBluetoothId = 0;
    mOutput = 0;
    mSndEndpoints = NULL;
    mDualMicEnabled = false;
    mBuiltinMicSelected = false;
    mFmRadioEnabled = false;
    mFmPrev = false;
    mFmVolume = 0;
    fmfd = -1;

    //Open the audio driver
    m7xsnddriverfd = open("/dev/msm_snd", O_RDWR);
    if (m7xsnddriverfd < 0) {
    	LOGE("Could not open MSM SND driver.");
    } else {
    	//Get the audio endpoints
    	if (get_sound_endpoints() < 0) {
            LOGE("Could not retrieve number of MSM SND endpoints.");
    	}
        else {
        	//Get the audio filters definition
        	if (get_audpp_filter() == 0) {
        		//Audio filters initializeds without errors
        		audpp_filter_inited = true;
        	}

        	// Get AUTO VOLUME (enabled as default)
        	int AUTO_VOLUME_ENABLED = get_auto_volume_config();

        	// Set the driver AVC and AGC
            ioctl(m7xsnddriverfd, SND_AVC_CTL, &AUTO_VOLUME_ENABLED);
            ioctl(m7xsnddriverfd, SND_AGC_CTL, &AUTO_VOLUME_ENABLED);
    	}
    }
}

AudioHardware::~AudioHardware() {
	//Close input streams
	for (size_t index = 0; index < mInputs.size(); index++) {
        closeInputStream((AudioStreamIn*)mInputs[index]);
    }
    mInputs.clear();

    //Close output stream
    closeOutputStream((AudioStreamOut*)mOutput);

    //Delete endpoints
    delete [] mSndEndpoints;

    //Close and release sound driver handle
    if (m7xsnddriverfd > 0)
    {
      close(m7xsnddriverfd);
      m7xsnddriverfd = -1;
    }

    //Reset pre-processing mask
    for (int index = 0; index < 9; index++) {
        enable_preproc_mask[index] = 0;
    }
    mInit = false;
}

status_t AudioHardware::initCheck()
{
    return mInit ? NO_ERROR : NO_INIT;
}

int AudioHardware::get_sound_endpoints(void)
{
    //Get the audio endpoints
    int rc = ioctl(m7xsnddriverfd, SND_GET_NUM_ENDPOINTS, &mNumSndEndpoints);
    if (rc >= 0) {
        //Construct the endpoints
        mSndEndpoints = new msm_snd_endpoint[mNumSndEndpoints];
        mInit = true;
        //Switch to front mic (Samsung hack)
        ioctl(m7xsnddriverfd, SND_SET_MAIN_MIC);
        LOGV("constructed (%d SND endpoints)", mNumSndEndpoints);

        //Map the endpoints on relative structures
        struct msm_snd_endpoint *ept = mSndEndpoints;

        //Scan and check the endpoints
        for (int cnt = 0; cnt < mNumSndEndpoints; cnt++, ept++) {
            ept->id = cnt;
            ioctl(m7xsnddriverfd, SND_GET_ENDPOINT, ept);
            LOGV("cnt = %d ept->name = %s ept->id = %d", cnt, ept->name, ept->id);

			#define CHECK_FOR(desc) if (!strcmp(ept->name, #desc)) SND_DEVICE_##desc = ept->id;
            CHECK_FOR(CURRENT);
            CHECK_FOR(HANDSET);
            CHECK_FOR(SPEAKER);
            CHECK_FOR(MEDIA_SPEAKER);
            CHECK_FOR(BT);
            CHECK_FOR(BT_EC_OFF);
            CHECK_FOR(HEADSET);
            CHECK_FOR(NO_MIC_HEADSET);
            CHECK_FOR(HEADSET_AND_SPEAKER);
            CHECK_FOR(IN_S_SADC_OUT_HANDSET);
            CHECK_FOR(IN_S_SADC_OUT_SPEAKER_PHONE);
            CHECK_FOR(TTY_HEADSET);
            CHECK_FOR(TTY_HCO);
            CHECK_FOR(TTY_VCO);
#ifdef HAVE_FM_RADIO
            CHECK_FOR(FM_SPEAKER);
            CHECK_FOR(FM_HEADSET);
#endif
            #undef CHECK_FOR
        }
    }
    return rc;
}

int AudioHardware::get_auto_volume_config(void)
{
	int     txtfd;
	struct  stat st;
	char   *read_buf;
	int     enabled = 1;
	static const char *const path =
			"/system/etc/AutoVolumeControl.txt";

	//Open configuration file
	txtfd = open(path, O_RDONLY);
	if (txtfd < 0) {
		LOGE("failed to open AUTO_VOLUME_CONTROL %s: %s (%d)",
			  path, strerror(errno), errno);
	} else {
		//Check file status and size
		if (fstat(txtfd, &st) < 0) {
			LOGE("failed to stat %s: %s (%d)",
				  path, strerror(errno), errno);
		} else {
			//Get the file data
			read_buf = (char *) mmap(0, st.st_size,
						PROT_READ | PROT_WRITE,
						MAP_PRIVATE,
						txtfd, 0);

			//Check file mmap result
			if (read_buf == MAP_FAILED) {
				LOGE("failed to mmap parameters file: %s (%d)",
					  strerror(errno), errno);
			} else {
				//Analyze the first char
				if(read_buf[0] == '0')
				   enabled = 0;
				munmap(read_buf, st.st_size);
			}
		}

		//Close configuration file
		close(txtfd);
	}
	return enabled;
}

void AudioHardware::audpp_token_error() {
    LOGE("malformatted pcm control buffer");
}

int AudioHardware::get_device_id(char device_code) {
    int device_id;

    switch (device_code) {
        case '1':
            device_id = 0;
            break;
        case '2':
            device_id = 1;
            break;
        case '3':
            device_id = 2;
            break;
        default:
            device_id = -EINVAL;
            break;
    }
    return device_id;
}

int AudioHardware::get_sample_index(char sample_code) {
    int sample_index;

    switch (sample_code) {
        case '1':
            sample_index = 0;
            break;
        case '2':
            sample_index = 1;
            break;
        case '3':
            sample_index = 2;
            break;
        case '4':
            sample_index = 3;
            break;
        case '5':
            sample_index = 4;
            break;
        case '6':
            sample_index = 5;
            break;
        case '7':
            sample_index = 6;
            break;
        case '8':
            sample_index = 7;
            break;
        case '9':
            sample_index = 8;
            break;
        default:
            sample_index = -EINVAL;
            break;
    }

    return sample_index;
}

int AudioHardware::check_and_set_audpp_parameters(char *buf, int size) {
    char *p, *ps;
    static const char *const seps = ",";
    int table_num;
    int i, j;
    int device_id = 0;
    int samp_index = 0;
    int fd;
    void *audioeq;
    void *(*eq_cal)(int32_t, int32_t, int32_t, uint16_t, int32_t, int32_t *, int32_t *, uint16_t *);
    eq_filter_type eq[12];
    uint16_t numerator[6];
    uint16_t denominator[4];
    uint16_t shift[2];

    // Process the data
    if ((buf[0] == 'A') && ((buf[1] == '1') || (buf[1] == '2') || (buf[1] == '3'))) {
        //IIR record (code 'A')
        device_id = get_device_id(buf[1]);

        //Table header
        if (!(p = strtok(buf, ","))) { audpp_token_error(); return -EINVAL;}
        table_num = strtol(p + 1, &ps, 10);

        //Table description
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}

        //IIR parameters
        for (i = 0; i < 48; i++) {
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            iir_cfg[device_id].iir_params[i] = (uint16_t)strtol(p, &ps, 16);
        }

        //IIR flag
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        rx_iir_flag[device_id] = (uint16_t)strtol(p, &ps, 16);

        //Number of bands
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        iir_cfg[device_id].num_bands = (uint16_t)strtol(p, &ps, 16);

        //Debug output
        LOGI("IIR flag[%d] = %02x.", device_id, rx_iir_flag[device_id]);

    } else if ((buf[0] == 'B') && ((buf[1] == '1') || (buf[1] == '2') || (buf[1] == '3'))) {
        //ADRC record (code 'B')
        device_id = get_device_id(buf[1]);

        //Table header
        if (!(p = strtok(buf, ","))) { audpp_token_error(); return -EINVAL;}
        table_num = strtol(p + 1, &ps, 10);

        //Table description
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}

        //ADRC Filter ADRC FLAG
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        adrc_flag[device_id] = (uint16_t)strtol(p, &ps, 16);

        //ADRC Filter COMP THRESHOLD
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        adrc_cfg[device_id].adrc_params[0] = (uint16_t)strtol(p, &ps, 16);

        //ADRC Filter COMP SLOPE
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        adrc_cfg[device_id].adrc_params[1] = (uint16_t)strtol(p, &ps, 16);

        //ADRC Filter COMP RMS TIME
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        adrc_cfg[device_id].adrc_params[2] = (uint16_t)strtol(p, &ps, 16);

        //ADRC Filter COMP ATTACK[0]
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        adrc_cfg[device_id].adrc_params[3] = (uint16_t)strtol(p, &ps, 16);

        //ADRC Filter COMP ATTACK[1]
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        adrc_cfg[device_id].adrc_params[4] = (uint16_t)strtol(p, &ps, 16);

        //ADRC Filter COMP RELEASE[0]
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        adrc_cfg[device_id].adrc_params[5] = (uint16_t)strtol(p, &ps, 16);

        //ADRC Filter COMP RELEASE[1]
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        adrc_cfg[device_id].adrc_params[6] = (uint16_t)strtol(p, &ps, 16);

        //ADRC Filter COMP DELAY
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        adrc_cfg[device_id].adrc_params[7] = (uint16_t)strtol(p, &ps, 16);

        //Debug output
        LOGI("ADRC flag[%d] = %02x.", device_id, adrc_flag[device_id]);

    } else if (buf[0] == 'C' && ((buf[1] == '1') || (buf[1] == '2') || (buf[1] == '3'))) {
        //EQ record (code 'C')
        device_id = get_device_id(buf[1]);

        //Table header
        if (!(p = strtok(buf, ","))) { audpp_token_error(); return -EINVAL;}
        table_num = strtol(p + 1, &ps, 10);

        //Table description
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}

        //EQ flag
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        eq_flag[device_id] = (uint16_t)strtol(p, &ps, 16);

        //Open equalizer
        audioeq = ::dlopen("/system/lib/libaudioeq.so", RTLD_NOW);
        if (audioeq == NULL) {
            LOGE("audioeq library open failure");
            return -1;
        }

        //Let equalizer calculate coefficients
        eq_cal = (void *(*) (int32_t, int32_t, int32_t, uint16_t, int32_t, int32_t *, int32_t *, uint16_t *))::dlsym(audioeq, "audioeq_calccoefs");

        //Cleanup the equalizer area
        memset(&equalizer[device_id], 0, sizeof(eq_filter));

        //Set the equalization bands for the device
        equalizer[device_id].bands = 8;
        for (i = 0; i < equalizer[device_id].bands; i++) {
            //EQ Filter Gain (for the current band)
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            eq[i].gain = (uint16_t)strtol(p, &ps, 16);

            //EQ Filter Frequency (for the current band)
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            eq[i].freq = (uint16_t)strtol(p, &ps, 16);

            //EQ Filter Type (for the current band)
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            eq[i].type = (uint16_t)strtol(p, &ps, 16);

            //EQ Filter Quality factor (for the current band)
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            eq[i].qf = (uint16_t)strtol(p, &ps, 16);

            //Perform equalization
            eq_cal(eq[i].gain, eq[i].freq, 48000, eq[i].type, eq[i].qf, (int32_t*)numerator, (int32_t *)denominator, shift);

            int params_offset;

            //Store equalization results (numerators) in band parameters area (initial offset = band id * 6)
            for (j = 0; j < 6; j++) {
                params_offset = (i * 6);
                equalizer[device_id].params[params_offset + j] = numerator[j];
            }

            //Store equalization results (denominators) in band parameters area (initial offset = band number * 6 + band id * 4)
            for (j = 0; j < 4; j++) {
                params_offset = (equalizer[device_id].bands * 6) + (i * 4);
                equalizer[device_id].params[params_offset + j] = denominator[j];
            }
            equalizer[device_id].params[(equalizer[device_id].bands * 10) + i] = shift[0];
        }

        //Close equalizer
        ::dlclose(audioeq);

        //Debug output
        LOGI("EQ flag[%d] = %02x.", device_id, eq_flag[device_id]);

    } else if ((buf[0] == 'D') && ((buf[1] == '1') || (buf[1] == '2') || (buf[1] == '3'))) {
        //MB_ADRC record (code 'D')
        device_id = get_device_id(buf[1]);

        //Table header
        if (!(p = strtok(buf, ","))) { audpp_token_error(); return -EINVAL;}
        table_num = strtol(p + 1, &ps, 10);

        //Table description
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}

        //MB_ADRC Filter number of bands
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        mbadrc_cfg[device_id].num_bands = (uint16_t)strtol(p, &ps, 16);

        //MB_ADRC Filter sample levels
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        mbadrc_cfg[device_id].down_samp_level = (uint16_t)strtol(p, &ps, 16);

        //MB_ADRC Filter ADRC delay
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        mbadrc_cfg[device_id].adrc_delay = (uint16_t)strtol(p, &ps, 16);

        //MB_ADRC Filter external buffer size
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        mbadrc_cfg[device_id].ext_buf_size = (uint16_t)strtol(p, &ps, 16);

        //MB_ADRC Filter external partition
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        mbadrc_cfg[device_id].ext_partition = (uint16_t)strtol(p, &ps, 16);

        //MB_ADRC Filter external buffer MSW
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        mbadrc_cfg[device_id].ext_buf_msw = (uint16_t)strtol(p, &ps, 16);

        //MB_ADRC Filter external buffer LSW
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        mbadrc_cfg[device_id].ext_buf_lsw = (uint16_t)strtol(p, &ps, 16);

        //For each configured band set the filter parameters
        for(i = 0; i < mbadrc_cfg[device_id].num_bands; i++) {
            //Initialize the filter parameters for the band
            for(j = 0; j < 10; j++) {
                if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
                mbadrc_cfg[device_id].adrc_band[i].adrc_band_params[j] = (uint16_t)strtol(p, &ps, 16);
            }
        }

        //Set the filter parameters in the external buffer
        for(i = 0;i < mbadrc_cfg[device_id].ext_buf_size/2; i++) {
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            mbadrc_cfg[device_id].ext_buf.buff[i] = (uint16_t)strtol(p, &ps, 16);
        }

        //MB_ADRC Filter MBADRC FLAG
        if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
        mbadrc_flag[device_id] = (uint16_t)strtol(p, &ps, 16);

        //Debug output
        LOGI("MBADRC flag[%d] = %02x.", device_id, mbadrc_flag[device_id]);

    } else if ((buf[0] == 'E') || (buf[0] == 'F') || (buf[0] == 'G')){
        // Get the sample index
        if ((samp_index = get_sample_index(buf[1]) == -EINVAL)) { audpp_token_error(); return -EINVAL;}

        //Pre-Processing features records TX_IIR,AGC,NS (codes 'E','F','G')
        if (buf[0] == 'E')  {
            //TX_IIR Filter table header
            if (!(p = strtok(buf, ","))) { audpp_token_error(); return -EINVAL;}
            table_num = strtol(p + 1, &ps, 10);

            //TX_IIR Filter table description
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}

            //TX_IIR Filter parameters
            for (i = 0; i < 48; i++) {
                if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
                j = (i >= 40)? i : ((i % 2)? (i - 1) : (i + 1));
                tx_iir_cfg[samp_index].iir_params[j] = (uint16_t)strtol(p, &ps, 16);
            }

            //TX_IIR Filter active flag
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            tx_iir_cfg[samp_index].active_flag = (uint16_t)strtol(p, &ps, 16);

            //TX_IIR Filter FLAG
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            txiir_flag[device_id] = (uint16_t)strtol(p, &ps, 16);

            //TX_IIR Filter number of bands
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            tx_iir_cfg[samp_index].num_bands = (uint16_t)strtol(p, &ps, 16);

            //TX_IIR Filter command id
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            tx_iir_cfg[samp_index].cmd_id = 0;

            //TX_IIR Filter check and eventually enable preprocessing bit
            if (txiir_flag[device_id] != 0)
                 enable_preproc_mask[samp_index] |= TX_IIR_ENABLE;

           //Debug output
            LOGI("TX IIR flag[%d] = %02x.", device_id,txiir_flag[device_id]);

        } else if (buf[0] == 'F')  {
            //AGC Filter table header
            if (!(p = strtok(buf, ","))) { audpp_token_error(); return -EINVAL;}
            table_num = strtol(p + 1, &ps, 10);

            //AGC Filter command id
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            tx_agc_cfg[samp_index].cmd_id = (uint16_t)strtol(p, &ps, 16);

            //AGC Filter tx_agc parameters mask
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            tx_agc_cfg[samp_index].tx_agc_param_mask = (uint16_t)strtol(p, &ps, 16);

            //AGC Filter tx_agc enable flag
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            tx_agc_cfg[samp_index].tx_agc_enable_flag = (uint16_t)strtol(p, &ps, 16);

            //AGC Filter static gain
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            tx_agc_cfg[samp_index].static_gain = (uint16_t)strtol(p, &ps, 16);

            //AGC Filter adaptive gain flag
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            tx_agc_cfg[samp_index].adaptive_gain_flag = (uint16_t)strtol(p, &ps, 16);

            //AGC Filter parameters
            for (i = 0; i < 19; i++) {
                if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
                tx_agc_cfg[samp_index].agc_params[i] = (uint16_t)strtol(p, &ps, 16);
            }

            //AGC Filter AGC FLAG
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            agc_flag[device_id] = (uint16_t)strtol(p, &ps, 16);

            //AGC Filter check and eventually enable preprocessing bit
            if (agc_flag[device_id] != 0)
                enable_preproc_mask[samp_index] |= AGC_ENABLE;

            //Debug output
            LOGI("AGC flag[%d] = %02x.", device_id, agc_flag[device_id]);

        } else if ((buf[0] == 'G')) {
            //NS record table header
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            table_num = strtol(p + 1, &ps, 10);

            //NS record command id
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            ns_cfg[samp_index].cmd_id = (uint16_t)strtol(p, &ps, 16);

            //NS record ec_mode
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            ns_cfg[samp_index].ec_mode_new = (uint16_t)strtol(p, &ps, 16);

            //NS record dens_gamma_n
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            ns_cfg[samp_index].dens_gamma_n = (uint16_t)strtol(p, &ps, 16);

            //NS record dens_nfe_block_size
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            ns_cfg[samp_index].dens_nfe_block_size = (uint16_t)strtol(p, &ps, 16);

            //NS record dens_limit_ns
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            ns_cfg[samp_index].dens_limit_ns = (uint16_t)strtol(p, &ps, 16);

            //NS record dens_limit_ns_d
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            ns_cfg[samp_index].dens_limit_ns_d = (uint16_t)strtol(p, &ps, 16);

            //NS record dens wb_gamma_e
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            ns_cfg[samp_index].wb_gamma_e = (uint16_t)strtol(p, &ps, 16);

            //NS record dens wb_gamma_n
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            ns_cfg[samp_index].wb_gamma_n = (uint16_t)strtol(p, &ps, 16);

            //NS record NS FLAG
            if (!(p = strtok(NULL, seps))) { audpp_token_error(); return -EINVAL;}
            ns_flag[device_id] = (uint16_t)strtol(p, &ps, 16);

            //NS record check and eventually enable preprocessing bit

           if (ns_flag[device_id] != 0)
                enable_preproc_mask[samp_index] |= NS_ENABLE;

           //Debug output
           LOGI("NS flag[%d] = %02x.", device_id, ns_flag[device_id]);

        }
    }
    return 0;
}

int AudioHardware::get_audpp_filter(void)
{
    struct stat st;
    char *read_buf;
    char *next_str, *current_str;
    int csvfd;
	static const char *const path =
    		"/system/etc/AudioFilter.csv";

    LOGI("get_audpp_filter");

    //Open the acoustic filters file
    csvfd = open(path, O_RDONLY);
    if (csvfd < 0) {
        //failed to open normal acoustic file ...
        LOGE("failed to open AUDIO_NORMAL_FILTER %s: %s (%d).",
             path, strerror(errno), errno);
        return -1;
    } else
        LOGI("open %s success.", path);

    if (fstat(csvfd, &st) < 0) {
        LOGE("failed to stat %s: %s (%d).",
             path, strerror(errno), errno);
        close(csvfd);
        return -1;
    }

    //Get the acoustic filters data
    read_buf = (char *) mmap(0, st.st_size,
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE,
                    csvfd, 0);

    if (read_buf == MAP_FAILED) {
        LOGE("failed to mmap parameters file: %s (%d)",
             strerror(errno), errno);
        close(csvfd);
        return -1;
    }

    //Start the acoustic filters data analysis
    current_str = read_buf;

    while (*current_str != (char)EOF)  {
        int len;
        next_str = strchr(current_str, '\n');
        if (!next_str)
           break;
        len = next_str - current_str;
        *next_str++ = '\0';

        //Process the filter data
        if (check_and_set_audpp_parameters(current_str, len)) {
            LOGI("failed to set audpp parameters, exiting.");
            munmap(read_buf, st.st_size);
            close(csvfd);
            return -1;
        }
        current_str = next_str;
    }

    munmap(read_buf, st.st_size);
    close(csvfd);
    return 0;
}

int AudioHardware::msm72xx_enable_preproc(bool state)
{
    uint16_t mask = 0x0000;

    if (audpp_filter_inited)
    {
        int fd;

        fd = open(PREPROC_CTL_DEVICE, O_RDWR);
        if (fd < 0) {
             LOGE("Cannot open PreProc Ctl device");
             return -EPERM;
        }

        if (enable_preproc_mask[audpre_index] & AGC_ENABLE) {
            //Setting AGC Params
            LOGI("AGC Filter Param1= %02x.", tx_agc_cfg[audpre_index].cmd_id);
            LOGI("AGC Filter Param2= %02x.", tx_agc_cfg[audpre_index].tx_agc_param_mask);
            LOGI("AGC Filter Param3= %02x.", tx_agc_cfg[audpre_index].tx_agc_enable_flag);
            LOGI("AGC Filter Param4= %02x.", tx_agc_cfg[audpre_index].static_gain);
            LOGI("AGC Filter Param5= %02x.", tx_agc_cfg[audpre_index].adaptive_gain_flag);
            LOGI("AGC Filter Param6= %02x.", tx_agc_cfg[audpre_index].agc_params[0]);
            LOGI("AGC Filter Param7= %02x.", tx_agc_cfg[audpre_index].agc_params[18]);
            if ((enable_preproc_mask[audpre_index] & AGC_ENABLE) &&
                (ioctl(fd, AUDIO_SET_AGC, &tx_agc_cfg[audpre_index]) < 0))
            {
                LOGE("set AGC filter error.");
            }
        }

        if (enable_preproc_mask[audpre_index] & NS_ENABLE) {
            //Setting NS Params
            LOGI("NS Filter Param1= %02x.", ns_cfg[audpre_index].cmd_id);
            LOGI("NS Filter Param2= %02x.", ns_cfg[audpre_index].ec_mode_new);
            LOGI("NS Filter Param3= %02x.", ns_cfg[audpre_index].dens_gamma_n);
            LOGI("NS Filter Param4= %02x.", ns_cfg[audpre_index].dens_nfe_block_size);
            LOGI("NS Filter Param5= %02x.", ns_cfg[audpre_index].dens_limit_ns);
            LOGI("NS Filter Param6= %02x.", ns_cfg[audpre_index].dens_limit_ns_d);
            LOGI("NS Filter Param7= %02x.", ns_cfg[audpre_index].wb_gamma_e);
            LOGI("NS Filter Param8= %02x.", ns_cfg[audpre_index].wb_gamma_n);
            if ((enable_preproc_mask[audpre_index] & NS_ENABLE) &&
                (ioctl(fd, AUDIO_SET_NS, &ns_cfg[audpre_index]) < 0))
            {
                LOGE("set NS filter error.");
            }
        }

        if (enable_preproc_mask[audpre_index] & TX_IIR_ENABLE) {
            //Setting TX_IIR Params
            LOGI("TX_IIR Filter Param1= %02x.", tx_iir_cfg[audpre_index].cmd_id);
            LOGI("TX_IIR Filter Param2= %02x.", tx_iir_cfg[audpre_index].active_flag);
            LOGI("TX_IIR Filter Param3= %02x.", tx_iir_cfg[audpre_index].num_bands);
            LOGI("TX_IIR Filter Param4= %02x.", tx_iir_cfg[audpre_index].iir_params[0]);
            LOGI("TX_IIR Filter Param5= %02x.", tx_iir_cfg[audpre_index].iir_params[1]);
            LOGI("TX_IIR Filter Param6= %02x.", tx_iir_cfg[audpre_index].iir_params[47]);
            if ((enable_preproc_mask[audpre_index] & TX_IIR_ENABLE) &&
                (ioctl(fd, AUDIO_SET_TX_IIR, &tx_iir_cfg[audpre_index]) < 0))
            {
               LOGE("set TX IIR filter error.");
            }
        }

        if (state == true) {
            //Setting AUDPRE_ENABLE
            if (ioctl(fd, AUDIO_ENABLE_AUDPRE, &enable_preproc_mask[audpre_index]) < 0) {
                LOGE("set AUDPRE_ENABLE error.");
            }
        } else {
            //Setting AUDPRE_ENABLE
            if (ioctl(fd, AUDIO_ENABLE_AUDPRE, &mask) < 0) {
                LOGE("set AUDPRE_ENABLE error.");
            }
        }
        close(fd);
    }

    return NO_ERROR;
}

int AudioHardware::msm72xx_enable_postproc(bool state)
{
    int fd;
    int device_id=0;
    int enable_mask = 0;
    int disable_mask = 0;

    //Check acoustic filters parsing status
    if (!audpp_filter_inited)
    {
        LOGE("Parsing error in AudioFilter.csv.");
        return -EINVAL;
    }

    //Check sound device ID
    if(mCurSndDevice < 0) {
        LOGE("Enabling/Disabling post processing features for device: %d", mCurSndDevice);
        return -EINVAL;
    }

    //Analyze the sound device ID
    if (mCurSndDevice == SND_DEVICE_SPEAKER) {
        device_id = 0;
        LOGI("set device to SND_DEVICE_SPEAKER device_id=0");
    } else if (mCurSndDevice == SND_DEVICE_MEDIA_SPEAKER) {
        device_id = 0;
        LOGI("set device to SND_DEVICE_MEDIA_SPEAKER device_id=0");
    } else if (mCurSndDevice == SND_DEVICE_HANDSET) {
        device_id = 1;
        LOGI("set device to SND_DEVICE_HANDSET device_id=1");
    } else if (mCurSndDevice == SND_DEVICE_HEADSET) {
        device_id = 2;
        LOGI("set device to SND_DEVICE_HEADSET device_id=2");
    } else if (mCurSndDevice == SND_DEVICE_NO_MIC_HEADSET) {
        device_id = 2;
        LOGI("set device to SND_DEVICE_NO_MIC_HEADSET device_id=2");
    } else {
		LOGE("Invalid sound device (%d)", mCurSndDevice);
		return -EINVAL;
    }

    //Open the sound device
    fd = open(PCM_CTL_DEVICE, O_RDWR);
    if (fd < 0) {
        LOGE("Cannot open PCM Ctl device");
        return -EPERM;
    }

    //Check state flag
    if(state){
        // Initialize the post processing mask
        enable_mask = post_proc_feature_mask;

        // MBADRC filter configuration check
        if (!(mbadrc_flag[device_id]) || !(post_proc_feature_mask & MBADRC_ENABLE)) {
            // Disable the MBADRC configuration bit
		    enable_mask &= MBADRC_DISABLE;
        } else {
            // Disable the ADRC configuration bit
		    enable_mask &= ADRC_DISABLE;
        }

        // ADRC filter configuration check
        if (!(adrc_flag[device_id]) || !(post_proc_feature_mask & ADRC_ENABLE)) {
            // Disable the ADRC configuration bit
		    enable_mask &= ADRC_DISABLE;
        }

        // EQ filter configuration check
        if (!(eq_flag[device_id]) || !(post_proc_feature_mask & EQ_ENABLE)) {
            // Disable the EQ configuration bit
            enable_mask &= EQ_DISABLE;
        }

        // IIR filter configuration check
        if (!(rx_iir_flag[device_id]) || !(post_proc_feature_mask & RX_IIR_ENABLE)) {
            // Disable the IIR configuration bit
            enable_mask &= RX_IIR_DISABLE;
        }

    	// Apply the MBADRC filter (if any)
    	if (enable_mask & MBADRC_ENABLE) {
			LOGI("MBADRC Filter MBADRC FLAG = %02x.", mbadrc_flag[device_id]);
			if (ioctl(fd, AUDIO_SET_MBADRC, &mbadrc_cfg[device_id]) < 0)
			{
				LOGE("set mbadrc filter error");
	            close(fd);
	            return -EPERM;
			}
    	} else {
			LOGV("MBADRC Disabled");
    	}

    	// Apply the ADRC filter (if any)
    	if (enable_mask & ADRC_ENABLE) {
			LOGI("ADRC Filter ADRC FLAG = %02x.", adrc_flag[device_id]);
			LOGI("ADRC Filter COMP THRESHOLD = %02x.", adrc_cfg[device_id].adrc_params[0]);
			LOGI("ADRC Filter COMP SLOPE = %02x.", adrc_cfg[device_id].adrc_params[1]);
			LOGI("ADRC Filter COMP RMS TIME = %02x.", adrc_cfg[device_id].adrc_params[2]);
			LOGI("ADRC Filter COMP ATTACK[0] = %02x.", adrc_cfg[device_id].adrc_params[3]);
			LOGI("ADRC Filter COMP ATTACK[1] = %02x.", adrc_cfg[device_id].adrc_params[4]);
			LOGI("ADRC Filter COMP RELEASE[0] = %02x.", adrc_cfg[device_id].adrc_params[5]);
			LOGI("ADRC Filter COMP RELEASE[1] = %02x.", adrc_cfg[device_id].adrc_params[6]);
			LOGI("ADRC Filter COMP DELAY = %02x.", adrc_cfg[device_id].adrc_params[7]);
			if (ioctl(fd, AUDIO_SET_ADRC, &adrc_cfg[device_id]) < 0)
			{
				LOGE("set adrc filter error.");
	            close(fd);
	            return -EPERM;
			}
    	} else {
			LOGV("ADRC Disabled");
    	}

    	// Apply the EQ filter (if any)
    	if (enable_mask & EQ_ENABLE) {
            LOGI("EQ Filter FLAG = %02x.", eq_flag[device_id]);
            if (ioctl(fd, AUDIO_SET_EQ, &equalizer[device_id]) < 0) {
                LOGE("set Equalizer error.");
                close(fd);
                return -EPERM;
            }
    	} else {
			LOGV("EQ Disabled");
    	}

    	// Apply the RX_IIR filter (if any)
    	if (enable_mask & RX_IIR_ENABLE) {
            LOGI("IIR Filter FLAG = %02x.", rx_iir_flag[device_id]);
            LOGI("IIR NUMBER OF BANDS = %02x.", iir_cfg[device_id].num_bands);
            LOGI("IIR Filter N1  = %02x.", iir_cfg[device_id].iir_params[0]);
            LOGI("IIR Filter N2  = %02x.", iir_cfg[device_id].iir_params[1]);
            LOGI("IIR Filter N3  = %02x.", iir_cfg[device_id].iir_params[2]);
            LOGI("IIR Filter N4  = %02x.", iir_cfg[device_id].iir_params[3]);
            LOGI("IIR FILTER M1  = %02x.", iir_cfg[device_id].iir_params[24]);
            LOGI("IIR FILTER M2  = %02x.", iir_cfg[device_id].iir_params[25]);
            LOGI("IIR FILTER M3  = %02x.", iir_cfg[device_id].iir_params[26]);
            LOGI("IIR FILTER M4  = %02x.", iir_cfg[device_id].iir_params[27]);
            LOGI("IIR FILTER M16 = %02x.", iir_cfg[device_id].iir_params[39]);
            LOGI("IIR FILTER SF1 = %02x.", iir_cfg[device_id].iir_params[40]);
            if (ioctl(fd, AUDIO_SET_RX_IIR, &iir_cfg[device_id]) < 0) {
                LOGE("set rx iir filter error.");
                close(fd);
                return -EPERM;
            }
    	} else {
			LOGV("IIR Disabled");
    	}

    	LOGI("Enabling post proc features with mask 0x%04x", enable_mask);
        if (ioctl(fd, AUDIO_ENABLE_AUDPP, &enable_mask) < 0) {
            LOGE("enable audpp error");
            close(fd);
            return -EPERM;
        }
    } else {
        // Initialize the post processing mask
        disable_mask = 0;

        // Clean the post processing bits
        if (post_proc_feature_mask & MBADRC_ENABLE) disable_mask &= MBADRC_DISABLE;
        if (post_proc_feature_mask & ADRC_ENABLE) disable_mask &= ADRC_DISABLE;
        if (post_proc_feature_mask & EQ_ENABLE) disable_mask &= EQ_DISABLE;
        if (post_proc_feature_mask & RX_IIR_ENABLE) disable_mask &= RX_IIR_DISABLE;

        LOGI("Disabling post proc features with mask 0x%04x", disable_mask);
        if (ioctl(fd, AUDIO_ENABLE_AUDPP, &disable_mask) < 0) {
            LOGE("enable audpp error");
            close(fd);
            return -EPERM;
        }
    }

    close(fd);
    return 0;
}

unsigned int AudioHardware::calculate_audpre_table_index(unsigned index)
{
    switch (index) {
        case 48000:    return SAMP_RATE_INDX_48000;
        case 44100:    return SAMP_RATE_INDX_44100;
        case 32000:    return SAMP_RATE_INDX_32000;
        case 24000:    return SAMP_RATE_INDX_24000;
        case 22050:    return SAMP_RATE_INDX_22050;
        case 16000:    return SAMP_RATE_INDX_16000;
        case 12000:    return SAMP_RATE_INDX_12000;
        case 11025:    return SAMP_RATE_INDX_11025;
        case 8000:     return SAMP_RATE_INDX_8000;
        default:       return -1;
    }
}

AudioStreamOut* AudioHardware::openOutputStream(
        uint32_t devices, int *format, uint32_t *channels, uint32_t *sampleRate, status_t *status)
{
    Mutex::Autolock lock(mLock);

    // only one output stream allowed
    if (mOutput) {
        if (status) {
            *status = INVALID_OPERATION;
        }
        return 0;
    }

    // create new output stream
    AudioStreamOutMSM72xx* out = new AudioStreamOutMSM72xx();
    status_t lStatus = out->set(this, devices, format, channels, sampleRate);
    if (status) {
        *status = lStatus;
    }
    if (lStatus == NO_ERROR) {
        mOutput = out;
    } else {
        delete out;
    }
    return mOutput;
}

void AudioHardware::closeOutputStream(AudioStreamOut* out) {
    Mutex::Autolock lock(mLock);
    if (mOutput == 0 || mOutput != out) {
        LOGW("Attempt to close invalid output stream");
    }
    else {
        delete mOutput;
        mOutput = 0;
    }
}

AudioStreamIn* AudioHardware::openInputStream(
        uint32_t devices, int *format, uint32_t *channels, uint32_t *sampleRate, status_t *status,
        AudioSystem::audio_in_acoustics acoustic_flags)
{
    // check for valid input source
    if (!AudioSystem::isInputDevice((AudioSystem::audio_devices)devices)) {
        return 0;
    }

    if ( (mMode == AudioSystem::MODE_IN_CALL) &&
         (getInputSampleRate(*sampleRate) > AUDIO_HW_IN_SAMPLERATE) &&
         (*format == AUDIO_HW_IN_FORMAT) )
    {
        LOGE("PCM recording, in a voice call, with sample rate more than 8K not supported \
                re-configure with 8K and try software re-sampler ");
        *status = BAD_VALUE;
        *sampleRate = AUDIO_HW_IN_SAMPLERATE;
        return 0;
    }

    mLock.lock();

    AudioStreamInMSM72xx* in = new AudioStreamInMSM72xx();
    status_t lStatus = in->set(this, devices, format, channels, sampleRate, acoustic_flags);
    if (status) {
        *status = lStatus;
    }
    if (lStatus != NO_ERROR) {
        mLock.unlock();
        delete in;
        return 0;
    }

    mInputs.add(in);
    mLock.unlock();

    return in;
}

void AudioHardware::closeInputStream(AudioStreamIn* in) {
    Mutex::Autolock lock(mLock);

    ssize_t index = mInputs.indexOf((AudioStreamInMSM72xx *)in);
    if (index < 0) {
        LOGW("Attempt to close invalid input stream");
    } else {
        mLock.unlock();
        delete mInputs[index];
        mLock.lock();
        mInputs.removeAt(index);
    }
}

status_t AudioHardware::setMode(int mode)
{
    status_t status = AudioHardwareBase::setMode(mode);
    if (status == NO_ERROR) {
        // make sure that doAudioRouteOrMute() is called by doRouting()
        // even if the new device selected is the same as current one.
        clearCurDevice();
    }
    return status;
}

bool AudioHardware::checkOutputStandby()
{
    if (mOutput)
        if (!mOutput->checkStandby())
            return false;

    return true;
}

status_t AudioHardware::getMicMute(bool* state)
{
    *state = mMicMute;
    return NO_ERROR;
}

status_t AudioHardware::setMicMute(bool state)
{
    Mutex::Autolock lock(mLock);
    return setMicMute_nosync(state);
}

// always call with mutex held
status_t AudioHardware::setMicMute_nosync(bool state)
{
    if (mMicMute != state) {
        mMicMute = state;
        return doAudioRouteOrMute(SND_DEVICE_CURRENT);
    }
    return NO_ERROR;
}

status_t AudioHardware::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 value;
    String8 key;
    const char BT_NREC_KEY[] = "bt_headset_nrec";
    const char BT_NAME_KEY[] = "bt_headset_name";
    const char BT_NREC_VALUE_ON[] = "on";

    LOGV("setParameters() %s", keyValuePairs.string());

    if (keyValuePairs.length() == 0) return BAD_VALUE;

    key = String8(BT_NREC_KEY);
    if (param.get(key, value) == NO_ERROR) {
        if (value == BT_NREC_VALUE_ON) {
            mBluetoothNrec = true;
        } else {
            mBluetoothNrec = false;
           LOGI("Turning noise reduction and echo cancellation off for BT "
                 "headset");
        }
    }

    key = String8(BT_NAME_KEY);
    if (param.get(key, value) == NO_ERROR) {
        mBluetoothId = 0;
        for (int i = 0; i < mNumSndEndpoints; i++) {
            if (!strcasecmp(value.string(), mSndEndpoints[i].name)) {
                mBluetoothId = mSndEndpoints[i].id;
                LOGI("Using custom acoustic parameters for %s", value.string());
                break;
            }
        }
        if (mBluetoothId == 0) {
            LOGI("Using default acoustic parameters "
                 "(%s not in acoustic database)", value.string());
            doRouting(NULL);
        }
    }

    key = String8(DUALMIC_KEY);
    if (param.get(key, value) == NO_ERROR) {
        if (value == "true") {
            mDualMicEnabled = true;
            LOGI("DualMike feature Enabled");
        } else {
            mDualMicEnabled = false;
            LOGI("DualMike feature Disabled");
        }
        doRouting(NULL);
    }

    key = String8(TTY_MODE_KEY);
    if (param.get(key, value) == NO_ERROR) {
        if (value == "full") {
            mTtyMode = TTY_FULL;
        } else if (value == "hco") {
            mTtyMode = TTY_HCO;
        } else if (value == "vco") {
            mTtyMode = TTY_VCO;
        } else {
            mTtyMode = TTY_OFF;
        }
        if (mMode != AudioSystem::MODE_IN_CALL){
           return NO_ERROR;
        }
    } else {
        mTtyMode = TTY_OFF;
    }

#ifdef HAVE_FM_RADIO
    key = String8(AudioParameter::keyFmOn);
    int devices;
    if (param.getInt(key, devices) == NO_ERROR) {
       setFmOnOff(true);
    }
    key = String8(AudioParameter::keyFmOff);
    if (param.getInt(key, devices) == NO_ERROR) {
       setFmOnOff(false);
    }
#endif

    doRouting(NULL);

    return NO_ERROR;
}

String8 AudioHardware::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;

    String8 key = String8(DUALMIC_KEY);

    if (param.get(key, value) == NO_ERROR) {
        value = String8(mDualMicEnabled ? "true" : "false");
        param.add(key, value);
    }

    key = String8("tunneled-input-formats");
    if ( param.get(key,value) == NO_ERROR ) {
        param.addInt(String8("AMR"), true );
        param.addInt(String8("QCELP"), true );
        param.addInt(String8("EVRC"), true );
    }
    LOGV("AudioHardware::getParameters() %s", param.toString().string());
    return param.toString();
}

size_t AudioHardware::getInputBufferSize(uint32_t sampleRate, int format, int channelCount)
{
    if ( (format != AudioSystem::PCM_16_BIT) &&
         (format != AudioSystem::AMR_NB)     &&
         (format != AudioSystem::AAC)){
        LOGW("getInputBufferSize bad format: 0x%x", format);
        return 0;
    }
    if (channelCount < 1 || channelCount > 2) {
        LOGW("getInputBufferSize bad channel count: %d", channelCount);
        return 0;
    }

    if (format == AudioSystem::AMR_NB)
       return 320*channelCount;
    else if (format == AudioSystem::AAC)
       return 2048;
    else
       return 2048*channelCount;
}

char * AudioHardware::get_sound_device(int32_t device) {
	char * str_device;

	if (device == SND_DEVICE_CURRENT) {
		str_device = (char *) "SND_DEVICE_CURRENT";
	} else if (device == SND_DEVICE_HANDSET) {
		str_device = (char *) "SND_DEVICE_HANDSET";
	} else if (device == SND_DEVICE_SPEAKER) {
		str_device = (char *) "SND_DEVICE_SPEAKER";
	} else if (device == SND_DEVICE_MEDIA_SPEAKER) {
		str_device = (char *) "SND_DEVICE_MEDIA_SPEAKER";
	} else if (device == SND_DEVICE_BT) {
		str_device = (char *) "SND_DEVICE_BT";
	} else if (device == SND_DEVICE_BT_EC_OFF) {
		str_device = (char *) "SND_DEVICE_BT_EC_OFF";
	} else if (device == SND_DEVICE_HEADSET) {
		str_device = (char *) "SND_DEVICE_HEADSET";
	} else if (device == SND_DEVICE_HEADSET_AND_SPEAKER) {
		str_device = (char *) "SND_DEVICE_HEADSET_AND_SPEAKER";
	} else if (device == SND_DEVICE_IN_S_SADC_OUT_HANDSET) {
		str_device = (char *) "SND_DEVICE_IN_S_SADC_OUT_HANDSET";
	} else if (device == SND_DEVICE_IN_S_SADC_OUT_SPEAKER_PHONE) {
		str_device = (char *) "SND_DEVICE_IN_S_SADC_OUT_SPEAKER_PHONE";
	} else if (device == SND_DEVICE_TTY_HEADSET) {
		str_device = (char *) "SND_DEVICE_TTY_HEADSET";
	} else if (device == SND_DEVICE_TTY_HCO) {
		str_device = (char *) "SND_DEVICE_TTY_HCO";
	} else if (device == SND_DEVICE_TTY_VCO) {
		str_device = (char *) "SND_DEVICE_TTY_VCO";
	} else if (device == SND_DEVICE_CARKIT) {
		str_device = (char *) "SND_DEVICE_CARKIT";
	} else if (device == SND_DEVICE_FM_SPEAKER) {
		str_device = (char *) "SND_DEVICE_FM_SPEAKER";
	} else if (device == SND_DEVICE_FM_HEADSET) {
		str_device = (char *) "SND_DEVICE_FM_HEADSET";
	} else if (device == SND_DEVICE_NO_MIC_HEADSET) {
		str_device = (char *) "SND_DEVICE_NO_MIC_HEADSET";
	} else {
		str_device = (char *) "UNKNOWN";
	}
	return str_device;
}

status_t AudioHardware::set_volume_rpc(int32_t device,
                                       uint32_t method,
                                       uint32_t volume)
{
#if LOG_SND_RPC
    LOGD("rpc_snd_set_volume(%s, %d, %d)", get_sound_device(device), method, volume);
#endif

    if (device == -1) return NO_ERROR;

    /*
     * rpc_snd_set_volume(
     *     device,              # Any hardware device enum, including
     *                          # SND_DEVICE_CURRENT
     *     method,              # must be SND_METHOD_VOICE to do anything useful
     *     volume,              # integer volume level, in range [0,5].
     *                          # note that 0 is audible (not quite muted)
     * )
     * rpc_snd_set_volume only works for in-call sound volume.
     */

     struct msm_snd_volume_config args;

     args.device = device;
     args.method = method;
     args.volume = volume;

     if (ioctl(m7xsnddriverfd, SND_SET_VOLUME, &args) < 0) {
         LOGE("snd_set_volume error.");
         return -EIO;
     }
     return NO_ERROR;
}

status_t AudioHardware::setVoiceVolume(float v)
{
    if (v < 0.0) {
        LOGW("setVoiceVolume(%f) under 0.0, assuming 0.0", v);
        v = 0.0;
    } else if (v > 1.0) {
        LOGW("setVoiceVolume(%f) over 1.0, assuming 1.0", v);
        v = 1.0;
    }

    int vol = lrint(v * 5.0);
    LOGD("setVoiceVolume(%f)", v);
    LOGI("Setting in-call volume to %d (available range is 0 to 5)", vol);

    if ((mCurSndDevice != -1) && ((mCurSndDevice == SND_DEVICE_TTY_HEADSET) || (mCurSndDevice == SND_DEVICE_TTY_VCO)))
    {
        vol = 1;
        LOGI("For TTY device in FULL or VCO mode, the volume level is set to: %d", vol);
    }

    Mutex::Autolock lock(mLock);
    set_volume_rpc(SND_DEVICE_CURRENT, SND_METHOD_VOICE, vol);
    return NO_ERROR;
}

status_t AudioHardware::setMasterVolume(float v)
{
    Mutex::Autolock lock(mLock);
    int vol = ceil(v * 5.0);
    LOGI("Set master volume to %d.", vol);
    set_volume_rpc(SND_DEVICE_HANDSET, SND_METHOD_VOICE, vol);
    set_volume_rpc(SND_DEVICE_SPEAKER, SND_METHOD_VOICE, vol);
    set_volume_rpc(SND_DEVICE_BT,      SND_METHOD_VOICE, vol);
    set_volume_rpc(SND_DEVICE_HEADSET, SND_METHOD_VOICE, vol);
    set_volume_rpc(SND_DEVICE_IN_S_SADC_OUT_HANDSET, SND_METHOD_VOICE, vol);
    set_volume_rpc(SND_DEVICE_IN_S_SADC_OUT_SPEAKER_PHONE, SND_METHOD_VOICE, vol);
    set_volume_rpc(SND_DEVICE_MEDIA_SPEAKER, SND_METHOD_VOICE, vol);
    set_volume_rpc(SND_DEVICE_TTY_HEADSET, SND_METHOD_VOICE, 1);
    set_volume_rpc(SND_DEVICE_TTY_VCO, SND_METHOD_VOICE, 1);
    // We return an error code here to let the audioflinger do in-software
    // volume on top of the maximum volume that we set through the SND API.
    // return error - software mixer will handle it
    return -1;
}

#ifdef HAVE_FM_RADIO
status_t AudioHardware::setFmOnOff(bool onoff)
{
    if (onoff) {
        mFmRadioEnabled = true;
    } else {
        mFmRadioEnabled = false;
    }

    return NO_ERROR;
}
#endif

status_t AudioHardware::do_route_audio_rpc(int32_t device,
                                           bool ear_mute,
                                           bool mic_mute)
{
    if (device == -1)
        return NO_ERROR;

#if LOG_SND_RPC
    LOGD("rpc_snd_set_device(%s, %d, %d)", get_sound_device(device), ear_mute, mic_mute);
#endif

    // RPC call to switch audio path

    /*
     * rpc_snd_set_device(
     *     device,              # Hardware device enum to use
     *     ear_mute,            # Set mute for outgoing voice audio
     *                          # this should only be unmuted when in-call
     *     mic_mute,            # Set mute for incoming voice audio
     *                          # this should only be unmuted when in-call or
     *                          # recording.
     *  )
     */
    
    // Inclusion for extamp
    struct msm_snd_extamp_config args2;
    args2.device=device;
    if (device == SND_DEVICE_HEADSET || device == SND_DEVICE_FM_HEADSET || device == SND_DEVICE_NO_MIC_HEADSET) {
        args2.speaker_volume=0;
        if (device == SND_DEVICE_NO_MIC_HEADSET) {
            args2.headset_volume=26;
        } else if (device == SND_DEVICE_FM_HEADSET) {
            args2.headset_volume=20;
        } else {
            args2.headset_volume=18;
        }
    }
    else {
        args2.speaker_volume=29;
        args2.headset_volume=0;
    }
    char extampOn[PROPERTY_VALUE_MAX];
    property_get("persist.sys.extamp-filter", extampOn, "0");
    if (strcmp(extampOn, "1") == 0) {
        if (ioctl(m7xsnddriverfd, SND_SET_EXTAMP, &args2) < 0) {
            LOGE("snd_set_extamp error.");
            return -EIO;
        }
    }
    // End of extamp    
    struct msm_snd_device_config args;

    args.device = device;
    args.ear_mute = ear_mute ? SND_MUTE_MUTED : SND_MUTE_UNMUTED;
    if ((device != SND_DEVICE_CURRENT) && (!mic_mute)) {
        //Explicitly mute the mic to release DSP resources
        args.mic_mute = SND_MUTE_MUTED;
        if (ioctl(m7xsnddriverfd, SND_SET_DEVICE, &args) < 0) {
            LOGE("snd_set_device error.");
            return -EIO;
        }
    }
    args.mic_mute = mic_mute ? SND_MUTE_MUTED : SND_MUTE_UNMUTED;

    if (ioctl(m7xsnddriverfd, SND_SET_DEVICE, &args) < 0) {
        LOGE("snd_set_device error.");
        return -EIO;
    }
    // Inclusion for set_volume
    //set_volume_rpc(SND_DEVICE_CURRENT, 0, 6);
    // End of extamp

    return NO_ERROR;
}

// always call with mutex held
status_t AudioHardware::doAudioRouteOrMute(int32_t device)
{
    // QCOM caveat: Audio will be routed to speaker if device=handset and mute=true
    // Also, the audio circuit causes battery drain unless mute=true
    // Android < 2.0 uses MODE_IN_CALL for routing audio to earpiece
    // Android >= 2.0 advises to use STREAM_VOICE_CALL streams and setSpeakerphoneOn()
    // Android >= 2.3 uses MODE_IN_COMMUNICATION for SIP calls
    bool mute = !isInCall();
    if (mute && (device == SND_DEVICE_HANDSET)) {
        //workaround to emulate Android >= 2.0 behaviour
        //enable routing to earpiece (unmute) if mic is selected as input
        mute = !mBuiltinMicSelected;
    }

    mFmPrev=mFmRadioEnabled;
#ifdef HAVE_FM_RADIO
    if (mFmRadioEnabled) {
        mute = 0;
        LOGI("unmute for radio");
    }
#endif
    LOGD("doAudioRouteOrMute() device %s, mMode %d, mMicMute %d, mBuiltinMicSelected %d, %s",
        get_sound_device(device), mMode, mMicMute, mBuiltinMicSelected, mute ? "muted" : "audio circuit active");
    return do_route_audio_rpc(device, mute, mMicMute);
}

status_t AudioHardware::doRouting(AudioStreamInMSM72xx *input)
{
    Mutex::Autolock lock(mLock);
    uint32_t outputDevices = mOutput->devices();
    status_t ret = NO_ERROR;
    int new_snd_device = -1;
    int new_post_proc_feature_mask = 0;

    if (input != NULL) {
        uint32_t inputDevice = input->devices();
        LOGI("do input routing device %x", inputDevice);
        mBuiltinMicSelected = (inputDevice == AudioSystem::DEVICE_IN_BUILTIN_MIC);
        // ignore routing device information when we start a recording in voice
        // call
        // Recording will happen through currently active tx device
        if (inputDevice == AudioSystem::DEVICE_IN_VOICE_CALL)
            return NO_ERROR;
        if (inputDevice != 0) {
            if (inputDevice & AudioSystem::DEVICE_IN_BLUETOOTH_SCO_HEADSET) {
                LOGI("Routing audio to Bluetooth PCM");
                new_snd_device = SND_DEVICE_BT;
            } else if (inputDevice & AudioSystem::DEVICE_IN_WIRED_HEADSET) {
                LOGI("Routing audio to Wired Headset");
                new_snd_device = SND_DEVICE_HEADSET;
            } else {
                if (outputDevices & AudioSystem::DEVICE_OUT_SPEAKER) {
                    LOGI("Routing audio to Speakerphone");
                    new_snd_device = SND_DEVICE_SPEAKER;
                    new_post_proc_feature_mask = (ADRC_ENABLE | EQ_ENABLE | RX_IIR_ENABLE | MBADRC_ENABLE);
                } else {
                    LOGI("Routing audio to Handset");
                    new_snd_device = SND_DEVICE_HANDSET;
                }
            }
        }
    }

    // if inputDevice == 0, restore output routing
    if (new_snd_device == -1) {
        if (outputDevices & (outputDevices - 1)) {
            if ((outputDevices & AudioSystem::DEVICE_OUT_SPEAKER) == 0) {
                LOGW("Hardware does not support requested route combination (%#X),"
                     " picking closest possible route...", outputDevices);
            }
        }

        if ((mTtyMode != TTY_OFF) && (mMode == AudioSystem::MODE_IN_CALL) &&
                (outputDevices & AudioSystem::DEVICE_OUT_WIRED_HEADSET)) {
            if (mTtyMode == TTY_FULL) {
                LOGI("Routing audio to TTY FULL Mode");
                new_snd_device = SND_DEVICE_TTY_HEADSET;
            } else if (mTtyMode == TTY_VCO) {
                LOGI("Routing audio to TTY VCO Mode");
                new_snd_device = SND_DEVICE_TTY_VCO;
            } else if (mTtyMode == TTY_HCO) {
                LOGI("Routing audio to TTY HCO Mode");
                new_snd_device = SND_DEVICE_TTY_HCO;
            }
        } else if (outputDevices &
                   (AudioSystem::DEVICE_OUT_BLUETOOTH_SCO | AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_HEADSET)) {
            LOGI("Routing audio to Bluetooth PCM");
            new_snd_device = SND_DEVICE_BT;
        } else if (outputDevices & AudioSystem::DEVICE_OUT_BLUETOOTH_SCO_CARKIT) {
            LOGI("Routing audio to Bluetooth PCM");
            new_snd_device = SND_DEVICE_CARKIT;
#ifdef COMBO_DEVICE_SUPPORTED
        } else if ((outputDevices & AudioSystem::DEVICE_OUT_WIRED_HEADSET) &&
                   (outputDevices & AudioSystem::DEVICE_OUT_SPEAKER)) {
            LOGI("Routing audio to Wired Headset and Speaker");
            new_snd_device = SND_DEVICE_HEADSET_AND_SPEAKER;
            new_post_proc_feature_mask = (ADRC_ENABLE | EQ_ENABLE | RX_IIR_ENABLE | MBADRC_ENABLE);
        } else if (outputDevices & AudioSystem::DEVICE_OUT_WIRED_HEADPHONE) {
            if (outputDevices & AudioSystem::DEVICE_OUT_SPEAKER) {
                if (mFmRadioEnabled) {
                    LOGI("Routing audio to FM Speakerphone");
                    new_snd_device = SND_DEVICE_FM_SPEAKER;
                    new_post_proc_feature_mask = (EQ_ENABLE | RX_IIR_ENABLE);
                    new_post_proc_feature_mask &= (MBADRC_DISABLE | ADRC_DISABLE);
                } else {
                    LOGI("Routing audio to No microphone Wired Headset and Speaker (%d,%x)", mMode, outputDevices);
                    new_snd_device = SND_DEVICE_HEADSET_AND_SPEAKER;
                    new_post_proc_feature_mask = (ADRC_ENABLE | EQ_ENABLE | RX_IIR_ENABLE | MBADRC_ENABLE);
                }
            } else {
                if (mFmRadioEnabled) {
                    LOGI("Routing audio to FM Headset");
                    new_snd_device = SND_DEVICE_FM_HEADSET;
                    new_post_proc_feature_mask = (EQ_ENABLE | RX_IIR_ENABLE);
                    new_post_proc_feature_mask &= (MBADRC_DISABLE | ADRC_DISABLE);
                } else {
                    LOGI("Routing audio to No microphone Wired Headset (%d,%x)", mMode, outputDevices);
                    new_snd_device = SND_DEVICE_NO_MIC_HEADSET;
                }
            }
#endif
        } else if (outputDevices & AudioSystem::DEVICE_OUT_WIRED_HEADSET) {
            if (mFmRadioEnabled) {
                LOGI("Routing FM audio to Wired Headset");
                new_snd_device = SND_DEVICE_FM_HEADSET;
                new_post_proc_feature_mask = (EQ_ENABLE | RX_IIR_ENABLE);
                new_post_proc_feature_mask &= (MBADRC_DISABLE | ADRC_DISABLE);
            } else {
                LOGI("Routing audio to Wired Headset");
                new_snd_device = SND_DEVICE_HEADSET;
                new_post_proc_feature_mask = (ADRC_ENABLE | EQ_ENABLE | RX_IIR_ENABLE | MBADRC_ENABLE);
            }
        } else if (outputDevices & AudioSystem::DEVICE_OUT_WIRED_HEADPHONE) {
            if (mFmRadioEnabled) {
                LOGI("Routing audio to FM Headset");
                new_snd_device = SND_DEVICE_FM_HEADSET;
                new_post_proc_feature_mask = (EQ_ENABLE | RX_IIR_ENABLE);
                new_post_proc_feature_mask &= (MBADRC_DISABLE | ADRC_DISABLE);
            } else {
                LOGI("Routing audio to Wired Headset");
                new_snd_device = SND_DEVICE_HEADSET;
                new_post_proc_feature_mask = (ADRC_ENABLE | EQ_ENABLE | RX_IIR_ENABLE | MBADRC_ENABLE);
            }
        } else if (outputDevices & AudioSystem::DEVICE_OUT_SPEAKER) {
            if (mFmRadioEnabled) {
                LOGI("Routing audio to FM Speakerphone");
                new_snd_device = SND_DEVICE_FM_SPEAKER;
                new_post_proc_feature_mask = (EQ_ENABLE | RX_IIR_ENABLE);
                new_post_proc_feature_mask &= (MBADRC_DISABLE | ADRC_DISABLE);
            } else {
                LOGI("Routing audio to Speakerphone");
                new_snd_device = SND_DEVICE_MEDIA_SPEAKER;
                new_post_proc_feature_mask = (ADRC_ENABLE | EQ_ENABLE | RX_IIR_ENABLE | MBADRC_ENABLE);
            }
        } else {
            LOGI("Routing audio to Handset");
            new_snd_device = SND_DEVICE_HANDSET;
            new_post_proc_feature_mask = (ADRC_ENABLE | EQ_ENABLE | RX_IIR_ENABLE | MBADRC_ENABLE);
        }
    }

    if (mDualMicEnabled && mMode == AudioSystem::MODE_IN_CALL) {
        if (new_snd_device == SND_DEVICE_HANDSET) {
            LOGI("Routing audio to handset with DualMike enabled");
            new_snd_device = SND_DEVICE_IN_S_SADC_OUT_HANDSET;
        } else if (new_snd_device == SND_DEVICE_SPEAKER) {
            LOGI("Routing audio to speakerphone with DualMike enabled");
            new_snd_device = SND_DEVICE_IN_S_SADC_OUT_SPEAKER_PHONE;
        }
    }

    if ((new_snd_device != -1) && ((new_snd_device != mCurSndDevice) || (mFmRadioEnabled != mFmPrev))) {
        ret = doAudioRouteOrMute(new_snd_device);

        //disable post proc first for previous session
        if (playback_in_progress)
           msm72xx_enable_postproc(false);

        //save the device id (active and current)
        mActSndDevice = new_snd_device;
        mCurSndDevice = new_snd_device;

       //enable post proc for new device
       post_proc_feature_mask = new_post_proc_feature_mask;

       if (playback_in_progress)
           msm72xx_enable_postproc(true);
    }

    return ret;
}

status_t AudioHardware::checkMicMute()
{
    Mutex::Autolock lock(mLock);
    if (mMode != AudioSystem::MODE_IN_CALL) {
        setMicMute_nosync(true);
    }

    return NO_ERROR;
}

status_t AudioHardware::dumpInternals(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    result.append("AudioHardware::dumpInternals\n");
    snprintf(buffer, SIZE, "\tmInit: %s\n", mInit? "true": "false");
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmMicMute: %s\n", mMicMute? "true": "false");
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmBluetoothNrec: %s\n", mBluetoothNrec? "true": "false");
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmBluetoothId: %d\n", mBluetoothId);
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

status_t AudioHardware::dump(int fd, const Vector<String16>& args)
{
    dumpInternals(fd, args);
    for (size_t index = 0; index < mInputs.size(); index++) {
        mInputs[index]->dump(fd, args);
    }

    if (mOutput) {
        mOutput->dump(fd, args);
    }
    return NO_ERROR;
}

uint32_t AudioHardware::getInputSampleRate(uint32_t sampleRate)
{
    uint32_t i;
    uint32_t prevDelta;
    uint32_t delta;

    for (i = 0, prevDelta = 0xFFFFFFFF; i < sizeof(inputSamplingRates)/sizeof(uint32_t); i++, prevDelta = delta) {
        delta = abs(sampleRate - inputSamplingRates[i]);
        if (delta > prevDelta) break;
    }
    // i is always > 0 here
    return inputSamplingRates[i-1];
}

// getActiveInput_l() must be called with mLock held
AudioHardware::AudioStreamInMSM72xx *AudioHardware::getActiveInput_l()
{
    for (size_t i = 0; i < mInputs.size(); i++) {
        // return first input found not being in standby mode
        // as only one input can be in this state
        if (mInputs[i]->state() > AudioStreamInMSM72xx::AUDIO_INPUT_CLOSED) {
            return mInputs[i];
        }
    }

    return NULL;
}

// ----------------------------------------------------------------------------

AudioHardware::AudioStreamOutMSM72xx::AudioStreamOutMSM72xx() :
    mHardware(0), mFd(-1), mStartCount(0), mRetryCount(0), mStandby(true), mDevices(0)
{
}

status_t AudioHardware::AudioStreamOutMSM72xx::set(
        AudioHardware* hw, uint32_t devices, int *pFormat, uint32_t *pChannels, uint32_t *pRate)
{
    int lFormat = pFormat ? *pFormat : 0;
    uint32_t lChannels = pChannels ? *pChannels : 0;
    uint32_t lRate = pRate ? *pRate : 0;

    mHardware = hw;

    // fix up defaults
    if (lFormat == 0) lFormat = format();
    if (lChannels == 0) lChannels = channels();
    if (lRate == 0) lRate = sampleRate();

    // check values
    if ((lFormat != format()) ||
        (lChannels != channels()) ||
        (lRate != sampleRate())) {
        if (pFormat) *pFormat = format();
        if (pChannels) *pChannels = channels();
        if (pRate) *pRate = sampleRate();
        return BAD_VALUE;
    }

    if (pFormat) *pFormat = lFormat;
    if (pChannels) *pChannels = lChannels;
    if (pRate) *pRate = lRate;

    mDevices = devices;

    return NO_ERROR;
}

AudioHardware::AudioStreamOutMSM72xx::~AudioStreamOutMSM72xx()
{
    if (mFd >= 0) close(mFd);
}

void AudioHardware::AudioStreamOutMSM72xx::close_stream(size_t count) {
    if (mFd >= 0) {
        ::close(mFd);
        mFd = -1;
    }

    // Simulate audio output timing in case of error
    usleep(count * 1000000 / frameSize() / sampleRate());
}

ssize_t AudioHardware::AudioStreamOutMSM72xx::write(const void* buffer, size_t bytes)
{
    // LOGD("AudioStreamOutMSM72xx::write(%p, %u)", buffer, bytes);
    status_t status = NO_INIT;
    size_t count = bytes;
    const uint8_t* p = static_cast<const uint8_t*>(buffer);

    if (mStandby) {
        // open driver
        LOGV("open driver");
        status = ::open("/dev/msm_pcm_out", O_RDWR);

        // check status
        if (status < 0) { LOGE("Cannot open /dev/msm_pcm_out errno: %d", errno); close_stream(bytes); return status; }

        mFd = status;

        // configuration
        LOGV("get config");
        struct msm_audio_config config;
        status = ioctl(mFd, AUDIO_GET_CONFIG, &config);

        // check status
        if (status < 0) { LOGE("Cannot read config"); close_stream(bytes); return status; }

        LOGV("set config");
        config.channel_count = AudioSystem::popCount(channels());
        config.sample_rate = sampleRate();
        config.buffer_size = bufferSize();
        config.buffer_count = AUDIO_HW_NUM_OUT_BUF;
        config.type = CODEC_TYPE_PCM;
        status = ioctl(mFd, AUDIO_SET_CONFIG, &config);

        // check status
        if (status < 0) { LOGE("Cannot set config"); close_stream(bytes); return status; }

        LOGV("buffer_size: %u", config.buffer_size);
        LOGV("buffer_count: %u", config.buffer_count);
        LOGV("channel_count: %u", config.channel_count);
        LOGV("sample_rate: %u", config.sample_rate);

        // fill 2 buffers before AUDIO_START
        mStartCount = AUDIO_HW_NUM_OUT_BUF;
        mStandby = false;
    }

    while (count) {
        ssize_t written = ::write(mFd, p, count);
        if (written >= 0) {
            count -= written;
            p += written;
        } else {
            if (errno != EAGAIN) return written;
            mRetryCount++;
            LOGW("EAGAIN - retry");
        }
    }

    // start audio after we fill 2 buffers
    if (mStartCount) {
        if (--mStartCount == 0) {
            ioctl(mFd, AUDIO_START, 0);
            mHardware->playback_in_progress = true;

            //enable post processing
            mHardware->msm72xx_enable_postproc(true);
        }
    }
    return bytes;
}

status_t AudioHardware::AudioStreamOutMSM72xx::standby()
{
    status_t status = NO_ERROR;
    if (!mStandby && mFd >= 0) {
        //disable post processing
        mHardware->msm72xx_enable_postproc(false);
        mHardware->playback_in_progress = false;
        ::close(mFd);
        mFd = -1;
    }
    mStandby = true;
    return status;
}

status_t AudioHardware::AudioStreamOutMSM72xx::dump(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    result.append("AudioStreamOutMSM72xx::dump\n");
    snprintf(buffer, SIZE, "\tsample rate: %d\n", sampleRate());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tbuffer size: %d\n", bufferSize());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tchannels: %d\n", channels());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tformat: %d\n", format());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmHardware: %p\n", mHardware);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmFd: %d\n", mFd);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmStartCount: %d\n", mStartCount);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmRetryCount: %d\n", mRetryCount);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmStandby: %s\n", mStandby? "true": "false");
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

bool AudioHardware::AudioStreamOutMSM72xx::checkStandby()
{
    return mStandby;
}


status_t AudioHardware::AudioStreamOutMSM72xx::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key = String8(AudioParameter::keyRouting);
    status_t status = NO_ERROR;
    int device;

    LOGV("AudioStreamOutMSM72xx::setParameters() %s", keyValuePairs.string());

    if (param.getInt(key, device) == NO_ERROR) {
        mDevices = device;
        LOGV("set output routing %x", mDevices);
        status = mHardware->setParameters(keyValuePairs);
        status = mHardware->doRouting(NULL);
        param.remove(key);
    }

    if (param.size()) {
        status = BAD_VALUE;
    }
    return status;
}

String8 AudioHardware::AudioStreamOutMSM72xx::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;
    String8 key = String8(AudioParameter::keyRouting);

    if (param.get(key, value) == NO_ERROR) {
        LOGV("get routing %x", mDevices);
        param.addInt(key, (int)mDevices);
    }

    LOGV("AudioStreamOutMSM72xx::getParameters() %s", param.toString().string());
    return param.toString();
}

status_t AudioHardware::AudioStreamOutMSM72xx::getRenderPosition(uint32_t *dspFrames)
{
    //TODO: enable when supported by driver
    return INVALID_OPERATION;
}

// ----------------------------------------------------------------------------

AudioHardware::AudioStreamInMSM72xx::AudioStreamInMSM72xx() :
    mHardware(0), mFd(-1), mState(AUDIO_INPUT_CLOSED), mRetryCount(0),
    mFormat(AUDIO_HW_IN_FORMAT), mChannels(AUDIO_HW_IN_CHANNELS),
    mSampleRate(AUDIO_HW_IN_SAMPLERATE), mBufferSize(AUDIO_HW_IN_BUFFERSIZE),
    mAcoustics((AudioSystem::audio_in_acoustics)0), mDevices(0)
{
}


void AudioHardware::AudioStreamInMSM72xx::close_stream(void) {
    if (mFd >= 0) {
        ::close(mFd);
        mFd = -1;
    }
}

status_t AudioHardware::AudioStreamInMSM72xx::set(
        AudioHardware* hw, uint32_t devices, int *pFormat, uint32_t *pChannels, uint32_t *pRate,
        AudioSystem::audio_in_acoustics acoustic_flags)
{
    if ((pFormat == 0) ||
        ((*pFormat != AUDIO_HW_IN_FORMAT) &&
         (*pFormat != AudioSystem::AMR_NB) &&
         (*pFormat != AudioSystem::AAC)))
    {
        *pFormat = AUDIO_HW_IN_FORMAT;
        LOGE("audio format bad value");
        return BAD_VALUE;
    }
    if (pRate == 0) {
        return BAD_VALUE;
    }
    uint32_t rate = hw->getInputSampleRate(*pRate);
    if (rate != *pRate) {
        *pRate = rate;
        return BAD_VALUE;
    }

    if (pChannels == 0 || (*pChannels & (AudioSystem::CHANNEL_IN_MONO | AudioSystem::CHANNEL_IN_STEREO)) == 0)
    {
        *pChannels = AUDIO_HW_IN_CHANNELS;
        return BAD_VALUE;
    }

    mHardware = hw;

    LOGV("AudioStreamInMSM72xx::set(%d, %d, %u)", *pFormat, *pChannels, *pRate);
    if (mFd >= 0) {
        LOGE("Audio record already open");
        return -EPERM;
    }

    struct msm_audio_config config;
    struct msm_audio_voicememo_config gcfg;
    memset(&gcfg,0,sizeof(gcfg));
    status_t status = 0;

    if (*pFormat == AUDIO_HW_IN_FORMAT)
    {
        // open audio input device
        status = ::open(PCM_IN_DEVICE, O_RDWR);
        if (status < 0) { LOGE("Cannot open %s errno: %d", PCM_IN_DEVICE, errno); close_stream(); return status; };

        // get the file handle
        mFd = status;

        // configuration
        status = ioctl(mFd, AUDIO_GET_CONFIG, &config);
        if (status < 0) { LOGE("Cannot read config"); close_stream(); return status; }

        LOGV("set config");
        config.channel_count = AudioSystem::popCount(*pChannels);
        config.sample_rate = *pRate;
        config.buffer_size = bufferSize();
        config.buffer_count = 2;
        config.type = CODEC_TYPE_PCM;

        status = ioctl(mFd, AUDIO_SET_CONFIG, &config);
        if (status < 0) {
            if (ioctl(mFd, AUDIO_GET_CONFIG, &config) == 0) {
                if (config.channel_count == 1) {
                    *pChannels = AudioSystem::CHANNEL_IN_MONO;
                } else {
                    *pChannels = AudioSystem::CHANNEL_IN_STEREO;
                }
                *pRate = config.sample_rate;
            }
            LOGE("Cannot set config");
            close_stream();
            return status;
        }

        LOGV("confirm config");

        status = ioctl(mFd, AUDIO_GET_CONFIG, &config);
        if (status < 0) { LOGE("Cannot read config"); close_stream(); return status; }

        LOGV("buffer_size: %u", config.buffer_size);
        LOGV("buffer_count: %u", config.buffer_count);
        LOGV("channel_count: %u", config.channel_count);
        LOGV("sample_rate: %u", config.sample_rate);

        mDevices = devices;
        mFormat = AUDIO_HW_IN_FORMAT;
        mChannels = *pChannels;
        mSampleRate = config.sample_rate;
        mBufferSize = config.buffer_size;
    }
    else if (*pFormat == AudioSystem::AMR_NB) {
        // open vocie memo input device
        status = ::open(VOICE_MEMO_DEVICE, O_RDWR);
        if (status < 0) { LOGE("Cannot open Voice Memo device for read"); close_stream(); return status; }

        // get the file handle
        mFd = status;

        // Config param
        status = ioctl(mFd, AUDIO_GET_CONFIG, &config);
        if (status) { LOGE(" Error getting buf config param AUDIO_GET_CONFIG"); close_stream(); return -status; }

        LOGV("The Config buffer size is %d", config.buffer_size);
        LOGV("The Config buffer count is %d", config.buffer_count);
        LOGV("The Config Channel count is %d", config.channel_count);
        LOGV("The Config Sample rate is %d", config.sample_rate);

        mDevices = devices;
        mChannels = *pChannels;
        mSampleRate = config.sample_rate;

        if (mDevices == AudioSystem::DEVICE_IN_VOICE_CALL) {
            if ((mChannels & AudioSystem::CHANNEL_IN_VOICE_DNLINK) &&
                (mChannels & AudioSystem::CHANNEL_IN_VOICE_UPLINK)) {
                LOGI("Recording Source: Voice Call Both Uplink and Downlink");
                gcfg.rec_type = RPC_VOC_REC_BOTH;
            } else if (mChannels & AudioSystem::CHANNEL_IN_VOICE_DNLINK) {
                LOGI("Recording Source: Voice Call DownLink");
                gcfg.rec_type = RPC_VOC_REC_FORWARD;
            } else if (mChannels & AudioSystem::CHANNEL_IN_VOICE_UPLINK) {
                LOGI("Recording Source: Voice Call UpLink");
                gcfg.rec_type = RPC_VOC_REC_REVERSE;
            }
        }
        else {
            LOGI("Recording Source: Mic/Headset");
            gcfg.rec_type = RPC_VOC_REC_REVERSE;
        }

        gcfg.rec_interval_ms = 0; // AV sync
        gcfg.auto_stop_ms = 0;

        switch (*pFormat)
        {
            case AudioSystem::AMR_NB:
                LOGI("Recording Format: AMR_NB");
                gcfg.capability = RPC_VOC_CAP_AMR; // RPC_VOC_CAP_AMR (64)
                gcfg.max_rate = RPC_VOC_AMR_RATE_1220; // Max rate (Fixed frame)
                gcfg.min_rate = RPC_VOC_AMR_RATE_1220; // Min rate (Fixed frame length)
                gcfg.frame_format = RPC_VOC_PB_AMR; // RPC_VOC_PB_AMR
                mFormat = AudioSystem::AMR_NB;
                mBufferSize = 320;
                break;

            default:
                break;
        }

        gcfg.dtx_enable = 0;
        gcfg.data_req_ms = 20;

        // Set Via  config param
        status = ioctl(mFd, AUDIO_SET_VOICEMEMO_CONFIG, &gcfg);
        if (status) { LOGE("Error: AUDIO_SET_VOICEMEMO_CONFIG failed"); close_stream(); return -status; }

        status = ioctl(mFd, AUDIO_GET_VOICEMEMO_CONFIG, &gcfg);
        if (status) { LOGE("Error: AUDIO_GET_VOICEMEMO_CONFIG failed"); close_stream(); return -status; }

        LOGV("After set rec_type = 0x%8x",gcfg.rec_type);
        LOGV("After set rec_interval_ms = 0x%8x",gcfg.rec_interval_ms);
        LOGV("After set auto_stop_ms = 0x%8x",gcfg.auto_stop_ms);
        LOGV("After set capability = 0x%8x",gcfg.capability);
        LOGV("After set max_rate = 0x%8x",gcfg.max_rate);
        LOGV("After set min_rate = 0x%8x",gcfg.min_rate);
        LOGV("After set frame_format = 0x%8x",gcfg.frame_format);
        LOGV("After set dtx_enable = 0x%8x",gcfg.dtx_enable);
        LOGV("After set data_req_ms = 0x%8x",gcfg.data_req_ms);
    } else if (*pFormat == AudioSystem::AAC) {
        // open AAC input device
        status = ::open(PCM_IN_DEVICE, O_RDWR);
        if (status < 0) { LOGE("Cannot open AAC input  device for read"); close_stream(); return -status; }

        // get the file handle
        mFd = status;

        // Config param
        status = ioctl(mFd, AUDIO_GET_CONFIG, &config);
        if (status) { LOGE(" Error getting buf config param AUDIO_GET_CONFIG"); close_stream(); return -status; }

        LOGV("The Config buffer size is %d", config.buffer_size);
        LOGV("The Config buffer count is %d", config.buffer_count);
        LOGV("The Config Channel count is %d", config.channel_count);
        LOGV("The Config Sample rate is %d", config.sample_rate);

        mDevices = devices;
        mChannels = *pChannels;
        mSampleRate = *pRate;
        mBufferSize = 2048;
        mFormat = *pFormat;

        config.channel_count = AudioSystem::popCount(*pChannels);
        config.sample_rate = *pRate;
        config.type = 1; // Configuring PCM_IN_DEVICE to AAC format

        status = ioctl(mFd, AUDIO_SET_CONFIG, &config);
        if (status) { LOGE(" Error in setting config of msm_pcm_in device"); close_stream(); return -status; }
    }

    //mHardware->setMicMute_nosync(false);
    mState = AUDIO_INPUT_OPENED;

    mHardware->audpre_index = mHardware->calculate_audpre_table_index(mSampleRate);
    if (mHardware->audpre_index < 0) { LOGE("wrong sampling rate"); close_stream(); return -EINVAL; }

    return NO_ERROR;
}

AudioHardware::AudioStreamInMSM72xx::~AudioStreamInMSM72xx()
{
    LOGV("AudioStreamInMSM72xx destructor");
    standby();
}

ssize_t AudioHardware::AudioStreamInMSM72xx::read( void* buffer, ssize_t bytes)
{
    LOGV("AudioStreamInMSM72xx::read(%p, %ld)", buffer, bytes);
    if (!mHardware) return -1;

    size_t count = bytes;
    size_t  aac_framesize= bytes;
    uint8_t* p = static_cast<uint8_t*>(buffer);
    uint32_t* recogPtr = (uint32_t *)p;
    uint16_t* frameCountPtr = NULL;
    uint16_t* frameSizePtr = NULL;

    if (mState < AUDIO_INPUT_OPENED) {
        AudioHardware *hw = mHardware;
        hw->mLock.lock();
        status_t status = set(hw, mDevices, &mFormat, &mChannels, &mSampleRate, mAcoustics);
        hw->mLock.unlock();
        if (status != NO_ERROR) {
            return -1;
        }
        mFirstread = false;
    }

    if (mState < AUDIO_INPUT_STARTED) {
        mState = AUDIO_INPUT_STARTED;
        // force routing to input device
        mHardware->clearCurDevice();
        mHardware->doRouting(this);
        if (ioctl(mFd, AUDIO_START, 0)) {
            LOGE("Error starting record");
            standby();
            return -1;
        }
        mHardware->msm72xx_enable_preproc(true);
    }

    // Resetting the bytes value, to return the appropriate read value
    bytes = 0;
    if (mFormat == AudioSystem::AAC)
    {
        *((uint32_t*)recogPtr) = 0x51434F4D ;// ('Q','C','O', 'M') Number to identify format as AAC by higher layers
        recogPtr++;
        frameCountPtr = (uint16_t*)recogPtr;
        *frameCountPtr = 0;
        p += 3*sizeof(uint16_t);
        count -= 3*sizeof(uint16_t);
    }
    while (count > 0) {
        if (mFormat == AudioSystem::AAC) {
            frameSizePtr = (uint16_t *)p;
            p += sizeof(uint16_t);
            if (!(count > 2)) break;
            count -= sizeof(uint16_t);
        }

        ssize_t bytesRead = ::read(mFd, p, count);
        if (bytesRead > 0) {
            LOGV("Number of Bytes read = %d", (int) bytesRead);
            count -= bytesRead;
            p += bytesRead;
            bytes += bytesRead;
            LOGV("Total Number of Bytes read = %d", (int) bytes);

            if (mFormat == AudioSystem::AAC){
                *frameSizePtr =  bytesRead;
                (*frameCountPtr)++;
            }

            if (!mFirstread)
            {
               mFirstread = true;
               break;
            }

        }
        else if (bytesRead == 0)
        {
            LOGI("Bytes Read = %d ,Buffer no longer sufficient",(int) bytesRead);
            break;
        } else {
            if (errno != EAGAIN) return bytesRead;
            mRetryCount++;
            LOGW("EAGAIN - retrying");
        }
    }
    if (mFormat == AudioSystem::AAC)
         return aac_framesize;

    return bytes;
}

status_t AudioHardware::AudioStreamInMSM72xx::standby()
{
    if (!mHardware) return -1;

    if (mState > AUDIO_INPUT_CLOSED) {
        mHardware->msm72xx_enable_preproc(false);
        if (mFd >= 0) {
            ::close(mFd);
            mFd = -1;
        }
        mState = AUDIO_INPUT_CLOSED;
    }


    // restore output routing if necessary
    mHardware->clearCurDevice();
    mHardware->doRouting(this);
    return NO_ERROR;
}

status_t AudioHardware::AudioStreamInMSM72xx::dump(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    result.append("AudioStreamInMSM72xx::dump\n");
    snprintf(buffer, SIZE, "\tsample rate: %d\n", sampleRate());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tbuffer size: %d\n", bufferSize());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tchannels: %d\n", channels());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tformat: %d\n", format());
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmHardware: %p\n", mHardware);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmFd count: %d\n", mFd);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmState: %d\n", mState);
    result.append(buffer);
    snprintf(buffer, SIZE, "\tmRetryCount: %d\n", mRetryCount);
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

status_t AudioHardware::AudioStreamInMSM72xx::setParameters(const String8& keyValuePairs)
{
    AudioParameter param = AudioParameter(keyValuePairs);
    String8 key = String8(AudioParameter::keyRouting);
    status_t status = NO_ERROR;
    int device;
    LOGV("AudioStreamInMSM72xx::setParameters() %s", keyValuePairs.string());

    if (param.getInt(key, device) == NO_ERROR) {
        LOGV("set input routing %x", device);
        if (device & (device - 1)) {
            status = BAD_VALUE;
        } else {
            mDevices = device;
            status = mHardware->doRouting(this);
        }
        param.remove(key);
    }

    if (param.size()) {
        status = BAD_VALUE;
    }
    return status;
}

#ifdef HAVE_FM_RADIO

status_t AudioHardware::setFmVolume(float v)
{
    unsigned int VolValue = (unsigned int)(AudioSystem::logToLinear(v));
    int volume = (unsigned int)(VolValue*VolValue/100);

    char volhex[10] = "";
    sprintf(volhex, "0x%x ", volume);
    char volreg[100] = "hcitool cmd 0x3f 0x15 0xf8 0x0 ";

    strcat(volreg, volhex);
    strcat(volreg, "0");

    system(volreg);

    return NO_ERROR;
}
#endif

String8 AudioHardware::AudioStreamInMSM72xx::getParameters(const String8& keys)
{
    AudioParameter param = AudioParameter(keys);
    String8 value;
    String8 key = String8(AudioParameter::keyRouting);

    if (param.get(key, value) == NO_ERROR) {
        LOGV("get routing %x", mDevices);
        param.addInt(key, (int)mDevices);
    }

    LOGV("AudioStreamInMSM72xx::getParameters() %s", param.toString().string());
    return param.toString();
}

// ----------------------------------------------------------------------------

extern "C" AudioHardwareInterface* createAudioHardware(void) {
    return new AudioHardware();
}

}; // namespace android
