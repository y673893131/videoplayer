#include "core_thread.h"

#include "../media/core_media.h"
#include "../util/core_util.h"

core_thread::core_thread()
    :m_media(nullptr)
{

}

core_thread::~core_thread()
{

}

bool core_thread::start(core_media *pInfo)
{
    m_media = pInfo;
    return true;
}

void core_thread::setFlag(int bit, bool value)
{
    m_media->setFlag(bit, value);
}

bool core_thread::testFlag(int bit)
{
    return m_media->testFlag(bit);
}
