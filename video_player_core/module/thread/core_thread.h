#ifndef CORE_THREAD_H
#define CORE_THREAD_H

class core_media;
class core_thread
{
public:
    core_thread();
    virtual ~core_thread();

    virtual bool start(core_media* pInfo);

protected:
    bool checkOpt();
    void setFlag(int bit, bool value = true);
    bool testFlag(int bit);
protected:
    core_media* m_media;
};

#endif // CORE_THREAD_H
