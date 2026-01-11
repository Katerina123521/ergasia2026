#pragma once

#include <vector>
#include "Node.h"

class Grid {
    int m_rows = 0;
    int m_cols = 0;
    std::vector<Node*> m_nodes;

public:
    ~Grid();

    void init(int rows, int cols, float startX, float startY, float cellSize);
    void draw() const;

    Node* getNode(int r, int c) const;
    Node* getNodeFromPoint(float mx, float my) const;
    void resetVisuals();
    void clearAll();

    const std::vector<Node*>& getAllNodes() const { return m_nodes; }
};