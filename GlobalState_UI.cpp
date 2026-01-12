#include "GlobalState.h"
#include "UIConstants.h"
#include "graphics.h"
#include "scancodes.h"

void GlobalState::setupUI()
{   
    float bw = UI_LEFT_W - 2.0f * UI_MARGIN;
    float bx = UI_LEFT_W * 0.5f;            // center of left sidebar
    float by = UI_TOP_H + 90.0f;
    float bh = 42.0f;

    float gap = 10.0f;
    // same width as Easy/Medium/Hard buttons (1/3 of panel width)
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

            // προσεγγιστικό πλάτος κειμένου
            float approxTextW = (float)b->text.size() * b->textSize * 0.55f;

            // pad ώστε το text να κάτσει περίπου στο κέντρο
            float pad = (b->w - approxTextW) * 0.5f;

            // μικρό clamp για να μην βγει αρνητικό / να μην κολλήσει στο border
            if (pad < 6.0f) pad = 6.0f;

            b->padX = pad;
        };


    auto addBtn = [&](const std::string& txt, const std::string& theme, const std::string& icon, std::function<void()> cb)
        {
            Button* b = new Button(bx, by, bw, bh, txt, cb);
            b->clickSound = "assets/hit1.wav";
            b->clickVolume = 0.20f;

            b->iconTex = icon;
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
            b->textSize = 18.0f;     // smaller for these
            skinBtn(b, theme);
            m_ui.push_back(b);
            return b;
        };

    auto addSmallBtn = [&](const std::string& txt, const std::string& theme, const std::string& icon, std::function<void()> cb)
        {
            Button* b = new Button(bx, by, smallW, bh, txt, cb);
            b->clickSound = "assets/hit1.wav";
            b->clickVolume = 0.20f;

            b->iconTex = icon;

            // λίγο πιο μικρό κείμενο/padding για να χωράνε καλύτερα
            b->textSize = 16.0f;
            centerTextPad(b);


            skinBtn(b, theme);

            m_ui.push_back(b);
            by += (bh + gap);
            return b;
        };


    addSmallBtn("      Run/Pause", "blue", "", [this]()

        {
            if (m_aState == AStarRunState::Idle || m_aState == AStarRunState::Found || m_aState == AStarRunState::NoPath)
                startAStar();
            else if (m_aState == AStarRunState::Running)
                m_aState = AStarRunState::Paused;
            else if (m_aState == AStarRunState::Paused)
                m_aState = AStarRunState::Running;
        });

    Button* speedBtn = addSmallBtn("       Speed x1", "orange", "", []() {});

    speedBtn->onClick = [this, speedBtn]()
        {
            m_speedIndex = (m_speedIndex + 1) % 7;
            m_stepDelayMs = SPEED_LEVELS[m_speedIndex];

            static const char* labels[7] = { "x0.5", "x0.75", "x1", "x1.5", "x2", "x3", "x5" };
            speedBtn->text = std::string("Speed ") + labels[m_speedIndex];

        };
    centerTextPad(speedBtn);



    addSmallBtn("Step", "blue", "", [this]()

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


    // --- Level row (3 buttons on one line) ---
    float levelH = bh * 0.80f;                 // a bit shorter than normal buttons
    float levelW = (bw - 2.0f * gap) / 3.0f;   // 3 buttons + 2 gaps inside panel width bw

    float leftEdge = bx - bw * 0.5f;           // panel left edge (since bx is panel center)
    float x1 = leftEdge + levelW * 0.5f;
    float x2 = x1 + levelW + gap;
    float x3 = x2 + levelW + gap;

    float yLevels = by;                         // current row y

    addBtnAt(x1, yLevels, levelW, levelH, "Easy", "green", [this]() { loadNextLevel(0); });
    addBtnAt(x2, yLevels, levelW, levelH, "Medium", "orange", [this]() { loadNextLevel(1); });
    addBtnAt(x3, yLevels, levelW, levelH, "Hard", "red", [this]() { loadNextLevel(2); });

    // move the cursor down AFTER the row
    by += (levelH + gap);

    addSmallBtn("Random", "orange", "", [this]()

        {
            cancelAStar();
            resetScore();
            // simple random fill
            for (Node* n : m_nodes)
            {
                if (n == m_start || n == m_goal) continue;
                // ~20% walls
                bool wall = (rand() % 100) < 20;
                n->walkable = !wall;
                n->state = wall ? NodeVizState::Wall : NodeVizState::Empty;
            }
        });

    // Reset search (keep walls + endpoints)
    addSmallBtn("Reset", "red", "", [this]()

        {
            cancelAStar();
            resetScore();
        });

    // Clear all (walls + path), keep endpoints
    addSmallBtn("Clear", "red", "", [this]()

        {
            cancelAStar();
            clearWallsAndPathKeepEndpoints();
            resetScore();
        });


    Button* hintBtn = addSmallBtn("       Hint OFF", "pink", "", []() {});


    hintBtn->onClick = [this, hintBtn]()
        {
            m_showShortestHint = !m_showShortestHint;
            hintBtn->text = std::string("Hint ") + (m_showShortestHint ? "ON" : "OFF");
        };
    centerTextPad(hintBtn);


    Button* helpBtn = addSmallBtn("       Help OFF", "pink", "", []() {});

    helpBtn->onClick = [this, helpBtn]()
        {
            m_showHelp = !m_showHelp;
            helpBtn->text = std::string("Help ") + (m_showHelp ? "ON" : "OFF");
        };
    centerTextPad(helpBtn);


    Button* musicBtn = addSmallBtn("       Music OFF", "pink", "", []() {});

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

            musicBtn->text = std::string("Music ") + (m_showHelp ? "ON" : "OFF");

        };
    centerTextPad(musicBtn);


    const float panelMargin = 6.0f;
    const float panelTop = UI_TOP_H + panelMargin;
    const float panelBottom = WIN_H - panelMargin;

    const float rightPanelW = UI_RIGHT_W - 2.0f * panelMargin;

    // άφησε χώρο για το “header” του panel (πάνω μέρος + γραμμή)
    const float legendTop = panelTop + 70.0f;
    const float legendBottom = panelBottom - 20.0f;

    const float legendH = legendBottom - legendTop;
    const float legendCy = (legendTop + legendBottom) * 0.5f;

    m_legend = new LegendPanel(
        WIN_W - UI_RIGHT_W * 0.5f,
        legendCy,
        rightPanelW,
        legendH
    );
    m_ui.push_back(m_legend);

    
}
