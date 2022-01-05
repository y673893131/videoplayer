#ifndef CORE_PACKETS_H
#define CORE_PACKETS_H

#include "../common.h"
#include "../lock/core_graud_lock.h"

#define FFMPEG_PK_SIZE_DEFAULT 1000

class core_packets
{
public:
    core_packets();
    core_packets(size_t maxSize);

    virtual ~core_packets();

    size_t size();
    bool empty();
    bool empty(AVPacket& pk, bool& isSeek);
    void clear();
    bool isMax();
    void push_flush();
    bool push_back(const AVPacket& pk);

private:
    size_t m_maxSize;
    core_graud_lock mutex;
    std::list<AVPacket> m_packets;
};

#endif // CORE_PACKETS_H
