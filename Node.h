#pragma once
#include "VisualAsset.h"
#include <vector>

enum class NodeVizState
{
    Empty,
    Wall,
    Start,
    Goal,
    Open,
    Closed,
    Path,
    PlayerPath,
    PlayerInvalid
};

class Node final : public VisualAsset
{
public:
    Node(int r, int c);

    // Graph/grid data
    int row = 0;
    int col = 0;
    bool walkable = true;

    // A* data
    float g = 1e9f;
    float h = 0.0f;
    float f = 1e9f;
    Node* parent = nullptr;

    std::vector<Node*> neighbors;

    // Rendering
    NodeVizState state = NodeVizState::Empty;

    void resetSearchData();
    void draw() const override;

    // Hit test (for mouse selection) in canvas units
    bool hitTest(float x, float y) const;

    // Layout shared by all nodes (set by GlobalState)
    static void setLayout(float originX, float originY, float cellSize);

private:
    static float s_originX;
    static float s_originY;
    static float s_cell;
};

