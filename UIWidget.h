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

    // optional sound when clicked
    std::string clickSound = ""; // e.g. "assets/hit1.wav"
    float clickVolume = 0.25f;

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
