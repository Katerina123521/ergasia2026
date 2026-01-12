#include "GlobalState.h"
#include <unordered_set>
#include <vector>

Node* GlobalState::nodeAt(int r, int c) const
{
    if (r < 0 || r >= m_rows || c < 0 || c >= m_cols) return nullptr;
    return m_nodes[r * m_cols + c];
}

void GlobalState::buildGridGraph()
{
    m_nodes.reserve(m_rows * m_cols);
    for (int r = 0; r < m_rows; r++)
    {
        for (int c = 0; c < m_cols; c++)
        {
            Node* n = new Node(r, c);
            m_nodes.push_back(n);
            m_drawables.push_back(n);
        }
    }

    for (int r = 0; r < m_rows; r++)
    {
        for (int c = 0; c < m_cols; c++)
        {
            Node* n = nodeAt(r, c);
            if (!n) continue;

            Node* up = nodeAt(r - 1, c);
            Node* down = nodeAt(r + 1, c);
            Node* left = nodeAt(r, c - 1);
            Node* right = nodeAt(r, c + 1);

            if (up)    n->neighbors.push_back(up);
            if (down)  n->neighbors.push_back(down);
            if (left)  n->neighbors.push_back(left);
            if (right) n->neighbors.push_back(right);
        }
    }
}

Node* GlobalState::nodeFromMouse(float mx, float my) const
{
    for (Node* n : m_nodes)
        if (n->hitTest(mx, my)) return n;
    return nullptr;
}


void GlobalState::clearWallsAndPathKeepEndpoints()
{
    for (Node* n : m_nodes)
    {
        n->walkable = true;
        n->state = NodeVizState::Empty;
        n->parent = nullptr;
        n->g = 1e9f; n->h = 0.0f; n->f = 1e9f;
    }

    if (m_start) m_start->state = NodeVizState::Start;
    if (m_goal)  m_goal->state = NodeVizState::Goal;
}

void GlobalState::rebuildGrid(int newRows, int newCols)
{
    if (newRows < 3) newRows = 3;
    if (newCols < 3) newCols = 3;

    cancelAStar();
    clearPlayerPath();
    resetScore();
    resetAttemptTimer();

    std::unordered_set<VisualAsset*> oldNodeDrawables;
    oldNodeDrawables.reserve(m_nodes.size() * 2 + 1);
    for (Node* n : m_nodes)
        oldNodeDrawables.insert(static_cast<VisualAsset*>(n));

    std::vector<VisualAsset*> kept;
    kept.reserve(m_drawables.size());
    for (VisualAsset* a : m_drawables)
        if (oldNodeDrawables.find(a) == oldNodeDrawables.end())
            kept.push_back(a);

    m_drawables.swap(kept);

    for (Node* n : m_nodes) delete n;
    m_nodes.clear();

    m_start = nullptr;
    m_goal = nullptr;

    m_rows = newRows;
    m_cols = newCols;

    buildGridGraph();

    m_status = "Grid rebuilt: " + std::to_string(m_rows) + "x" + std::to_string(m_cols);
}
