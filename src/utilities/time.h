#ifndef TIME_H
#define TIME_H

#include <QTime>

class Time
{
public:
    Time();
    ~Time();

    int 	elapsed() const;
    int 	msec() const;
    int 	restart();
    int 	second() const;
    void 	start();

    void    setTime( QTime t );

    static Time currentTime();

private:
    int m_startTime;

    QTime m_time;
};

#endif // TIME_H
