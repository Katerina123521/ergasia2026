#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include "GlobalState.h"
#include "UIConstants.h"
#include "graphics.h"
#include "scancodes.h"


GlobalState::GlobalState()
    : m_status("Welcome! Enter the draw mode and Run the A*!")
{
}

GlobalState::~GlobalState()
{
    graphics::stopMusic(300);
    resetScore();

    // Delete UI widgets
    for (UIWidget* w : m_ui)
        delete w;
    m_ui.clear();
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

    if (m_musicOn)
        graphics::playMusic(m_musicFile, m_musicVolume, true, 800);

    setupUI();
}
