#include "GlobalState.h"
#include <algorithm>
#include <unordered_set>
#include <cmath>


void GlobalState::clearPlayerPath()
{
    for (Node* n : m_playerPath)
    {
        if (!n) continue;
        if (n == m_start || n == m_goal) continue;
        if (!n->walkable) continue; // wall stays wall
        // Only clear if it is player path
        if (n->state == NodeVizState::PlayerPath)
            n->state = NodeVizState::Empty;
    }
    m_playerPath.clear();
    m_lastDrawn = nullptr;
}

bool GlobalState::isAdjacent(const Node* a, const Node* b) const
{
    if (!a || !b) return false;
    const int dr = std::abs(a->row - b->row);
    const int dc = std::abs(a->col - b->col);
    return (dr + dc) == 1; // 4-neighborhood
}

void GlobalState::beginPlayerDrawing()
{
    resetScore();
    clearPlayerPath();
    clearInvalidMarks();

    if (m_start)
    {
        m_playerPath.push_back(m_start);
        m_lastDrawn = m_start;
    }
    m_isDrawing = true;
    m_status = "Draw Mode: drag from Start to Goal (adjacent cells only).";
}

void GlobalState::endPlayerDrawing()
{
    m_isDrawing = false;
    m_lastDrawn = nullptr;
}

bool GlobalState::appendPlayerPath(Node* n)
{
    if (!n) return false;
    if (!n->walkable) return false;          // cannot draw through walls
    if (n == m_start) return false;          // already included as first
    if (m_playerPath.empty()) return false;

    Node* last = m_playerPath.back();

    // must be adjacent to the previous node
    if (!isAdjacent(last, n)) return false;

    // avoid revisiting nodes
    if (std::find(m_playerPath.begin(), m_playerPath.end(), n) != m_playerPath.end())
        return false;

    m_playerPath.push_back(n);

    // paint it
    if (n != m_goal && n != m_start)
        n->state = NodeVizState::PlayerPath;

    if (n == m_goal)
        m_status = "Player path reached GOAL!";

    return true;
}

void GlobalState::clearInvalidMarks()
{
    // Remove any PlayerInvalid marks, keep walls/start/goal
    for (Node* n : m_grid.getAllNodes())

    {
        if (!n) continue;
        if (n == m_start) { n->state = NodeVizState::Start; continue; }
        if (n == m_goal) { n->state = NodeVizState::Goal;  continue; }
        if (!n->walkable) { n->state = NodeVizState::Wall;  continue; }

        if (n->state == NodeVizState::PlayerInvalid)
            n->state = NodeVizState::Empty;
    }
}

void GlobalState::markInvalidNode(int index)
{
    if (index < 0 || index >= (int)m_playerPath.size()) return;
    Node* n = m_playerPath[index];
    if (!n) return;
    if (n == m_start || n == m_goal) return;
    if (!n->walkable) return;

    n->state = NodeVizState::PlayerInvalid;
}

GlobalState::PlayerPathValidation GlobalState::validatePlayerPath()
{
    clearInvalidMarks();
    m_invalidIndex = -1;
    m_pathComplete = false;

    if (m_playerPath.empty())
        return PlayerPathValidation::Empty;

    // Must start at Start
    if (m_playerPath.front() != m_start)
    {
        m_invalidIndex = 0;
        markInvalidNode(0);
        return PlayerPathValidation::InvalidStart;
    }

    std::unordered_set<Node*> seen;
    seen.reserve(m_playerPath.size() * 2);

    for (int i = 0; i < (int)m_playerPath.size(); i++)
    {
        Node* cur = m_playerPath[i];
        if (!cur)
        {
            m_invalidIndex = i;
            return PlayerPathValidation::InvalidWall;
        }

        // No walls allowed
        if (!cur->walkable)
        {
            m_invalidIndex = i;
            return PlayerPathValidation::InvalidWall;
        }

        // No duplicates
        if (seen.find(cur) != seen.end())
        {
            m_invalidIndex = i;
            markInvalidNode(i);
            return PlayerPathValidation::InvalidDuplicate;
        }
        seen.insert(cur);

        // Must be continuous (adjacent moves)
        if (i > 0)
        {
            Node* prev = m_playerPath[i - 1];
            if (!isAdjacent(prev, cur))
            {
                m_invalidIndex = i;
                markInvalidNode(i);
                return PlayerPathValidation::InvalidNonAdjacent;
            }
        }
    }

    if (m_playerPath.back() == m_goal)
    {
        m_pathComplete = true;
        return PlayerPathValidation::ValidToGoal;
    }

    return PlayerPathValidation::ValidButNotAtGoal;
}