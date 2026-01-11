#include "UIWidget.h"

static void drawPanel(float cx, float cy, float w, float h, bool hovered, bool pressed)
{
    // shadow
    graphics::Brush sh;
    sh.fill_color[0] = 0.0f; sh.fill_color[1] = 0.0f; sh.fill_color[2] = 0.0f;
    sh.fill_opacity = 0.25f;
    sh.outline_opacity = 0.0f;
    graphics::drawRect(cx + 3.0f, cy + 3.0f, w, h, sh);

    // main panel
    graphics::Brush br;
    br.outline_opacity = 1.0f;
    br.outline_width = 1.0f;
    br.outline_color[0] = 0.10f; br.outline_color[1] = 0.10f; br.outline_color[2] = 0.12f;

    br.gradient = true;
    br.gradient_dir_u = 0.0f;
    br.gradient_dir_v = 1.0f;

    // base colors
    float a = 0.95f;
    br.fill_opacity = a;
    br.fill_secondary_opacity = a;

    br.fill_color[0] = 0.16f; br.fill_color[1] = 0.18f; br.fill_color[2] = 0.22f;
    br.fill_secondary_color[0] = 0.11f; br.fill_secondary_color[1] = 0.12f; br.fill_secondary_color[2] = 0.16f;

    if (hovered)
    {
        br.fill_color[0] += 0.04f; br.fill_color[1] += 0.04f; br.fill_color[2] += 0.04f;
        br.fill_secondary_color[0] += 0.03f; br.fill_secondary_color[1] += 0.03f; br.fill_secondary_color[2] += 0.03f;
    }
    if (pressed)
    {
        br.fill_color[0] -= 0.03f; br.fill_color[1] -= 0.03f; br.fill_color[2] -= 0.03f;
        br.fill_secondary_color[0] -= 0.03f; br.fill_secondary_color[1] -= 0.03f; br.fill_secondary_color[2] -= 0.03f;
    }

    graphics::drawRect(cx, cy, w, h, br);
}

bool Button::handleInput(const graphics::MouseState& ms, float mx, float my)
{
    if (!visible) return false;

    hovered = hitTest(mx, my);

    // If disabled still capture mouse when hovering
    if (!enabled)
    {
        pressed = false;
        return hovered;
    }

    // Press begins only if click starts inside
    if (hovered && ms.button_left_pressed)
    {
        pressed = true;
        return true;
    }

    // Release ends press; click triggers only if released on button
    if (ms.button_left_released)
    {
        bool wasPressed = pressed;
        pressed = false;

        if (wasPressed && hovered)
        {
            if (!clickSound.empty())
                graphics::playSound(clickSound, clickVolume);

            if (onClick) onClick();
            return true;
        }
    }

    return hovered || pressed;
}


void Button::draw() const
{
    if (!visible) return;

	std::string tex = texIdle; //texture based on state
    if (!enabled)        tex = texDisabled;
    else if (pressed)    tex = texDown;
    else if (hovered)    tex = texHover;

    
    graphics::Brush br;
    br.outline_opacity = 0.0f;
    br.fill_opacity = 1.0f;
    br.fill_color[0] = 1.0f; br.fill_color[1] = 1.0f; br.fill_color[2] = 1.0f; //white
    br.texture = tex;

    graphics::drawRect(cx, cy, w, h, br);

    //If we have icon
    float iconW = 0.0f;
    if (!iconTex.empty())
    {
        graphics::Brush ib = br;
        ib.texture = iconTex;

        float iconSize = h * iconScale;
        float iconX = (cx - w * 0.5f) + padX + iconSize * 0.5f;
        graphics::drawRect(iconX, cy, iconSize, iconSize, ib);

        iconW = iconSize + 10.0f; // spacing 
    }

    // text
    graphics::Brush txt;

    txt.outline_opacity = 0.0f;
    txt.fill_opacity = enabled ? 1.0f : 0.6f;
    txt.fill_color[0] = 1.0f; txt.fill_color[1] = 1.0f; txt.fill_color[2] = 1.0f;

    const float textX = (cx - w * 0.5f) + padX + iconW;
    const float textY = cy + textSize * 0.35f;
    graphics::drawText(textX, textY, textSize, text, txt);
}


void ToggleButton::draw() const
{
    if (!visible) return;

    drawPanel(cx, cy, w, h, hovered, pressed);

    graphics::Brush txt;
    txt.fill_color[0] = 1.0f; txt.fill_color[1] = 1.0f; txt.fill_color[2] = 1.0f;

    const float pad = 14.0f;
    const float size = 18.0f;

    std::string state = (value && *value) ? labelOn : labelOff;
    graphics::drawText(cx - w * 0.5f + pad, cy + 6.0f, size, text + " : " + state, txt);
}

