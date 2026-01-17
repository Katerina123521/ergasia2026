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

    if (hovered && ms.button_left_pressed)
    {
        pressed = true;
        return true;
    }

    // Release ends press
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

	std::string tex = texIdle;
    if (!enabled)        tex = texDisabled;
    else if (pressed)    tex = texDown;
    else if (hovered)    tex = texHover;

    
    graphics::Brush br;
    br.outline_opacity = 0.0f;
    br.fill_opacity = 1.0f;
    br.fill_color[0] = 1.0f; br.fill_color[1] = 1.0f; br.fill_color[2] = 1.0f;
    br.texture = tex;

    graphics::drawRect(cx, cy, w, h, br);

    // text
    graphics::Brush txt;

    txt.outline_opacity = 0.0f;
    txt.fill_opacity = enabled ? 1.0f : 0.6f;
    txt.fill_color[0] = 1.0f; txt.fill_color[1] = 1.0f; txt.fill_color[2] = 1.0f;

    const float textX = (cx - w * 0.5f) + padX;
    const float textY = cy + textSize * 0.20f;
    graphics::drawText(textX, textY, textSize, text, txt);
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
    txt.outline_opacity = 0.0f;
    graphics::drawText(x + s * 0.7f, y + 6.0f, 14.0f, label, txt);
}

void LegendPanel::draw() const
{
    if (!visible) return;

    // panel bg
    drawPanel(cx, cy, w, h, false, false);

    graphics::Brush title;
    title.fill_color[0] = 1.0f; title.fill_color[1] = 1.0f; title.fill_color[2] = 1.0f;
    title.outline_opacity = 0.0f;

    const float titleSize = 18.0f;
    const std::string titleStr = "GUIDE";

    const float approxTextW = (float)titleStr.size() * titleSize * 0.45f;
    const float titleX = cx - approxTextW * 0.5f;
    const float titleY = cy - h * 0.5f + 24.0f;

    graphics::drawText(titleX, titleY, titleSize, titleStr, title);

    // Separator line after title
    graphics::Brush line;
    line.fill_color[0] = 0.25f; line.fill_color[1] = 0.70f; line.fill_color[2] = 0.95f;
    line.fill_opacity = 0.35f;
    line.outline_opacity = 0.0f;
    graphics::drawRect(cx, cy - h * 0.5f + 40.0f, w - 32.0f, 2.0f, line);

    float left = cx - w * 0.5f + 18.0f;
    float startY = cy - h * 0.5f + 52.0f;

    // 3-column layout for legend items with better spacing
    const float legendPanelW = w - 36.0f;
    const float colWidth = legendPanelW / 3.0f;
    const float rowHeight = 30.0f;
    const float boxSize = 16.0f;

    struct LegendItem {
        float r, g, b;
        const char* label;
    };
    
    LegendItem items[] = {
        {0.10f, 0.10f, 0.10f, "Wall"},
        {0.20f, 0.80f, 0.20f, "Start"},
        {0.90f, 0.25f, 0.25f, "Goal"},
        {0.35f, 0.70f, 0.95f, "Open set"},
        {0.55f, 0.55f, 0.70f, "Closed set"},
        {1.00f, 0.85f, 0.15f, "Final path"},
        {0.85f, 0.25f, 0.85f, "Player path"}
    };

    for (int i = 0; i < 7; i++)
    {
        int row = i / 3;
        int col = i % 3;
        
        float x = left + col * colWidth + 8.0f;
        float y = startY + row * rowHeight;
        
        legendRow(x, y, boxSize, items[i].r, items[i].g, items[i].b, items[i].label);
    }

    float y = startY + 3 * rowHeight;
    y += 20.0f;

    // Separator line before controls
    graphics::drawRect(cx, y, w - 32.0f, 2.0f, line);
    y += 16.0f;

    // Controls section - Two column layout
    graphics::Brush txt;
    txt.fill_color[0] = 1.0f; txt.fill_color[1] = 1.0f; txt.fill_color[2] = 1.0f;
    txt.outline_opacity = 0.0f;

    const float controlTitleSize = 18.0f;
    const float controlLineSize = 13.0f;
    const float controlGap = 8.0f;

    const std::string controlsTitle = "CONTROLS";
    const float controlsTitleW = (float)controlsTitle.size() * controlTitleSize * 0.45f;
    graphics::drawText(cx - controlsTitleW * 0.5f, y + 6.0f, controlTitleSize, controlsTitle, txt);
    y += controlTitleSize + controlGap + 4.0f;

    // Two column layout
    const float columnGap = 12.0f;
    const float colW = (w - 36.0f - columnGap) / 2.0f;
    
    // Basic controls
    const float leftX = left + 4.0f;
    float leftY = y;
    
    const char* leftControls[] = {
        "LMB: toggle wall",
        "RMB: set Start",
        "Shift+RMB: Goal",
        "SPACE: run/pause",
        "R: reset search"
    };

    for (int i = 0; i < 5; i++)
    {
        graphics::drawText(leftX, leftY + 6.0f, controlLineSize, leftControls[i], txt);
        leftY += controlLineSize + controlGap;
    }

    // Right column - Draw Mode controls
    const float rightX = leftX + colW + columnGap;
    float rightY = y;

    // Draw Mode title
    graphics::Brush dimTxt = txt;
    dimTxt.fill_opacity = 0.9f;
    graphics::drawText(rightX, rightY + 6.0f, controlLineSize + 1.0f, "D: toggle Draw Mode", dimTxt);
    rightY += (controlLineSize + 1.0f) + controlGap + 4.0f;

    // Draw mode description
    dimTxt.fill_opacity = 0.75f;
    const char* drawModeLines[] = {
        "Draw Mode:",
        "- LMB drag to",
        "  draw path",
        "- Start to Goal"
    };

    for (int i = 0; i < 4; i++)
    {
        graphics::drawText(rightX, rightY + 6.0f, controlLineSize - 1.0f, drawModeLines[i], dimTxt);
        rightY += (controlLineSize - 1.0f) + (controlGap - 2.0f);
    }
}
