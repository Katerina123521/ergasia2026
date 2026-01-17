#include "GlobalState.h"

Node* GlobalState::nodeAt(int r, int c) const
{
    return m_grid.getNode(r, c);
}

Node* GlobalState::nodeFromMouse(float mx, float my) const
{
    return m_grid.getNodeFromPoint(mx, my);
}

void GlobalState::buildGridGraph()
{
    // Create nodes + neighbors in Grid
    m_grid.init(m_rows, m_cols, m_originX, m_originY, m_cell);

    // Rebuild polymorphic draw list (non-owning)
    m_drawables.clear();
    m_drawables.reserve(m_rows * m_cols);

    for (Node* n : m_grid.getAllNodes())
        m_drawables.push_back(n);
}

void GlobalState::clearWallsAndPathKeepEndpoints()
{
    for (Node* n : m_grid.getAllNodes())
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

    m_rows = newRows;
    m_cols = newCols;

    setupLayout();
    buildGridGraph();

    m_start = nodeAt(m_rows / 2, 2);
    m_goal = nodeAt(m_rows / 2, m_cols - 3);

    if (m_start) m_start->state = NodeVizState::Start;
    if (m_goal)  m_goal->state = NodeVizState::Goal;

    m_status = "Grid rebuilt: " + std::to_string(m_rows) + "x" + std::to_string(m_cols);
}
