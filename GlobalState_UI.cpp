#include "GlobalState.h"
#include "UIConstants.h"
#include "graphics.h"
#include "scancodes.h"

void GlobalState::setupUI()
{   
    float bw = UI_LEFT_W - 2.0f * UI_MARGIN;
    float bx = UI_LEFT_W * 0.5f;
    float by = UI_TOP_H + 90.0f;
    float bh = 42.0f;

    float gap = 10.0f;
    float smallW = (bw - 2.0f * gap) / 2.2f;
    auto skinBtn = [&](Button* b, const std::string& theme)
        {
            b->texIdle = "assets/ui/btn_" + theme + "_idle.png";
            b->texHover = "assets/ui/btn_" + theme + "_hover.png";
            b->texDown = "assets/ui/btn_" + theme + "_down.png";
            b->texDisabled = "assets/ui/btn_gray_idle.png";
        };

    auto centerTextPad = [](Button* b)
        {
            if (!b) return;

            float approxTextW = (float)b->text.size() * b->textSize * 0.55f;
            float pad = (b->w - approxTextW) * 0.5f;
            if (pad < 6.0f) pad = 6.0f;

            b->padX = pad;
        };

    auto addBtn = [&](const std::string& txt, const std::string& theme, std::function<void()> cb)
        {
            Button* b = new Button(bx, by, bw, bh, txt, cb);
            b->clickSound = "assets/hit1.wav";
            b->clickVolume = 0.20f;
            b->textSize = 18.0f;
            b->padX = 18.0f;

            skinBtn(b, theme);

            m_ui.push_back(b);
            by += (bh + gap);
            return b;
        };

    auto addBtnAt = [&](float x, float y,
        float w, float h,
        const std::string& txt,
        const std::string& theme,
        std::function<void()> cb)
        {
            Button* b = new Button(x, y, w, h, txt, cb);
            b->clickSound = "assets/hit1.wav";
            b->clickVolume = 0.20f;
            b->textSize = 18.0f;
            skinBtn(b, theme);
            m_ui.push_back(b);
            return b;
        };

    auto addSmallBtn = [&](const std::string& txt, const std::string& theme, std::function<void()> cb)
        {
            Button* b = new Button(bx, by, smallW, bh, txt, cb);
            b->clickSound = "assets/hit1.wav";
            b->clickVolume = 0.20f;
            b->textSize = 16.0f;
            centerTextPad(b);

            skinBtn(b, theme);

            m_ui.push_back(b);
            by += (bh + gap);
            return b;
        };

    // --- Draw Mode button ---
    float drawModeH = bh * 1.0f;
    float drawModeW = (bw - gap) / 2.0f;
    
    float drawModeLeftEdge = bx - bw * 0.5f;
    float drawX = drawModeLeftEdge + drawModeW * 0.5f;
    
    Button* drawModeBtn = addBtnAt(drawX, by, drawModeW, drawModeH, "Draw Mode OFF", "pink", []() {});
    drawModeBtn->onClick = [this, drawModeBtn]()
        {
            m_drawMode = !m_drawMode;

            // If switching on, clear any old player path
            if (m_drawMode)
            {
                // safest: stop A* visuals so colors don't mix
                if (m_aState == AStarRunState::Running || m_aState == AStarRunState::Paused)
                    cancelAStar();

                clearPlayerPath();
                m_status = "Draw Mode: ON (drag from Start to Goal).";
            }
            else
            {
                endPlayerDrawing();
                m_status = "Draw Mode: OFF (LMB toggles walls).";
            }

            drawModeBtn->text = std::string("Draw Mode ") + (m_drawMode ? "ON" : "OFF");
        };
    
    by += (drawModeH + gap);

    // --- Run/Pause and Step button  ---
    float controlH = bh * 0.95f;
    float controlW = (bw - gap) / 2.0f;

    float controlLeftEdge = bx - bw * 0.5f;
    float cx1 = controlLeftEdge + controlW * 0.5f;
    float cx2 = cx1 + controlW + gap;

    float yControls = by;

    addBtnAt(cx1, yControls, controlW, controlH, "Run/Pause", "blue", [this]()
        {
            if (m_aState == AStarRunState::Idle || m_aState == AStarRunState::Found || m_aState == AStarRunState::NoPath)
                startAStar();
            else if (m_aState == AStarRunState::Running)
                m_aState = AStarRunState::Paused;
            else if (m_aState == AStarRunState::Paused)
                m_aState = AStarRunState::Running;
        });

    addBtnAt(cx2, yControls, controlW, controlH, "Step A*", "blue", [this]()
        {
            if (m_aState == AStarRunState::Idle || m_aState == AStarRunState::Found || m_aState == AStarRunState::NoPath)
            {
                startAStar();
                m_aState = AStarRunState::Paused;
            }
            if (m_aState == AStarRunState::Running) m_aState = AStarRunState::Paused;

            if (m_aState == AStarRunState::Paused)
                stepAStar();
        });

    by += (controlH + gap);

    // --- Speed and Random row (2 buttons on one line) ---
    float speedRandomH = bh * 0.95f;
    float speedRandomW = (bw - gap) / 2.0f;

    float speedRandomLeftEdge = bx - bw * 0.5f;
    float sx1 = speedRandomLeftEdge + speedRandomW * 0.5f;
    float sx2 = sx1 + speedRandomW + gap;

    float ySpeedRandom = by;

    Button* speedBtn = addBtnAt(sx1, ySpeedRandom, speedRandomW, speedRandomH, "Speed x1", "orange", []() {});
    speedBtn->onClick = [this, speedBtn]()
        {
            m_speedIndex = (m_speedIndex + 1) % 7;
            m_stepDelayMs = SPEED_LEVELS[m_speedIndex];

            static const char* labels[7] = { "x0.5", "x0.75", "x1", "x1.5", "x2", "x3", "x5" };
            speedBtn->text = std::string("Speed ") + labels[m_speedIndex];
        };

    addBtnAt(sx2, ySpeedRandom, speedRandomW, speedRandomH, "Random Walls", "orange", [this]()
        {
            cancelAStar();
            resetScore();
            for (Node* n : m_grid.getAllNodes())
            {
                if (n == m_start || n == m_goal) continue;
                // 20% walls
                bool wall = (rand() % 100) < 20;
                n->walkable = !wall;
                n->state = wall ? NodeVizState::Wall : NodeVizState::Empty;
            }
        });

    by += (speedRandomH + gap);

    // --- Level row ---
    float levelH = bh * 0.80f;
    float levelW = (bw - 2.0f * gap) / 3.0f;

    float leftEdge = bx - bw * 0.5f;
    float x1 = leftEdge + levelW * 0.5f;
    float x2 = x1 + levelW + gap;
    float x3 = x2 + levelW + gap;

    float yLevels = by;

    addBtnAt(x1, yLevels, levelW, levelH, "Easy", "green", [this]() { loadNextLevel(0); });
    addBtnAt(x2, yLevels, levelW, levelH, "Medium", "orange", [this]() { loadNextLevel(1); });
    addBtnAt(x3, yLevels, levelW, levelH, "Hard", "red", [this]() { loadNextLevel(2); });

    by += (levelH + gap);

    // --- Reset/Clear row  ---
    float actionH = bh * 0.95f;
    float actionW = (bw - gap) / 2.0f;

    float actionLeftEdge = bx - bw * 0.5f;
    float ax1 = actionLeftEdge + actionW * 0.5f;
    float ax2 = ax1 + actionW + gap;

    float yActions = by;

    addBtnAt(ax1, yActions, actionW, actionH, "Reset A*", "red", [this]()
        {
            cancelAStar();
            resetScore();
        });

    addBtnAt(ax2, yActions, actionW, actionH, "Clear Walls", "red", [this]()
        {
            cancelAStar();
            clearWallsAndPathKeepEndpoints();
            resetScore();
        });

    by += (actionH + gap);

    // --- Toggle buttons row ---
    float toggleH = bh * 0.95f;
    float toggleW = (bw - 2.0f * gap) / 3.0f;

    float toggleLeftEdge = bx - bw * 0.5f;
    float tx1 = toggleLeftEdge + toggleW * 0.5f;
    float tx2 = tx1 + toggleW + gap;
    float tx3 = tx2 + toggleW + gap;

    float yToggles = by;

    Button* hintBtn = addBtnAt(tx1, yToggles, toggleW, toggleH, "Hint ON", "pink", []() {});
    hintBtn->padX = 8.0f;
    hintBtn->onClick = [this, hintBtn]()
        {
            m_showShortestHint = !m_showShortestHint;
            hintBtn->text = std::string("Hint ") + (m_showShortestHint ? "ON" : "OFF");
        };

    Button* helpBtn = addBtnAt(tx2, yToggles, toggleW, toggleH, "Help", "pink", []() {});
    helpBtn->onClick = [this]()
        {
            m_showHelp = !m_showHelp;
        };

    Button* musicBtn = addBtnAt(tx3, yToggles, toggleW, toggleH, "Music ON", "pink", []() {});
    musicBtn->padX = 8.0f;
    musicBtn->onClick = [this, musicBtn]()
        {
            m_musicOn = !m_musicOn;

            if (m_musicOn)
            {
                graphics::playMusic(m_musicFile, m_musicVolume, true, 600);
            }
            else
            {
                graphics::stopMusic(600);
            }

            musicBtn->text = std::string("Music ") + (m_musicOn ? "ON" : "OFF");
        };

    by += (toggleH + gap);

    // --- Guide Panel ---
    const float panelMargin = 6.0f;
    const float panelTop = by + 2.0f;
    const float panelBottom = WIN_H - panelMargin;

    const float legendW = bw;
    const float legendH = panelBottom - panelTop;
    const float legendCy = (panelTop + panelBottom) * 0.5f;

    m_legend = new LegendPanel(
        bx,
        legendCy,
        legendW,
        legendH
    );
    m_ui.push_back(m_legend);

}