static void legendRow(float x, float y, float s, float r, float g, float b, const std::string& label)
{
    graphics::Brush box;
    box.fill_color[0] = r; box.fill_color[1] = g; box.fill_color[2] = b;
    box.fill_opacity = 1.0f;
    box.outline_opacity = 0.6f;
    box.outline_width = 1.0f;
    box.outline_color[0] = 0.1f; box.outline_color[1] = 0.1f; box.outline_color[2] = 0.1f;
    graphics::drawRect(x, y, s, s, box);

    graphics::Brush txt;
    txt.fill_color[0] = 0.95f; txt.fill_color[1] = 0.95f; txt.fill_color[2] = 0.95f;
    graphics::drawText(x + s * 0.7f, y + 6.0f, 16.0f, label, txt);
}

void LegendPanel::draw() const
{
    if (!visible) return;

    // panel bg
    drawPanel(cx, cy, w, h, false, false);

    graphics::Brush title;
    title.fill_color[0] = 1.0f; title.fill_color[1] = 1.0f; title.fill_color[2] = 1.0f;

    const float titleSize = 20.0f;
    const std::string titleStr = "LEGEND";

    // Προσέγγιση πλάτους κειμένου για centering
    const float approxTextW = (float)titleStr.size() * titleSize * 0.45f;

    const float titleX = cx - approxTextW * 0.5f;
    const float titleY = cy - h * 0.5f + 26.0f;

    graphics::drawText(titleX, titleY, titleSize, titleStr, title);


    float left = cx - w * 0.5f + 18.0f;
    float y = cy - h * 0.5f + 56.0f;

    const float legendStep = 26.0f;   // ύψος σειράς
    const float legendGap = 6.0f;    // μικρό κενό ανάμεσα στις σειρές
    const float legendLine = legendStep + legendGap;

    legendRow(left + 10.0f, y, 16.0f, 0.10f, 0.10f, 0.10f, "Wall");        y += legendLine;
    legendRow(left + 10.0f, y, 16.0f, 0.20f, 0.80f, 0.20f, "Start");       y += legendLine;
    legendRow(left + 10.0f, y, 16.0f, 0.90f, 0.25f, 0.25f, "Goal");        y += legendLine;
    legendRow(left + 10.0f, y, 16.0f, 0.35f, 0.70f, 0.95f, "Open set");    y += legendLine;
    legendRow(left + 10.0f, y, 16.0f, 0.55f, 0.55f, 0.70f, "Closed set");  y += legendLine;
    legendRow(left + 10.0f, y, 16.0f, 1.00f, 0.85f, 0.15f, "Final path");  y += legendLine;
    legendRow(left + 10.0f, y, 16.0f, 0.85f, 0.25f, 0.85f, "Player path"); y += legendLine;



    // ---- Controls block: BOTTOM of the panel (bigger font + more spacing) ----
    {
        graphics::Brush small;
        small.fill_color[0] = 1.0f; small.fill_color[1] = 1.0f; small.fill_color[2] = 1.0f;
        small.fill_opacity = 1.0f;
        small.outline_opacity = 0.0f;

        // Ρυθμίσεις εμφάνισης
        const float fontTitle = 22.0f;   // Controls:
        const float fontLine = 16.0f;   // οι γραμμές
        const float gap = 12.0f;         // κενό μεταξύ σειρών
        const float padBottom = 28.0f;   // απόσταση από κάτω

        const char* lines[] = {
            "Controls:",
            "L MB:   toggle wall",
            "R MB:   set Start",
            "Shift + R MB:   set Goal",
            "SPACE:   run/pause",
            "R:   reset search",       
            "D:   toggle Draw Mode",
            "Draw Mode:   L MB drag to draw path"
        };
        const int N = (int)(sizeof(lines) / sizeof(lines[0]));

        // Υπολογισμός συνολικού ύψους του block
        float totalH = 0.0f;
        for (int i = 0; i < N; i++)
        {
            float fs = (i == 0) ? fontTitle : fontLine;
            totalH += fs;
            if (i < N - 1) totalH += gap;
        }

        // bottom-aligned start y
        const float bottom = cy + h * 0.5f;
        float yy = (bottom - padBottom) - totalH;

        const float x = left;

        for (int i = 0; i < N; i++)
        {
            float fs = (i == 0) ? fontTitle : fontLine;

            // (προαιρετικό) μικρή σκιά για να ξεχωρίζει χωρίς να παχαίνει
            graphics::Brush sh = small;
            sh.fill_color[0] = 0.0f; sh.fill_color[1] = 0.0f; sh.fill_color[2] = 0.0f;
            sh.fill_opacity = 0.35f;
            graphics::drawText(x + 1.0f, yy + 1.0f, fs, lines[i], sh);

            // καθαρό κείμενο (ΜΟΝΟ 1 φορά)
            graphics::drawText(x, yy, fs, lines[i], small);


            yy += fs + gap;
        }
    }


}
