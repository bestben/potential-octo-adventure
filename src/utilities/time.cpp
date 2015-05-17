#include "time.h"

#include <glfw/glfw3.h>

Time::Time() {
	m_startTime = 0.0;
}

Time::~Time() {

}

int Time::elapsed() const
{
	return (int)((glfwGetTime() - m_startTime) * 1000.0);
}

void Time::restart()
{
	m_startTime = glfwGetTime();
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
