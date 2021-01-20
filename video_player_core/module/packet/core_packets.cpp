#include "core_packets.h"

core_packets::core_packets()
    :m_maxSize(FFMPEG_PK_SIZE_DEFAULT)
{

}

core_packets::core_packets(size_t maxSize)
    :m_maxSize(maxSize)
{

}

core_packets::~core_packets()
{
    clear();
}

size_t core_packets::size()
{
    LOCK(mutex)
    return m_packets.size();
}

bool core_packets::empty()
{
    return m_packets.empty();
}

bool core_packets::empty(AVPacket& pk)
{
    LOCK(mutex)
    bool bEmpty = m_packets.empty();
    if(!bEmpty)
    {
        pk = m_packets.front();
        m_packets.pop_front();
    }

    return bEmpty;
}

void core_packets::clear()
{
    LOCK(mutex)
    for(auto& it : m_packets)
    {
        av_packet_unref(&it);
    }

    m_packets.clear();
}

bool core_packets::isMax()
{
    return m_maxSize <= m_packets.size();
}

bool core_packets::push_back(const char* msg)
{
    LOCK(mutex)
    if(isMax()) return false;
    AVPacket pk;
    av_new_packet(&pk, static_cast<int>(strlen(msg)) + 1);
    strcpy(reinterpret_cast<char*>(pk.data), msg);
    m_packets.push_back(pk);
    return true;
}

bool core_packets::push_back(const AVPacket& pk)
{
    LOCK(mutex)
    if(isMax()) return false;
    m_packets.push_back(pk);
    return true;
}
