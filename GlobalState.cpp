#include "GlobalState.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include "UIConstants.h"
#include "graphics.h"
#include "scancodes.h"


GlobalState::GlobalState()
    : m_status("LMB: wall | RMB: start | Shift+RMB: goal | SPACE: A* | C: clear")
{
}

GlobalState::~GlobalState()
{
    graphics::stopMusic(300);
    resetScore();

    // Delete UI widgets first (they are NOT in m_nodes)
    for (UIWidget* w : m_ui)
        delete w;
    m_ui.clear();

    // Delete grid nodes
    for (Node* n : m_nodes)
        delete n;
    m_nodes.clear();

    m_drawables.clear();
}

void GlobalState::init()
{
    // Assets / levels
    graphics::preloadBitmaps("assets/ui");

    m_levelsEasy = { "assets/levels/easy_01.txt",   "assets/levels/easy_02.txt" };
    m_levelsMedium = { "assets/levels/medium_01.txt", "assets/levels/medium_02.txt" };
    m_levelsHard = { "assets/levels/hard_01.txt",   "assets/levels/hard_02.txt" };

    // Layout + grid
    setupLayout();
    buildGridGraph();

    // Default start/goal
    m_start = nodeAt(m_rows / 2, 2);
    m_goal = nodeAt(m_rows / 2, m_cols - 3);

    if (m_start) m_start->state = NodeVizState::Start;
    if (m_goal)  m_goal->state = NodeVizState::Goal;

    // Font + music
    graphics::setFont("assets/orbitron.ttf");
    if (m_musicOn)
        graphics::playMusic(m_musicFile, m_musicVolume, true, 800);

    // UI
    setupUI();
}
