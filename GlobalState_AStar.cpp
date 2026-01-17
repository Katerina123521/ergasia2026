#include "GlobalState.h"

void GlobalState::resetSearchVisuals()
{
    for (Node* n : m_grid.getAllNodes())
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

    m_pf.cancel();
    m_pf.start(m_start, m_goal);

    while (true)
    {
        auto res = m_pf.step();
        if (res == Pathfinder::Result::Found)
        {
            Node* p = m_goal->parent;
            while (p && p != m_start)
            {
                p->state = NodeVizState::Path;
                p = p->parent;
            }
            return true;
        }
        if (res == Pathfinder::Result::NoPath)
        {
            return false;
        }
    }
}

void GlobalState::cancelAStar()
{
    m_pf.cancel();
    m_aState = AStarRunState::Idle;
    m_stepAccumMs = 0.0f;

    resetSearchVisuals();
    m_status = "A*: stopped. SPACE: start/pause | R: reset search";
}

void GlobalState::startAStar()
{
    if (!m_start || !m_goal) return;

    resetSearchVisuals();
    m_pf.start(m_start, m_goal);

    m_stepAccumMs = 0.0f;
    m_aState = AStarRunState::Running;
    m_status = "A*: running (step-by-step)... SPACE pause/resume | R reset";
}

bool GlobalState::stepAStar()
{
    auto res = m_pf.step();

    if (res == Pathfinder::Result::Found)
    {
        // reconstruct path
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

    if (res == Pathfinder::Result::NoPath)
    {
        m_aState = AStarRunState::NoPath;
        m_status = "A*: No path.";
        return true;
    }

    return false;
}
