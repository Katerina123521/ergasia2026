#pragma once
#include <vector>
#include <string>
#include "VisualAsset.h"
#include "Node.h"
#include "UIWidget.h"


class GlobalState final
{
public:
    GlobalState();
    ~GlobalState();

    void init();
    void update(float dt);
    void draw() const;

private:
    // Grid settings
    const int m_rows = 15;
    const int m_cols = 20;

    float m_cell = 40.0f;
    float m_originX = 0.0f;
    float m_originY = 0.0f;

    // Ownership: dynamically allocated nodes (assignment requires dynamic memory) :contentReference[oaicite:4]{index=4}
    std::vector<Node*> m_nodes;

    // Polymorphic collection (inheritance + polymorphism) :contentReference[oaicite:5]{index=5}
    std::vector<VisualAsset*> m_drawables;

    Node* m_start = nullptr;
    Node* m_goal = nullptr;

    std::string m_status;

private:
    Node* nodeAt(int r, int c) const;
    Node* nodeFromMouse(float mx, float my) const;

    void buildGridGraph();
    void clearWallsAndPathKeepEndpoints();
    void resetSearchVisuals();

    bool runAStar(); // returns true if found path
    float heuristic(const Node* a, const Node* b) const;

    enum class AStarRunState { Idle, Running, Paused, Found, NoPath };

    AStarRunState m_aState = AStarRunState::Idle;

    // Open list
    std::vector<Node*> m_open;

    // timing for animation
    float m_stepDelayMs = 35.0f;
    float m_stepAccumMs = 0.0f;

    // key edge detection
    bool m_prevSpace = false;
    bool m_prevR = false;

    void startAStar();
    bool stepAStar();
    void cancelAStar();

    std::vector<UIWidget*> m_ui;

    bool m_showHelp = false;   // big overlay
    LegendPanel* m_legend = nullptr;

    // (optional) keep UI text/status here
    std::string m_title = "A* Pathfinder";

    bool m_musicOn = true;
    float m_musicVolume = 0.20f;
    std::string m_musicFile = "assets/bgm.ogg";


};
