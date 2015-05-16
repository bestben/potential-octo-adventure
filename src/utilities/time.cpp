#include "time.h"

Time::Time()
{

}

Time::~Time()
{

}

int Time::elapsed() const
{
    return m_time.elapsed();
}

int Time::msec() const
{
    return m_time.msec();
}

int Time::restart()
{
    return m_time.restart();
}

int Time::second() const
{
    return m_time.second();
}

void Time::start()
{
    m_time.start();
}

void Time::setTime( QTime t )
{
    m_time = t;
}

Time Time::currentTime()
{
    Time res;
    res.setTime(QTime::currentTime());
    return res;
}
