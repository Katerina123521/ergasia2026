#include "GlobalState.h"
#include <algorithm>
#include <cmath>

#include "graphics.h"
#include "scancodes.h"

GlobalState::GlobalState()
{
    m_status = "LMB: wall | RMB: start | Shift+RMB: goal | SPACE: A* | C: clear";
}

GlobalState::~GlobalState()
{
    graphics::stopMusic(300);

    // delete UI widgets first (they are NOT in m_nodes)
    for (UIWidget* w : m_ui) delete w;
    m_ui.clear();

    // delete grid nodes
    for (Node* n : m_nodes) delete n;
    m_nodes.clear();

    m_drawables.clear();
}

static constexpr int WIN_W = 1024;
static constexpr int WIN_H = 768;

static constexpr float UI_LEFT_W = 260.0f;
static constexpr float UI_RIGHT_W = 260.0f;
static constexpr float UI_TOP_H = 64.0f;
static constexpr float UI_MARGIN = 14.0f;


void GlobalState::init()
{

    const float availW = WIN_W - UI_LEFT_W - UI_RIGHT_W - 2.0f * UI_MARGIN;
    const float availH = WIN_H - UI_TOP_H - 2.0f * UI_MARGIN;

    m_cell = std::min(availW / m_cols, availH / m_rows);
    // Center grid in the window
    const float gridW = m_cols * m_cell;
    const float gridH = m_rows * m_cell;
     
    //m_originX = (WIN_W - gridW) * 0.5f;
    //m_originY = (WIN_H - gridH) * 0.5f;

    m_originX = UI_LEFT_W + UI_MARGIN + (availW - gridW) * 0.5f;
    m_originY = UI_TOP_H + UI_MARGIN + (availH - gridH) * 0.5f;

    Node::setLayout(m_originX, m_originY, m_cell);

    buildGridGraph();

    // Default start/goal
    m_start = nodeAt(m_rows / 2, 2);
    m_goal = nodeAt(m_rows / 2, m_cols - 3);

    if (m_start) m_start->state = NodeVizState::Start;
    if (m_goal)  m_goal->state = NodeVizState::Goal;

    graphics::setFont("assets/orbitron.ttf");
    if (m_musicOn)
        graphics::playMusic(m_musicFile, m_musicVolume, true, 800);



    // --- UI: left control panel buttons ---
    float bw = UI_LEFT_W - 2.0f * UI_MARGIN;
    float bx = UI_LEFT_W * 0.5f;            // center of left sidebar
    float by = UI_TOP_H + 90.0f;
    float bh = 42.0f;

    float gap = 10.0f;

    auto addBtn = [&](const std::string& txt, std::function<void()> cb)
        {
            Button* b = new Button(bx, by, bw, bh, txt, cb);
            b->clickSound = "assets/hit1.wav";
            b->clickVolume = 0.20f;
            m_ui.push_back(b);
            by += (bh + gap);
            return b;
        };

    // Run/Pause
    addBtn("Run / Pause", [this]()
        {
            // Use your step-by-step state machine here:
            // Idle/Found/NoPath -> startAStar()
            // Running -> Paused
            // Paused -> Running
            if (m_aState == AStarRunState::Idle || m_aState == AStarRunState::Found || m_aState == AStarRunState::NoPath)
                startAStar();
            else if (m_aState == AStarRunState::Running)
                m_aState = AStarRunState::Paused;
            else if (m_aState == AStarRunState::Paused)
                m_aState = AStarRunState::Running;
        });

    // Step (one expansion)
    addBtn("Step (one node)", [this]()
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

    // Reset search (keep walls + endpoints)
    addBtn("Reset Search", [this]()
        {
            cancelAStar(); // should keep walls + start/goal, clear Open/Closed/Path
        });

    // Clear all (walls + path), keep endpoints
    addBtn("Clear Walls", [this]()
        {
            cancelAStar();
            clearWallsAndPathKeepEndpoints();
        });

    // Random walls (nice for quick demos)
    addBtn("Random Walls", [this]()
        {
            cancelAStar();
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

    // Speed -
    addBtn("Speed -", [this]()
        {
            m_stepDelayMs = std::min(200.0f, m_stepDelayMs + 10.0f);
        });

    // Speed +
    addBtn("Speed +", [this]()
        {
            m_stepDelayMs = std::max(5.0f, m_stepDelayMs - 10.0f);
        });

    addBtn("Music ON/OFF", [this]()
        {
            m_musicOn = !m_musicOn;

            if (m_musicOn)
            {
                graphics::playMusic(m_musicFile, m_musicVolume, true, 600);
                m_status = "Music: ON";
            }
            else
            {
                graphics::stopMusic(600);
                m_status = "Music: OFF";
            }
        });


    // Help toggle
    ToggleButton* helpBtn = new ToggleButton(bx, by, bw, bh, "Help", &m_showHelp);
    helpBtn->clickSound = "assets/hit1.wav";
    helpBtn->clickVolume = 0.20f;
    m_ui.push_back(helpBtn);
    by += (bh + gap);

    m_legend = new LegendPanel(
        WIN_W - UI_RIGHT_W * 0.5f,
        UI_TOP_H + 220.0f,
        UI_RIGHT_W - 2.0f * UI_MARGIN,
        320.0f
    );
    m_ui.push_back(m_legend);

}

Node* GlobalState::nodeAt(int r, int c) const
{
    if (r < 0 || r >= m_rows || c < 0 || c >= m_cols) return nullptr;
    return m_nodes[r * m_cols + c];
}

void GlobalState::buildGridGraph()
{
    // Create nodes (dynamic memory)
    m_nodes.reserve(m_rows * m_cols);
    for (int r = 0; r < m_rows; r++)
    {
        for (int c = 0; c < m_cols; c++)
        {
            Node* n = new Node(r, c);
            m_nodes.push_back(n);

            // add to polymorphic list
            m_drawables.push_back(n);
        }
    }

    // 4-neighborhood edges (graph)
    for (int r = 0; r < m_rows; r++)
    {
        for (int c = 0; c < m_cols; c++)
        {
            Node* n = nodeAt(r, c);
            if (!n) continue;

            Node* up = nodeAt(r - 1, c);
            Node* down = nodeAt(r + 1, c);
            Node* left = nodeAt(r, c - 1);
            Node* right = nodeAt(r, c + 1);

            if (up)    n->neighbors.push_back(up);
            if (down)  n->neighbors.push_back(down);
            if (left)  n->neighbors.push_back(left);
            if (right) n->neighbors.push_back(right);
        }
    }
}

float GlobalState::heuristic(const Node* a, const Node* b) const
{
    // Manhattan distance works well on 4-neighborhood grids
    return float(std::abs(a->row - b->row) + std::abs(a->col - b->col));
}

Node* GlobalState::nodeFromMouse(float mx, float my) const
{
    for (Node* n : m_nodes)
        if (n->hitTest(mx, my)) return n;
    return nullptr;
}

void GlobalState::resetSearchVisuals()
{
    for (Node* n : m_nodes)
    {
        // Preserve walls + endpoints
        if (n == m_start)
        {
            n->walkable = true;
            n->state = NodeVizState::Start;
            n->g = 1e9f; n->h = 0.0f; n->f = 1e9f; n->parent = nullptr;
            continue;
        }
        if (n == m_goal)
        {
            n->walkable = true;
            n->state = NodeVizState::Goal;
            n->g = 1e9f; n->h = 0.0f; n->f = 1e9f; n->parent = nullptr;
            continue;
        }

        n->resetSearchData();
    }
}

void GlobalState::clearWallsAndPathKeepEndpoints()
{
    for (Node* n : m_nodes)
    {
        n->walkable = true;
        n->state = NodeVizState::Empty;
        n->parent = nullptr;
        n->g = 1e9f; n->h = 0.0f; n->f = 1e9f;
    }

    if (m_start) m_start->state = NodeVizState::Start;
    if (m_goal)  m_goal->state = NodeVizState::Goal;
}

bool GlobalState::runAStar()
{
    if (!m_start || !m_goal) return false;

    resetSearchVisuals();

    std::vector<Node*> open;
    open.reserve(m_rows * m_cols);

    m_start->g = 0.0f;
    m_start->h = heuristic(m_start, m_goal);
    m_start->f = m_start->g + m_start->h;

    open.push_back(m_start);

    while (!open.empty())
    {
        // pick min-f node (simple O(n), ok for small grid)
        auto itMin = std::min_element(open.begin(), open.end(),
            [](const Node* a, const Node* b) { return a->f < b->f; });

        Node* current = *itMin;
        open.erase(itMin);

        if (current != m_start && current != m_goal)
            current->state = NodeVizState::Closed;

        if (current == m_goal)
        {
            // reconstruct path
            Node* p = m_goal->parent;
            while (p && p != m_start)
            {
                p->state = NodeVizState::Path;
                p = p->parent;
            }
            return true;
        }

        for (Node* nb : current->neighbors)
        {
            if (!nb->walkable) continue;
            if (nb->state == NodeVizState::Closed) continue;

            const float tentative_g = current->g + 1.0f;

            const bool inOpen = (std::find(open.begin(), open.end(), nb) != open.end());
            if (!inOpen || tentative_g < nb->g)
            {
                nb->parent = current;
                nb->g = tentative_g;
                nb->h = heuristic(nb, m_goal);
                nb->f = nb->g + nb->h;

                if (!inOpen)
                {
                    open.push_back(nb);
                    if (nb != m_start && nb != m_goal)
                        nb->state = NodeVizState::Open;
                }
            }
        }
    }

    return false;
}

void GlobalState::cancelAStar()
{
    m_aState = AStarRunState::Idle;
    m_open.clear();
    m_stepAccumMs = 0.0f;

    // καθάρισε Open/Closed/Path αλλά κράτα walls + start/goal
    resetSearchVisuals();

    m_status = "A*: stopped. SPACE: start/pause | R: reset search";
}

void GlobalState::startAStar()
{
    if (!m_start || !m_goal) return;

    resetSearchVisuals();

    m_open.clear();
    m_open.reserve(m_rows * m_cols);

    m_start->g = 0.0f;
    m_start->h = heuristic(m_start, m_goal);
    m_start->f = m_start->g + m_start->h;
    m_start->parent = nullptr;

    m_open.push_back(m_start);

    m_stepAccumMs = 0.0f;
    m_aState = AStarRunState::Running;
    m_status = "A*: running (step-by-step)... SPACE pause/resume | R reset";
}

bool GlobalState::stepAStar()
{
    if (!m_start || !m_goal) return true;

    if (m_open.empty())
    {
        m_aState = AStarRunState::NoPath;
        m_status = "A*: No path (open list empty).";
        return true;
    }

    // pick min-f node
    auto itMin = std::min_element(m_open.begin(), m_open.end(),
        [](const Node* a, const Node* b)
        {
            if (a->f == b->f) return a->h < b->h; // tie-breaker
            return a->f < b->f;
        });

    Node* current = *itMin;
    m_open.erase(itMin);

    if (current != m_start && current != m_goal)
        current->state = NodeVizState::Closed;

    // goal reached
    if (current == m_goal)
    {
        Node* p = m_goal->parent;
        while (p && p != m_start)
        {
            p->state = NodeVizState::Path;
            p = p->parent;
        }

        m_aState = AStarRunState::Found;
        m_status = "A*: Path found!";
        return true;
    }

    // relax neighbors (1 expansion step)
    for (Node* nb : current->neighbors)
    {
        if (!nb->walkable) continue;
        if (nb->state == NodeVizState::Closed) continue;

        const float tentative_g = current->g + 1.0f;

        const bool inOpen = (std::find(m_open.begin(), m_open.end(), nb) != m_open.end());
        if (!inOpen || tentative_g < nb->g)
        {
            nb->parent = current;
            nb->g = tentative_g;
            nb->h = heuristic(nb, m_goal);
            nb->f = nb->g + nb->h;

            if (!inOpen)
            {
                m_open.push_back(nb);
                if (nb != m_start && nb != m_goal)
                    nb->state = NodeVizState::Open;
            }
        }
    }

    return false; // not finished yet
}

void GlobalState::update(float dt)
{
    (void)dt;

    // Polymorphic update call (even if nodes do nothing)
    for (VisualAsset* a : m_drawables)
        a->update(dt);

    graphics::MouseState ms;
    graphics::getMouseState(ms);

    // --- key edge detection ---
    bool spaceDown = graphics::getKeyState(graphics::SCANCODE_SPACE);
    bool rDown = graphics::getKeyState(graphics::SCANCODE_R);

    bool spacePressed = spaceDown && !m_prevSpace;
    bool rPressed = rDown && !m_prevR;

    m_prevSpace = spaceDown;
    m_prevR = rDown;


    const float mx = graphics::windowToCanvasX((float)ms.cur_pos_x);
    const float my = graphics::windowToCanvasY((float)ms.cur_pos_y);

    bool uiCaptured = false;
    for (UIWidget* w : m_ui)
    {
        if (w && w->visible)
            // If a widget returns true, it means:
            // "I used this mouse input, don't pass it to the grid."
            uiCaptured = w->handleInput(ms, mx, my) || uiCaptured;
    }


    const bool shift = graphics::getKeyState(graphics::SCANCODE_LSHIFT) ||
        graphics::getKeyState(graphics::SCANCODE_RSHIFT);
    if (!uiCaptured)
    {
        if (ms.button_left_pressed)
        {
            Node* n = nodeFromMouse(mx, my);
            if (n && n != m_start && n != m_goal)
            {
                // toggle wall
                n->walkable = !n->walkable;
                n->state = n->walkable ? NodeVizState::Empty : NodeVizState::Wall;
            }
        }

        if (ms.button_right_pressed)
        {
            Node* n = nodeFromMouse(mx, my);
            if (n && n->walkable)
            {
                if (shift)
                {
                    if (m_goal) m_goal->state = NodeVizState::Empty;
                    m_goal = n;
                    m_goal->state = NodeVizState::Goal;
                }
                else
                {
                    if (m_start) m_start->state = NodeVizState::Empty;
                    m_start = n;
                    m_start->state = NodeVizState::Start;
                }
            }
        }
    }

    if (spacePressed)
    {
        if (m_aState == AStarRunState::Idle || m_aState == AStarRunState::Found || m_aState == AStarRunState::NoPath)
        {
            startAStar();
        }
        else if (m_aState == AStarRunState::Running)
        {
            m_aState = AStarRunState::Paused;
            m_status = "A*: paused. SPACE resume | R reset";
        }
        else if (m_aState == AStarRunState::Paused)
        {
            m_aState = AStarRunState::Running;
            m_status = "A*: running (step-by-step)... SPACE pause | R reset";
        }
    }

    if (rPressed)
    {
        cancelAStar(); 
    }

    if (graphics::getKeyState(graphics::SCANCODE_C))
    {
        clearWallsAndPathKeepEndpoints();
        m_status = "Cleared. LMB: wall | RMB: start | Shift+RMB: goal | SPACE: A* | C: clear";
    }

    if (graphics::getKeyState(graphics::SCANCODE_I))
    {
        bool ok = runAStar();
        m_status = ok ? "Instant A*: Path found!" : "Instant A*: No path.";
    }

    if (m_aState == AStarRunState::Running)
    {
        m_stepAccumMs += dt;
        if (m_stepAccumMs >= m_stepDelayMs)
        {
            m_stepAccumMs -= m_stepDelayMs;
            stepAStar(); // one expansion per step
        }
    }


}

void GlobalState::draw() const
{
    auto drawSidebar = [](float cx, float cy, float w, float h, const char* title)
        {
            // shadow
            graphics::Brush sh;
            sh.fill_color[0] = 0.0f; sh.fill_color[1] = 0.0f; sh.fill_color[2] = 0.0f;
            sh.fill_opacity = 0.25f;
            sh.outline_opacity = 0.0f;
            graphics::drawRect(cx + 4.0f, cy + 4.0f, w, h, sh);

            // panel
            graphics::Brush br;
            br.gradient = true;
            br.gradient_dir_u = 0.0f; br.gradient_dir_v = 1.0f;
            br.fill_opacity = 0.95f;
            br.fill_secondary_opacity = 0.95f;

            br.fill_color[0] = 0.10f; br.fill_color[1] = 0.11f; br.fill_color[2] = 0.14f;
            br.fill_secondary_color[0] = 0.07f; br.fill_secondary_color[1] = 0.08f; br.fill_secondary_color[2] = 0.11f;

            br.outline_opacity = 1.0f;
            br.outline_width = 1.0f;
            br.outline_color[0] = 0.15f; br.outline_color[1] = 0.16f; br.outline_color[2] = 0.20f;

            graphics::drawRect(cx, cy, w, h, br);


            // thin separator line
            graphics::Brush line;
            line.fill_color[0] = 0.25f; line.fill_color[1] = 0.70f; line.fill_color[2] = 0.95f;
            line.fill_opacity = 0.35f;
            line.outline_opacity = 0.0f;
            graphics::drawRect(cx, cy - h * 0.5f + 44.0f, w - 32.0f, 2.0f, line);
        };

    // left and right panels
    drawSidebar(UI_LEFT_W * 0.5f, UI_TOP_H + (WIN_H - UI_TOP_H) * 0.5f,
        UI_LEFT_W - 2.0f * UI_MARGIN, WIN_H - UI_TOP_H - 2.0f * UI_MARGIN,
        "Controls");

    drawSidebar(WIN_W - UI_RIGHT_W * 0.5f, UI_TOP_H + (WIN_H - UI_TOP_H) * 0.5f,
        UI_RIGHT_W - 2.0f * UI_MARGIN, WIN_H - UI_TOP_H - 2.0f * UI_MARGIN,
        "Legend");


    // Background
    graphics::Brush bg;
    bg.fill_color[0] = 0.12f; bg.fill_color[1] = 0.12f; bg.fill_color[2] = 0.14f;
    bg.fill_opacity = 1.0f;
    graphics::drawRect(WIN_W * 0.5f, WIN_H * 0.5f, (float)WIN_W, (float)WIN_H, bg);

    // Polymorphic draw call
    for (const VisualAsset* a : m_drawables)
        a->draw();

    for (UIWidget* w : m_ui)
        if (w) w->draw();

    // Top HUD bar
    graphics::Brush bar;
    bar.fill_color[0] = 0.05f; bar.fill_color[1] = 0.06f; bar.fill_color[2] = 0.08f;
    bar.fill_opacity = 0.85f;
    bar.outline_opacity = 0.0f;
    graphics::drawRect(512.0f, 32.0f, 1024.0f, 64.0f, bar); 

    graphics::Brush t;
    t.fill_color[0] = 1.0f; t.fill_color[1] = 1.0f; t.fill_color[2] = 1.0f;

    // move text down so it doesn't touch the top edge
    graphics::drawText(18.0f, 48.0f, 26.0f, m_title, t);
    graphics::drawText(300.0f, 48.0f, 18.0f, m_status, t);


    if (m_showHelp)
    {
        graphics::Brush ov;
        ov.fill_color[0] = 0.0f; ov.fill_color[1] = 0.0f; ov.fill_color[2] = 0.0f;
        ov.fill_opacity = 0.55f;
        ov.outline_opacity = 0.0f;
        graphics::drawRect(512.0f, 384.0f, 1024.0f, 768.0f, ov);

        // help panel
        graphics::Brush p;
        p.fill_color[0] = 0.14f; p.fill_color[1] = 0.16f; p.fill_color[2] = 0.20f;
        p.fill_opacity = 0.95f;
        p.outline_opacity = 0.8f;
        p.outline_width = 1.0f;
        p.outline_color[0] = 0.08f; p.outline_color[1] = 0.08f; p.outline_color[2] = 0.10f;
        graphics::drawRect(512.0f, 384.0f, 760.0f, 420.0f, p);

        graphics::Brush txt;
        txt.fill_color[0] = 1.0f; txt.fill_color[1] = 1.0f; txt.fill_color[2] = 1.0f;

        float x = 170.0f, y = 240.0f;
        graphics::drawText(x, y, 26.0f, "How to play", txt); y += 34.0f;
        graphics::drawText(x, y, 18.0f, "Build walls and watch A* explore step-by-step.", txt); y += 26.0f;
        graphics::drawText(x, y, 16.0f, "LMB: toggle wall", txt); y += 20.0f;
        graphics::drawText(x, y, 16.0f, "RMB: set Start", txt); y += 20.0f;
        graphics::drawText(x, y, 16.0f, "Shift+RMB: set Goal", txt); y += 20.0f;
        graphics::drawText(x, y, 16.0f, "Use the buttons: Run/Pause, Step, Reset, Clear, Random.", txt); y += 20.0f;
        graphics::drawText(x, y, 16.0f, "Tip: change Speed to slow down or speed up the animation.", txt);
    }


}
