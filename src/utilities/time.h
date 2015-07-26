#ifndef TIME_H
#define TIME_H

class Time
{
public:
    Time();
    ~Time();

    float 	elapsed() const; // in milliseconds
    void 	restart();
    void 	start();

    static Time currentTime();

private:
    double m_startTime;
};

#endif // TIME_H
