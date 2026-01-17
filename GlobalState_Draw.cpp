#include "GlobalState.h"
#include "graphics.h"
#include "UIConstants.h"
#include <cmath>

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

    const float panelMargin = 6.0f;
    const float panelTop = UI_TOP_H + panelMargin;
    const float panelBottom = WIN_H - panelMargin;
    const float panelH = panelBottom - panelTop;
    const float panelCy = (panelTop + panelBottom) * 0.5f;

    drawSidebar(UI_LEFT_W * 0.5f, panelCy,
        UI_LEFT_W - 2.0f * panelMargin, panelH,
        "Controls");

    // Background
    graphics::Brush bg;
    bg.outline_opacity = 0.0f;
    bg.fill_opacity = 1.0f;
    bg.fill_color[0] = 0.65f;
    bg.fill_color[1] = 0.65f;
    bg.fill_color[2] = 0.65f;

    bg.texture = "assets/bg.png";
    graphics::drawRect(WIN_W * 0.5f, WIN_H * 0.5f, (float)WIN_W, (float)WIN_H, bg);

    for (const VisualAsset* a : m_drawables)
        a->draw();

    drawShortestHintOverlay();

    // Draw UI elements ONLY if help is NOT showing
    if (!m_showHelp)
    {
        for (const UIWidget* w : m_ui)
            if (w) w->draw();
    }


    // Top HUD bar
    graphics::Brush bar;
    bar.fill_color[0] = 0.05f; bar.fill_color[1] = 0.06f; bar.fill_color[2] = 0.08f;
    bar.fill_opacity = 0.85f;
    bar.outline_opacity = 0.0f;
    graphics::drawRect(512.0f, 40.0f, 1024.0f, 80.0f, bar);

    graphics::Brush t;
    t.fill_color[0] = 1.0f; t.fill_color[1] = 1.0f; t.fill_color[2] = 1.0f;
    graphics::setFont(m_fontTitle);
    graphics::drawText(18.0f, 48.0f, 26.0f, m_title, t);
    graphics::setFont(m_fontUI);
    std::string stats =
            "Score " + std::to_string(m_score) +
            " | overlap " + std::to_string((int)std::round(m_overlap * 100.0f)) + "%" +
            " | eff " + std::to_string((int)std::round(m_efficiency * 100.0f)) + "%" +
            " | shortest " + std::to_string(m_shortestSteps) +
            " | yours " + std::to_string(m_playerSteps);
    std::string line = (m_score > 0) ? stats : m_status;
    graphics::drawText(350.0f, 50.0f, 23.0f, line, t);

    std::string timeStr = m_timerEnabled
        ? ("Time: " + formatTime(m_timerRunning ? m_attemptTimeMs : m_lastAttemptMs))
        : "Time: OFF";

    std::string pointsStr =
        "Points: " + std::to_string(m_finalPoints);

    graphics::drawText(900.0f, 28.0f, 20.0f, timeStr, t);
    graphics::drawText(900.0f, 48.0f, 20.0f, pointsStr, t);
        
    if (m_timeBonus != 0)
    {
        std::string bonusStr = "+ " + std::to_string(m_timeBonus) + " time bonus";
        graphics::drawText(900.0f, 68.0f, 18.0f, bonusStr, t);
    }

    // Help overlay 
    if (m_showHelp)
    {
        // Full screen dark overlay
        graphics::Brush ov;
        ov.fill_color[0] = 0.0f; ov.fill_color[1] = 0.0f; ov.fill_color[2] = 0.0f;
        ov.fill_opacity = 0.70f;
        ov.outline_opacity = 0.0f;
        graphics::drawRect(512.0f, 384.0f, 1024.0f, 768.0f, ov);

        // Help panel
        graphics::Brush p;
        p.fill_color[0] = 0.14f; p.fill_color[1] = 0.16f; p.fill_color[2] = 0.20f;
        p.fill_opacity = 1.0f;
        p.outline_opacity = 1.0f;
        p.outline_width = 2.0f;
        p.outline_color[0] = 0.25f; p.outline_color[1] = 0.70f; p.outline_color[2] = 0.95f;
        graphics::drawRect(512.0f, 384.0f, 760.0f, 580.0f, p);

        // Close button in top-right corner of help panel
        float closeBtnSize = 32.0f;
        float closeBtnX = 512.0f + 760.0f * 0.5f - closeBtnSize * 0.5f - 10.0f;
        float closeBtnY = 384.0f - 560.0f * 0.5f + closeBtnSize * 0.5f + 10.0f;
            
        graphics::Brush closeBtn;
        closeBtn.fill_color[0] = 0.90f; closeBtn.fill_color[1] = 0.25f; closeBtn.fill_color[2] = 0.25f;
        closeBtn.fill_opacity = 0.9f;
        closeBtn.outline_opacity = 1.0f;
        closeBtn.outline_width = 1.0f;
        closeBtn.outline_color[0] = 1.0f; closeBtn.outline_color[1] = 1.0f; closeBtn.outline_color[2] = 1.0f;
        graphics::drawRect(closeBtnX, closeBtnY, closeBtnSize, closeBtnSize, closeBtn);

        graphics::Brush closeTxt;
        closeTxt.fill_color[0] = 1.0f; closeTxt.fill_color[1] = 1.0f; closeTxt.fill_color[2] = 1.0f;
        closeTxt.outline_opacity = 0.0f;
        graphics::drawText(closeBtnX - 8.0f, closeBtnY + 8.0f, 24.0f, "X", closeTxt);

        // Help text
        graphics::Brush txt;
        txt.fill_color[0] = 1.0f; txt.fill_color[1] = 1.0f; txt.fill_color[2] = 1.0f;
        txt.outline_opacity = 0.0f;
        auto drawBold = [&](float xx, float yy, float size, const std::string& s, const graphics::Brush& b)
            {
                graphics::drawText(xx, yy, size, s, b);
                graphics::drawText(xx + 1.0f, yy, size, s, b);     // thicker look
            };
        float x = 170.0f, y = 150.0f;
        graphics::drawText(x, y, 28.0f, "StarPath Quest - Mission Briefing", txt); y += 38.0f;

        graphics::drawText(x, y, 18.0f,
            "Welcome, Navigator! Your mission is to guide the explorer from START to GOAL.", txt); y += 28.0f;

        graphics::drawText(x, y, 18.0f,
            "You can build walls, watch A* search, or draw your own route for a score.", txt); y += 30.0f;
        graphics::drawText(x, y, 18.0f,
            "Keep in mind of the time when drawing! It can give you better score.", txt); y += 30.0f;
        graphics::drawText(x, y, 18.0f,
            "Try to always find the shorter path. Use the HINT button for help.", txt); y += 30.0f;

        // --- Button section title
        graphics::drawText(x, y, 20.0f, "Buttons", txt); y += 30.0f;

        // Align labels + descriptions in two columns
        float labelX = x;
        float descX = x + 260.0f;
        graphics::Brush dimTxt = txt;
        dimTxt.fill_opacity = 0.85f;

        drawBold(labelX, y, 16.0f, "Draw", txt);
        graphics::drawText(descX, y, 16.0f, "Enable Draw Mode. Drag your mouse from Start to Goal.", dimTxt); y += 22.0f;

        drawBold(labelX, y, 16.0f, "Run / Pause", txt);
        graphics::drawText(descX, y, 16.0f, "Start or pause A* search step-by-step.", dimTxt); y += 22.0f;

        drawBold(labelX, y, 16.0f, "Step", txt);
        graphics::drawText(descX, y, 16.0f, "Advance A* by one steop.", dimTxt); y += 22.0f;

        drawBold(labelX, y, 16.0f, "Speed", txt);
        graphics::drawText(descX, y, 16.0f, "Change A* animation speed.", dimTxt); y += 22.0f;

        drawBold(labelX, y, 16.0f, "Random", txt);
        graphics::drawText(descX, y, 16.0f, "Add Random Walls.", dimTxt); y += 22.0f;

        drawBold(labelX, y, 16.0f, "Reset A*", txt);
        graphics::drawText(descX, y, 16.0f, "Reset A* search.", dimTxt); y += 22.0f;

        drawBold(labelX, y, 16.0f, "Clear Walls", txt);
        graphics::drawText(descX, y, 16.0f, "Remove walls (keep START/GOAL).", dimTxt); y += 28.0f;

        graphics::drawText(x, y, 20.0f, "Quick Controls", txt); y += 30.0f;

        drawBold(labelX, y, 16.0f, "LMB", txt);
        graphics::drawText(descX, y, 16.0f, "Toggle wall (when Draw Mode is OFF).", dimTxt); y += 22.0f;

        drawBold(labelX, y, 16.0f, "RMB", txt);
        graphics::drawText(descX, y, 16.0f, "Place START.", dimTxt); y += 22.0f;

        drawBold(labelX, y, 16.0f, "Shift+RMB", txt);
        graphics::drawText(descX, y, 16.0f, "Place GOAL.", dimTxt); y += 22.0f;

        drawBold(labelX, y, 16.0f, "SPACE", txt);
        graphics::drawText(descX, y, 16.0f, "Run / Pause A*.", dimTxt); y += 22.0f;

        drawBold(labelX, y, 16.0f, "R", txt);
        graphics::drawText(descX, y, 16.0f, "Reset search visuals.", dimTxt); y += 22.0f;

        drawBold(labelX, y, 16.0f, "D", txt);
        graphics::drawText(descX, y, 16.0f, "Toggle Draw Mode.", dimTxt); y += 28.0f;

        drawBold(labelX, y, 16.0f, "I", txt);
        graphics::drawText(descX, y, 16.0f, "Instant A* search solution.", dimTxt); y += 28.0f;
    }
}

void GlobalState::drawShortestHintOverlay() const
{
    if (!m_showShortestHint) return;
    if (m_shortestSteps < 0) return;
    if (m_onShortest.empty()) return;

    graphics::Brush br;
    br.fill_opacity = 0.0f;
    br.outline_opacity = 0.45f;
    br.outline_width = 2.0f;
    br.outline_color[0] = 0.25f;
    br.outline_color[1] = 0.95f;
    br.outline_color[2] = 0.55f;

    for (Node* n : m_grid.getAllNodes())
    {
        if (!n || !n->walkable) continue;
        if (!m_onShortest[idx(n)]) continue;

        const float cx = m_originX + n->col * m_cell + m_cell * 0.5f;
        const float cy = m_originY + n->row * m_cell + m_cell * 0.5f;

        graphics::drawRect(cx, cy, m_cell - 6.0f, m_cell - 6.0f, br);
    };
}
