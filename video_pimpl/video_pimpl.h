#ifndef VIDEO_PIMPL_H
#define VIDEO_PIMPL_H

#include <memory>
/*
 * impl:
*/

#define VP_RLEASE(Class) if(Class){ delete Class; Class = nullptr; }

#define VP_INIT(ptr) vp_ptr(ptr)

#define VP_DECLARE(Class) VP_Impl<Class, Class##Private> vp_ptr;

#define VP_DECLARE_PRIVATE(Class) \
    inline Class##Private* vp_get(){ return reinterpret_cast<Class##Private*>(vp_ptr.get_pri()); } \
    inline const Class##Private* vp_get() const{ return reinterpret_cast<const Class##Private*>(vp_ptr.get_pri()); } \
    friend class Class##Private;

#define VP_DECLARE_PUBLIC(Class) \
    inline Class* vp_get(){ return static_cast<Class*>(get_pub()); } \
    inline const Class* vp_get() const{ return static_cast<const Class*>(get_pub()); } \
    friend class Class;

#define VP_D(Class) Class##Private* const d = vp_get()
#define VP_Q(Class) Class * const q = vp_get()

template<typename Class>
class VP_Data
{
public:
    VP_Data(Class* ptr)
        : pub_ptr(ptr)
    {
    }

    virtual ~VP_Data()
    {
    }

    Class* get_pub()
    {
        return pub_ptr;
    }

    const Class* get_pub() const
    {
        return pub_ptr;
    }

private:
    Class* pub_ptr;
};

template<typename Pub, typename Pri>
class VP_Impl
{
public:
    VP_Impl()
        :pri(new Pri)
    {
    }

    VP_Impl(Pri* ptr)
        :pri(ptr)
    {
    }

    ~VP_Impl()
    {
        VP_RLEASE(pri)
    }

    Pri* get_pri()
    {
        return reinterpret_cast<Pri*>(pri);
    }

    const Pri* get_pri() const
    {
        return reinterpret_cast<Pri*>(pri);
    }

private:
    VP_Impl(const VP_Impl&) = delete;
    VP_Impl& operator=(const VP_Impl&) = delete;

private:
    VP_Data<Pub>* pri;
};

/*
 * test case
 *
#include <QDebug>
class basePrivate;
class base
{
    VP_DECLARE_PRIVATE(base)
public:
    base();
    base(basePrivate*);

protected:
    VP_DECLARE(base)
};

class basePrivate : public VP_Data<base>
{
    VP_DECLARE_PUBLIC(base)
public:
    inline basePrivate(base* parent)
        : VP_Data(parent)
    {
        qDebug() << __FUNCTION__ << this;
    }

    ~basePrivate()
    {
        qDebug() << __FUNCTION__ << this;
    }
};

base::base()
    : VP_INIT(new basePrivate(this))
{
    qDebug() << __FUNCTION__;
}

base::base(basePrivate* pri)
    : VP_INIT(pri)
{
    qDebug() << __FUNCTION__;
}

class childPrivate;
class child : public base
{
    VP_DECLARE_PRIVATE(child)
public:
    child();
    ~child()
    {
        qDebug() << __FUNCTION__;
    }
private:

};

class childPrivate : public basePrivate
{
    VP_DECLARE_PUBLIC(child)
public:
    inline childPrivate(child* parent)
      : basePrivate(parent)
    {
        qDebug() << __FUNCTION__;
    }

    ~childPrivate()
    {
        qDebug() << __FUNCTION__;
    }
};

child::child()
    : base(new childPrivate(this))
{
    qDebug() << __FUNCTION__;
}
*/

#endif // VIDEO_PIMPL_H
