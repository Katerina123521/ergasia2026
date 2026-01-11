#pragma once
#include "VisualAsset.h"
#include "graphics.h"
#include <string>
#include <functional>

class UIWidget : public VisualAsset
{
public:
    float cx = 0, cy = 0, w = 0, h = 0;
    bool visible = true;
    bool enabled = true;

    // interaction state
    bool hovered = false;
    bool pressed = false;

    UIWidget(float x, float y, float width, float height)
        : cx(x), cy(y), w(width), h(height) {
    }

    bool hitTest(float x, float y) const
    {
        return (cx - w * 0.5f <= x && x <= cx + w * 0.5f) &&
            (cy - h * 0.5f <= y && y <= cy + h * 0.5f);
    }

    // returns true if this widget "captures" the mouse (prevents grid click-through)
    virtual bool handleInput(const graphics::MouseState& ms, float mx, float my) { (void)ms; (void)mx; (void)my; return false; }
};

class Button : public UIWidget
{
public:
    std::string text;
    std::function<void()> onClick;

    std::string clickSound = "";
    float clickVolume = 0.25f;

    std::string texIdle = "assets/ui/btn_blue_idle.png";
    std::string texHover = "assets/ui/btn_blue_hover.png";
    std::string texDown = "assets/ui/btn_blue_down.png";
    std::string texDisabled = "assets/ui/btn_gray_idle.png";

    std::string iconTex = "";   // e.g. "assets/ui/icon_play.png"
    float iconScale = 0.55f;    // icon size relative to button height

    float textSize = 18.0f;
    float padX = 18.0f;

    Button(float x, float y, float width, float height, std::string t, std::function<void()> cb)
        : UIWidget(x, y, width, height), text(std::move(t)), onClick(std::move(cb)) {
    }

    bool handleInput(const graphics::MouseState& ms, float mx, float my) override;
    void draw() const override;
};

class ToggleButton : public Button
{
public:
    bool* value = nullptr; // points to a bool in GlobalState
    std::string labelOn = "ON";
    std::string labelOff = "OFF";

    ToggleButton(float x, float y, float width, float height, std::string t, bool* v)
        : Button(x, y, width, height, std::move(t), nullptr), value(v)
    {
        onClick = [this]() { if (value) *value = !*value; };
    }

    void draw() const override;
};

class LegendPanel : public UIWidget
{
public:
    LegendPanel(float x, float y, float width, float height)
        : UIWidget(x, y, width, height) {
    }

    void draw() const override;
};
