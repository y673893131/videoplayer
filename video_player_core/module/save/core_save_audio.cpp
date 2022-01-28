#include "core_save_audio.h"
#include "../util/core_util.h"
#include "../media/core_media.h"
#include "../common.h"
#include "Log/Log.h"

core_save_audio::core_save_audio()
    : core_save_base()
{
}

core_save_audio::~core_save_audio()
{
    uninit();
}

bool core_save_audio::init(core_media* media, int nIndex)
{
    uninit();
    if(!core_save_base::init(media, nIndex))
    {
        uninit();
        return false;
    }

    aac_decode_extradata(&m_adtsCtx, m_stream->codec->extradata
                         , m_stream->codec->extradata_size);

    m_adtsCtx.objecttype = profile_low_complexity;
    m_adtsCtx.sample_rate_index = aac_frea_44100;
    m_adtsCtx.channel_conf = 2;
//    m_adstHeader = {
//        // fixed
//        0xFFF,                  /* syncword */
//        aac_mpeg_2,             /* id */
//        0x00,                   /* layer */
//        1,                      /* protection_absent */
//        m_adtsCtx.objecttype,           /* profile */
//        m_adtsCtx.sample_rate_index,    /* sampling_frequency_index */
//        0,                      /* private_bit */
//        m_adtsCtx.channel_conf, /* channel_configuration */
//        0,                      /* original_copy */
//        0,                      /* home */
//        // variable
//        0,                      /* copyright_identification_bit */
//        0,                      /* copyright_identification_start */
//        ADTS_HEADER_SIZE,       /* aac_frame_length */
//        0x7FF,                  /* adst_buffer_fullness */
//        0                       /* number_of_raw_data_blocks_in_frame */
//        };

    return true;
}

void core_save_audio::uninit()
{
    core_save_base::uninit();
}

void core_save_audio::save(AVPacket* pk)
{
    if(!m_format)
        return;
//    format_adst_header(pk->size + ADTS_HEADER_SIZE);
//    fwrite(&m_adstHeader, 1, ADTS_HEADER_SIZE, m_pFile);
//    fwrite(pk->data, 1, static_cast<size_t>(pk->size), m_pFile);
//    fflush(m_pFile);
//    return;
    AVPacket* _pk = av_packet_clone(pk);
    rescalePacket(_pk);
//    auto newlt = _pk->dts;
//    auto lt1 = m_lastDts;
//    Log(Log_Warning, "rescalePacket %lld(%lld->%lld) -> %lld", lt1, pk->dts,pk->pts, newlt);
    int ret = av_interleaved_write_frame(m_format, _pk);
    if(ret < 0)
    {
        char buf[1024] = {};
        av_strerror(ret, buf, 1024);
        Log(Log_Warning, "av_interleaved_write_frame failed, %s", buf);
    }
    av_packet_unref(_pk);
}

std::string core_save_audio::outoutFile()
{
    return core_util::toDateTime(time(nullptr)) + ".aac";
}

int core_save_audio::aac_decode_extradata(ADTSContext *adts, unsigned char *pbuf, int bufsize)
{
      int aot, aotext, samfreindex;
      int channelconfig;
      unsigned char *p = pbuf;
      if (!adts || !pbuf || bufsize<2)
      {
            return -1;
      }
      aot = (p[0]>>3)&0x1f;
      if (aot == 31)
      {
            aotext = (p[0]<<3 | (p[1]>>5)) & 0x3f;
            aot = 32 + aotext;
            samfreindex = (p[1]>>1) & 0x0f;
            if (samfreindex == 0x0f)
            {
                  channelconfig = ((p[4]<<3) | (p[5]>>5)) & 0x0f;
            }
            else
            {
                  channelconfig = ((p[1]<<3)|(p[2]>>5)) & 0x0f;
            }
      }
      else
      {
            samfreindex = ((p[0]<<1)|p[1]>>7) & 0x0f;
            if (samfreindex == 0x0f)
            {
                  channelconfig = (p[4]>>3) & 0x0f;
            }
            else
            {
                  channelconfig = (p[1]>>3) & 0x0f;
            }
      }
#ifdef AOT_PROFILE_CTRL
      if (aot < 2) aot = 2;
#endif
      adts->objecttype = aot-1;
      adts->sample_rate_index = samfreindex;
      adts->channel_conf = channelconfig;
      adts->write_adts = 1;
      return 0;
}

void core_save_audio::format_adst_header(int nLen)
{
    m_adstHeader.aac_frame_length = nLen;
}
