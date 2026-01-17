#include "GlobalState.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

template <typename T>
T clampValue(T v, T lo, T hi)
{
    return (v < lo) ? lo : (v > hi) ? hi : v;
}


void GlobalState::startAttemptTimer()
{
    if (!m_timerEnabled) return;
    m_attemptTimeMs = 0.0f;
    m_timerRunning = true;
}

void GlobalState::stopAttemptTimer()
{
    if (!m_timerEnabled) return;
    m_timerRunning = false;
    m_lastAttemptMs = m_attemptTimeMs;
}

void GlobalState::resetAttemptTimer()
{
    m_timerRunning = false;
    m_attemptTimeMs = 0.0f;
    m_lastAttemptMs = 0.0f;
    m_timeBonus = 0;
}

void GlobalState::updateAttemptTimer(float dt)
{
    if (!m_timerEnabled) return;
    if (m_timerRunning)
        m_attemptTimeMs += dt;
}

std::string GlobalState::formatTime(float ms) const
{
    int totalMs = (int)std::round(std::max(0.0f, ms));
    int minutes = totalMs / 60000;
    int seconds = (totalMs % 60000) / 1000;
    int msec = totalMs % 1000;

    std::ostringstream oss;
    oss << minutes << ":" << std::setw(2) << std::setfill('0') << seconds
        << "." << std::setw(3) << std::setfill('0') << msec;
    return oss.str();
}

void GlobalState::applyTimerBonus()
{
    m_timeBonus = 0;

    // Par time depends on shortest steps (BFS shortest distance)
    const float parMs = m_parBaseMs + m_parPerStepMs * std::max(0, m_shortestSteps);

    if (!m_timerEnabled || m_lastAttemptMs <= 0.0f || parMs <= 0.0f)
    {
        int pts = m_score;
        if (!m_allowScoreOver100) pts = std::min(pts, 100);
        m_finalPoints = std::max(0, pts);
        return;
    }

    // Faster than par -> bonus, slower -> 0
    float ratio = (parMs - m_lastAttemptMs) / parMs; // >0 if faster
    ratio = clampValue(ratio, 0.0f, 1.0f);

    m_timeBonus = (int)std::round((float)m_maxTimeBonus * ratio);

    int pts = m_score + m_timeBonus;
    if (!m_allowScoreOver100) pts = std::min(pts, 100);
    m_finalPoints = std::max(0, pts);
}