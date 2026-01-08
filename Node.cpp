#include "Node.h"
#include "graphics.h"
#include "scancodes.h"

#include <cmath>

float Node::s_originX = 0.0f;
float Node::s_originY = 0.0f;
float Node::s_cell = 40.0f;

Node::Node(int r, int c) : row(r), col(c) {}

void Node::setLayout(float originX, float originY, float cellSize)
{
    s_originX = originX;
    s_originY = originY;
    s_cell = cellSize;
}

void Node::resetSearchData()
{
    g = 1e9f;
    h = 0.0f;
    f = 1e9f;
    parent = nullptr;

    if (state == NodeVizState::Open || state == NodeVizState::Closed || state == NodeVizState::Path)
        state = NodeVizState::Empty;

    // Keep Start/Goal/Wall intact
    if (!walkable) state = NodeVizState::Wall;
}

bool Node::hitTest(float x, float y) const
{
    const float cx = s_originX + col * s_cell + s_cell * 0.5f;
    const float cy = s_originY + row * s_cell + s_cell * 0.5f;
    const float w = s_cell;
    const float h = s_cell;

    // Point-in-rect test (matches the assignment collision section) :contentReference[oaicite:3]{index=3}
    return (cx - w * 0.5f <= x && x <= cx + w * 0.5f) &&
        (cy - h * 0.5f <= y && y <= cy + h * 0.5f);
}

static void setColorForState(graphics::Brush& br, NodeVizState st)
{
    br.outline_opacity = 1.0f;
    br.outline_width = 1.0f;
    br.outline_color[0] = 0.15f; br.outline_color[1] = 0.15f; br.outline_color[2] = 0.15f;

    br.fill_opacity = 1.0f;

    switch (st)
    {
    case NodeVizState::Empty:
        br.fill_color[0] = 0.92f; br.fill_color[1] = 0.92f; br.fill_color[2] = 0.92f; break;
    case NodeVizState::Wall:
        br.fill_color[0] = 0.10f; br.fill_color[1] = 0.10f; br.fill_color[2] = 0.10f; break;
    case NodeVizState::Start:
        br.fill_color[0] = 0.20f; br.fill_color[1] = 0.80f; br.fill_color[2] = 0.20f; break;
    case NodeVizState::Goal:
        br.fill_color[0] = 0.90f; br.fill_color[1] = 0.25f; br.fill_color[2] = 0.25f; break;
    case NodeVizState::Open:
        br.fill_color[0] = 0.35f; br.fill_color[1] = 0.70f; br.fill_color[2] = 0.95f; break;
    case NodeVizState::Closed:
        br.fill_color[0] = 0.55f; br.fill_color[1] = 0.55f; br.fill_color[2] = 0.70f; break;
    case NodeVizState::Path:
        br.fill_color[0] = 1.00f; br.fill_color[1] = 0.85f; br.fill_color[2] = 0.15f; break;
    }
}

void Node::draw() const
{
    const float cx = s_originX + col * s_cell + s_cell * 0.5f;
    const float cy = s_originY + row * s_cell + s_cell * 0.5f;

    graphics::Brush br;
    setColorForState(br, state);

    graphics::drawRect(cx, cy, s_cell - 1.0f, s_cell - 1.0f, br);
}
