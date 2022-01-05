#ifndef CORE_SAVE_AUDIO_H
#define CORE_SAVE_AUDIO_H

#include "video_player_core.h"
#include "../common.h"
#include "core_save_base.h"

// ISO-14496-15 AVC file format
//total: 7Byte( 56 bits )
#define ADTS_HEADER_SIZE 7
struct adst_header_t
{
    //fixd( 28 bits )
    unsigned int syncword : 12;
    unsigned int id : 1;
    unsigned int layer : 2;
    unsigned int protection_absent : 1;
    unsigned int profile : 1;
    unsigned int sampling_frequency_index : 4;
    unsigned int private_bit : 1;
    unsigned int channel_configuration : 3;
    unsigned int original_copy : 1;
    unsigned int home : 1;
    //variable( 28 bits )
    unsigned int copyright_identification_bit : 1;
    unsigned int copyright_identification_start : 1;
    unsigned int aac_frame_length : 13;
    unsigned int adst_buffer_fullness : 11;
    unsigned int number_of_raw_data_blocks_in_frame : 2;
//        unsigned int crc : 16;
};

//id
enum aac_mpeg
{
    aac_mpeg_4,
    aac_mpeg_2
};

//profile
enum aac_profile
{
    //mpeg-2
    profile_main,
    profile_low_complexity,//LC
    profile_scalable_sampling_rate,//(SSR)
    prifile_reserved
};

//freq
enum aac_freq
{
    aac_freq_96000,
    aac_frea_88200,
    aac_frea_64000,
    aac_frea_48000,
    aac_frea_44100,
    aac_frea_32000,
    aac_frea_24000,
    aac_frea_22050,
    aac_frea_16000,
    aac_frea_12000,
    aac_frea_11025,
    aac_frea_8000,
    aac_frea_7350
};

struct ADTSContext
{
    int write_adts;
    int objecttype;
    int sample_rate_index;
    int channel_conf;
};

class core_save_audio : public core_save_base
{
public:
    core_save_audio();
    ~core_save_audio() override;

    void save(AVPacket*) override;
private:
    bool init(AVFormatContext* pFormat, int nIndex) override;
    void uninit() override;
    AVOutputFormat *guess() override;

private:
    std::string outoutFile() override;
    int aac_decode_extradata(ADTSContext *adts, unsigned char *pbuf, int bufsize);
    void format_adst_header(int nLen);

private:
    ADTSContext m_adtsCtx;
    adst_header_t m_adstHeader;
};

#endif // CORE_SAVE_AUDIO_H
