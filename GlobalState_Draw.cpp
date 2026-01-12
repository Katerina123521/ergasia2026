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

        const float panelMargin = 6.0f;                 // μικρότερο margin => πιο “μακρύ” panel
        const float panelTop = UI_TOP_H + panelMargin;
        const float panelBottom = WIN_H - panelMargin;
        const float panelH = panelBottom - panelTop;
        const float panelCy = (panelTop + panelBottom) * 0.5f;

        drawSidebar(UI_LEFT_W * 0.5f, panelCy,
            UI_LEFT_W - 2.0f * panelMargin, panelH,
            "Controls");

        drawSidebar(WIN_W - UI_RIGHT_W * 0.5f, panelCy,
            UI_RIGHT_W - 2.0f * panelMargin, panelH,
            "Legend");



        // Background
        graphics::Brush bg;
        bg.outline_opacity = 0.0f;
        bg.fill_opacity = 1.0f;
        bg.fill_color[0] = 0.65f;
        bg.fill_color[1] = 0.65f;
        bg.fill_color[2] = 0.65f;

        bg.texture = "assets/bg.png";
        graphics::drawRect(WIN_W * 0.5f, WIN_H * 0.5f, (float)WIN_W, (float)WIN_H, bg);

        // Polymorphic draw call
        for (const VisualAsset* a : m_drawables)
            a->draw();

        // overlay hint
        drawShortestHintOverlay();

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
        graphics::setFont(m_fontTitle);
        graphics::drawText(18.0f, 48.0f, 26.0f, m_title, t);
        graphics::setFont(m_fontUI);
        //graphics::drawText(300.0f, 48.0f, 18.0f, m_status, t);
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
        if (m_timeBonus != 0)
            pointsStr += " + " + std::to_string(m_timeBonus) + "time bonus";

        graphics::drawText(900.0f, 28.0f, 20.0f, timeStr, t);
        graphics::drawText(900.0f, 48.0f, 20.0f, pointsStr, t);


        if (m_showHelp)
        {
            graphics::Brush ov;
            ov.fill_color[0] = 0.0f; ov.fill_color[1] = 0.0f; ov.fill_color[2] = 0.0f;
            ov.fill_opacity = 0.55f;
            ov.outline_opacity = 0.0f;
            graphics::drawRect(512.0f, 384.0f, 1024.0f, 768.0f, ov);

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
    br.outline_color[2] = 0.55f; // green-ish

    for (Node* n : m_nodes)
    {
        if (!n || !n->walkable) continue;
        if (!m_onShortest[idx(n)]) continue;

        const float cx = m_originX + n->col * m_cell + m_cell * 0.5f;
        const float cy = m_originY + n->row * m_cell + m_cell * 0.5f;

        graphics::drawRect(cx, cy, m_cell - 6.0f, m_cell - 6.0f, br);

    };
}
