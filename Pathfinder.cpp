#include "Pathfinder.h"
#include <algorithm>
#include <cmath>

float Pathfinder::heuristic(const Node* a, const Node* b) const
{
    return float(std::abs(a->row - b->row) + std::abs(a->col - b->col));
}

void Pathfinder::start(Node* start, Node* goal)
{
    m_open.clear();
    m_start = start;
    m_goal = goal;

    if (!m_start || !m_goal) return;

    m_start->g = 0.0f;
    m_start->h = heuristic(m_start, m_goal);
    m_start->f = m_start->g + m_start->h;
    m_start->parent = nullptr;

    m_open.push_back(m_start);
}

Pathfinder::Result Pathfinder::step()
{
    if (!m_start || !m_goal) return Result::NoPath;

    if (m_open.empty())
        return Result::NoPath;

    auto itMin = std::min_element(m_open.begin(), m_open.end(),
        [](const Node* a, const Node* b)
        {
            if (a->f == b->f) return a->h < b->h;
            return a->f < b->f;
        });

    Node* current = *itMin;
    m_open.erase(itMin);

    if (current != m_start && current != m_goal)
        current->state = NodeVizState::Closed;

    if (current == m_goal)
        return Result::Found;

    for (Node* nb : current->neighbors)
    {
        if (!nb->walkable) continue;
        if (nb->state == NodeVizState::Closed) continue;

        float tentative_g = current->g + 1.0f;

        bool inOpen = (std::find(m_open.begin(), m_open.end(), nb) != m_open.end());
        if (!inOpen || tentative_g < nb->g)
        {
            nb->parent = current;
            nb->g = tentative_g;
            nb->h = heuristic(nb, m_goal);
            nb->f = nb->g + nb->h;

            if (!inOpen)
            {
                m_open.push_back(nb);
                if (nb != m_start && nb != m_goal)
                    nb->state = NodeVizState::Open;
            }
        }
    }

    return Result::Running;
}

void Pathfinder::cancel()
{
    m_open.clear();
    m_start = nullptr;
    m_goal = nullptr;
}
