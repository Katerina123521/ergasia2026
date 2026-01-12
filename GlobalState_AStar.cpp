#include "GlobalState.h"
#include <algorithm>
#include <cmath>

float GlobalState::heuristic(const Node* a, const Node* b) const
{
    // Manhattan distance works well on 4-neighborhood grids
    return float(std::abs(a->row - b->row) + std::abs(a->col - b->col));
}
void GlobalState::resetSearchVisuals()
{
    for (Node* n : m_nodes)
    {
        // Preserve walls + endpoints
        if (n == m_start)
        {
            n->walkable = true;
            n->state = NodeVizState::Start;
            n->g = 1e9f; n->h = 0.0f; n->f = 1e9f; n->parent = nullptr;
            continue;
        }
        if (n == m_goal)
        {
            n->walkable = true;
            n->state = NodeVizState::Goal;
            n->g = 1e9f; n->h = 0.0f; n->f = 1e9f; n->parent = nullptr;
            continue;
        }

        n->resetSearchData();
    }
}

bool GlobalState::runAStar()
{
    if (!m_start || !m_goal) return false;

    resetSearchVisuals();

    std::vector<Node*> open;
    open.reserve(m_rows * m_cols);

    m_start->g = 0.0f;
    m_start->h = heuristic(m_start, m_goal);
    m_start->f = m_start->g + m_start->h;

    open.push_back(m_start);

    while (!open.empty())
    {
        // pick min-f node (simple O(n), ok for small grid)
        auto itMin = std::min_element(open.begin(), open.end(),
            [](const Node* a, const Node* b) { return a->f < b->f; });

        Node* current = *itMin;
        open.erase(itMin);

        if (current != m_start && current != m_goal)
            current->state = NodeVizState::Closed;

        if (current == m_goal)
        {
            // reconstruct path
            Node* p = m_goal->parent;
            while (p && p != m_start)
            {
                p->state = NodeVizState::Path;
                p = p->parent;
            }
            return true;
        }

        for (Node* nb : current->neighbors)
        {
            if (!nb->walkable) continue;
            if (nb->state == NodeVizState::Closed) continue;

            const float tentative_g = current->g + 1.0f;

            const bool inOpen = (std::find(open.begin(), open.end(), nb) != open.end());
            if (!inOpen || tentative_g < nb->g)
            {
                nb->parent = current;
                nb->g = tentative_g;
                nb->h = heuristic(nb, m_goal);
                nb->f = nb->g + nb->h;

                if (!inOpen)
                {
                    open.push_back(nb);
                    if (nb != m_start && nb != m_goal)
                        nb->state = NodeVizState::Open;
                }
            }
        }
    }

    return false;
}

void GlobalState::cancelAStar()
{
    m_aState = AStarRunState::Idle;
    m_open.clear();
    m_stepAccumMs = 0.0f;

    // �������� Open/Closed/Path ���� ����� walls + start/goal
    resetSearchVisuals();

    m_status = "A*: stopped. SPACE: start/pause | R: reset search";
}

void GlobalState::startAStar()
{
    if (!m_start || !m_goal) return;

    resetSearchVisuals();

    m_open.clear();
    m_open.reserve(m_rows * m_cols);

    m_start->g = 0.0f;
    m_start->h = heuristic(m_start, m_goal);
    m_start->f = m_start->g + m_start->h;
    m_start->parent = nullptr;

    m_open.push_back(m_start);

    m_stepAccumMs = 0.0f;
    m_aState = AStarRunState::Running;
    m_status = "A*: running (step-by-step)... SPACE pause/resume | R reset";
}

bool GlobalState::stepAStar()
{
    if (!m_start || !m_goal) return true;

    if (m_open.empty())
    {
        m_aState = AStarRunState::NoPath;
        m_status = "A*: No path (open list empty).";
        return true;
    }

    // pick min-f node
    auto itMin = std::min_element(m_open.begin(), m_open.end(),
        [](const Node* a, const Node* b)
        {
            if (a->f == b->f) return a->h < b->h; // tie-breaker
            return a->f < b->f;
        });

    Node* current = *itMin;
    m_open.erase(itMin);

    if (current != m_start && current != m_goal)
        current->state = NodeVizState::Closed;

    // goal reached
    if (current == m_goal)
    {
        Node* p = m_goal->parent;
        while (p && p != m_start)
        {
            p->state = NodeVizState::Path;
            p = p->parent;
        }

        m_aState = AStarRunState::Found;
        m_status = "A*: Path found!";
        return true;
    }

    // relax neighbors (1 expansion step)
    for (Node* nb : current->neighbors)
    {
        if (!nb->walkable) continue;
        if (nb->state == NodeVizState::Closed) continue;

        const float tentative_g = current->g + 1.0f;

        const bool inOpen = (std::find(m_open.begin(), m_open.end(), nb) != m_open.end());
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

    return false; // not finished yet
}