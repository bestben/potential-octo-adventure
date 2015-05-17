#include "time.h"

#include <GLFW/glfw3.h>

Time::Time() {
	m_startTime = 0.0;
}

Time::~Time() {

}

int Time::elapsed() const
{
	return (int)((glfwGetTime() - m_startTime) * 1000.0);
}

int Time::restart()
{
	m_startTime = glfwGetTime();
	return m_startTime;
}

void Time::start()
{
	m_startTime = glfwGetTime();
}

Time Time::currentTime()
{
    Time res;
	res.m_startTime = glfwGetTime();
    return res;
}
