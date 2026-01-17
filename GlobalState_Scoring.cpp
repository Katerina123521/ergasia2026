#include "GlobalState.h"
#include <queue>
#include <limits>
#include <algorithm>
#include <cmath>

static constexpr int INF = std::numeric_limits<int>::max() / 4;


void GlobalState::resetScore()
{
    m_shortestSteps = -1;
    m_playerSteps = -1;
    m_onShortestCount = 0;
    m_overlap = 0.0f;
    m_efficiency = 0.0f;
    m_score = 0;
}

void GlobalState::computeBfsDistances(Node* src, std::vector<int>& dist)
{
    dist.assign(m_rows * m_cols, INF);
    if (!src) return;

    std::queue<Node*> q;
    dist[idx(src)] = 0;
    q.push(src);

    while (!q.empty())
    {
        Node* cur = q.front();
        q.pop();

        const int curd = dist[idx(cur)];

        for (Node* nb : cur->neighbors)
        {
            if (!nb || !nb->walkable) continue;

            const int nbIdx = idx(nb);
            if (dist[nbIdx] > curd + 1)
            {
                dist[nbIdx] = curd + 1;
                q.push(nb);
            }
        }
    }
}

bool GlobalState::computeShortestCorridor()
{
    if (!m_start || !m_goal) return false;

    computeBfsDistances(m_start, m_distStart);
    computeBfsDistances(m_goal, m_distGoal);

    const int goalI = idx(m_goal);
    if (m_distStart[goalI] >= INF)
    {
        m_shortestSteps = -1;
        m_onShortest.assign(m_rows * m_cols, 0);
        return false;
    }

    m_shortestSteps = m_distStart[goalI];

    m_onShortest.assign(m_rows * m_cols, 0);
    for (int i = 0; i < (int)m_onShortest.size(); i++)
    {
        if (m_distStart[i] >= INF || m_distGoal[i] >= INF) continue;
        if (m_distStart[i] + m_distGoal[i] == m_shortestSteps)
            m_onShortest[i] = 1; // this cell lies on at least one optimal path
    }

    return true;
}

bool GlobalState::computeScore()
{
    resetScore();

    if (m_pathValidation != PlayerPathValidation::ValidToGoal)
    {
        m_status = "Cannot score: draw a valid path that reaches the GOAL.";
        return false;
    }

    if (!computeShortestCorridor())
    {
        m_status = "No valid route exists in this level (Start->Goal unreachable).";
        return false;
    }

    // Steps are edges between nodes
    m_playerSteps = (int)m_playerPath.size() - 1;
    if (m_playerSteps < 0) m_playerSteps = 0;

    // Corridor overlap: fraction of player's visited cells that lie on ANY shortest path corridor
    int hits = 0;
    for (Node* n : m_playerPath)
    {
        if (!n) continue;
        if (m_onShortest[idx(n)]) hits++;
    }
    m_onShortestCount = hits;

    // Overlap as "how cleanly you stayed on the optimal corridor"
    m_overlap = (m_playerPath.empty()) ? 0.0f : (float)hits / (float)m_playerPath.size();

    // Efficiency: 1 if you matched shortest steps, smaller if longer
    m_efficiency = (m_shortestSteps <= 0) ? 1.0f :
        (float)m_shortestSteps / (float)std::max(m_shortestSteps, m_playerSteps);

    // Weighted score [0..100]
    float s = 100.0f * (0.65f * m_overlap + 0.35f * m_efficiency);
    s = std::max(0.0f, std::min(100.0f, s));
    m_score = (int)std::round(s);
    applyTimerBonus();

    return true;
}
