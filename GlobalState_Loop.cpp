#include "GlobalState.h"
#include "UIConstants.h"
#include "graphics.h"
#include "scancodes.h"

void GlobalState::update(float dt)
{
        (void)dt;

        graphics::MouseState ms{};
        graphics::getMouseState(ms);

        updateAttemptTimer(dt);

        // --- key edge detection ---
        bool spaceDown = graphics::getKeyState(graphics::SCANCODE_SPACE);
        bool rDown = graphics::getKeyState(graphics::SCANCODE_R);
        bool dDown = graphics::getKeyState(graphics::SCANCODE_D);
        bool dPressed = dDown && !m_prevD;
        m_prevD = dDown;

        bool spacePressed = spaceDown && !m_prevSpace;
        bool rPressed = rDown && !m_prevR;

        m_prevSpace = spaceDown;
        m_prevR = rDown;

        const float mx = graphics::windowToCanvasX((float)ms.cur_pos_x);
        const float my = graphics::windowToCanvasY((float)ms.cur_pos_y);

        // Handle help window close button click
        if (m_showHelp && ms.button_left_pressed)
        {
            const float closeBtnSize = HELP_CLOSE_BTN_SIZE;
            const float closeBtnX = HELP_PANEL_CX + HELP_PANEL_W * 0.5f - closeBtnSize * 0.5f - HELP_CLOSE_BTN_MARGIN;
            const float closeBtnY = HELP_PANEL_CY - HELP_PANEL_H * 0.5f + closeBtnSize * 0.5f + HELP_CLOSE_BTN_MARGIN;

            const float half = closeBtnSize * 0.5f;
            const float minX = closeBtnX - half;
            const float maxX = closeBtnX + half;
            const float minY = closeBtnY - half;
            const float maxY = closeBtnY + half;
            
            // Check if click is within close button bounds
            if (mx >= minX && mx <= maxX && my >= minY && my <= maxY)
            {
                m_showHelp = false;
                return;
            }
        }

        bool uiCaptured = false;
        // Don't process UI if help is showing
        if (!m_showHelp)
        {
            for (UIWidget* w : m_ui)
            {
                if (w && w->visible)
                    uiCaptured = w->handleInput(ms, mx, my) || uiCaptured;
            }
        }

        if (m_showHelp)
        {
            return;
        }
        
        if (!uiCaptured)
        {
            // If A* is running and player starts editing/drawing, stop A*
            auto stopAStarIfActive = [this]()
                {
                    if (m_aState == AStarRunState::Running || m_aState == AStarRunState::Paused) {
                        cancelAStar();
                        resetAttemptTimer();
                        resetScore();
                    }

                };

            // --- LMB behavior ---
            if (!m_drawMode)
            {
                // NORMAL MODE: LMB toggles walls
                if (ms.button_left_pressed)
                {
                    stopAStarIfActive();
                    Node* n = nodeFromMouse(mx, my);
                    if (n && n != m_start && n != m_goal)
                    {
                        n->walkable = !n->walkable;
                        n->state = n->walkable ? NodeVizState::Empty : NodeVizState::Wall;
                    }
                }
            }
            else
            {
                // DRAW MODE: LMB draws a player path
                if (ms.button_left_pressed)
                {
                    stopAStarIfActive();
                    beginPlayerDrawing();
                    startAttemptTimer();
                    // If user clicked somewhere other than start, we still begin at start.
                }

                if (ms.button_left_down && m_isDrawing)
                {
                    Node* n = nodeFromMouse(mx, my);

                    // Only attempt when the hovered node changes
                    if (n && n != m_lastDrawn)
                    {
                        // allow the first step from start
                        if (m_playerPath.empty())
                        {
                            beginPlayerDrawing();
                        }

                        if (appendPlayerPath(n))
                        {
                            m_lastDrawn = n;
                        }
                        else
                        {
                            // keep last drawn to prevent spamming the same invalid cell
                            m_lastDrawn = n;
                        }
                    }
                }

                if (ms.button_left_released)
                {
                    endPlayerDrawing();
                    stopAttemptTimer();


                    m_pathValidation = validatePlayerPath();

                    switch (m_pathValidation)
                    {
                    case PlayerPathValidation::Empty:
                        m_status = "Draw Mode: draw a path from Start to Goal.";
                        break;

                    case PlayerPathValidation::ValidToGoal:
                        m_status = "Valid path to GOAL!";
                        break;

                    case PlayerPathValidation::ValidButNotAtGoal:
                        m_status = "Path is valid so far, but it doesn't reach the GOAL.";
                        break;

                    case PlayerPathValidation::InvalidStart:
                        m_status = "Invalid path: it must start at the green Start cell.";
                        break;

                    case PlayerPathValidation::InvalidWall:
                        m_status = "Invalid path: it goes through a wall.";
                        break;

                    case PlayerPathValidation::InvalidNonAdjacent:
                        m_status = "Invalid path: it must move only to adjacent cells.";
                        break;

                    case PlayerPathValidation::InvalidDuplicate:
                        m_status = "Invalid path: you revisited a cell.";
                        break;
                    }

                    if (m_pathValidation == PlayerPathValidation::ValidToGoal)
                    {
                        computeScore();
                    }
                }

            }

            if (ms.button_right_pressed && !ms.button_left_down && !m_isDrawing)
            {
                stopAStarIfActive();

                bool shift = graphics::getKeyState(graphics::SCANCODE_LSHIFT) ||
                    graphics::getKeyState(graphics::SCANCODE_RSHIFT);

                Node* n = nodeFromMouse(mx, my);
                if (n && n->walkable)
                {
                    clearPlayerPath();

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
            resetScore();
        }
        if (dPressed)
        {
            m_drawMode = !m_drawMode;

            if (m_drawMode)
            {
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
