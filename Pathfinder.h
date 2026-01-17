#pragma once
#include <vector>
#include "Node.h"

class Pathfinder
{
public:
    enum class Result { Running, Found, NoPath };

    void start(Node* start, Node* goal);
    Result step();
    void cancel();

private:
    float heuristic(const Node* a, const Node* b) const;

    std::vector<Node*> m_open;
    Node* m_start = nullptr;
    Node* m_goal = nullptr;
};
