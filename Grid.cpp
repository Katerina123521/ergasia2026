#include "Grid.h"
#include <algorithm>

Grid::~Grid()
{
    clearAll();
}

void Grid::init(int rows, int cols, float startX, float startY, float cellSize)
{
    clearAll();

    m_rows = rows;
    m_cols = cols;

    Node::setLayout(startX, startY, cellSize);

    m_nodes.reserve(m_rows * m_cols);

    // Allocate nodes
    for (int r = 0; r < m_rows; r++)
        for (int c = 0; c < m_cols; c++)
            m_nodes.push_back(new Node(r, c));

    // Build 4-neighborhood adjacency
    for (int r = 0; r < m_rows; r++)
    {
        for (int c = 0; c < m_cols; c++)
        {
            Node* n = getNode(r, c);
            if (!n) continue;

            if (Node* up = getNode(r - 1, c))    n->neighbors.push_back(up);
            if (Node* down = getNode(r + 1, c))  n->neighbors.push_back(down);
            if (Node* left = getNode(r, c - 1))  n->neighbors.push_back(left);
            if (Node* right = getNode(r, c + 1)) n->neighbors.push_back(right);
        }
    }
}

Node* Grid::getNode(int r, int c) const
{
    if (r < 0 || r >= m_rows || c < 0 || c >= m_cols) return nullptr;
    return m_nodes[r * m_cols + c];
}

Node* Grid::getNodeFromPoint(float mx, float my) const
{
    for (Node* n : m_nodes)
        if (n && n->hitTest(mx, my)) return n;
    return nullptr;
}

void Grid::resetVisuals()
{
    for (Node* n : m_nodes)
        if (n) n->resetSearchData();
}

void Grid::clearAll()
{
    for (Node* n : m_nodes) delete n;
    m_nodes.clear();
    m_rows = 0;
    m_cols = 0;
}

void Grid::draw() const
{
    for (Node* n : m_nodes)
        if (n) n->draw();
}
