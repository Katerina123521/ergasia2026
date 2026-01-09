#include "graphics.h"
#include "scancodes.h"
#include "GlobalState.h"

static void draw()
{
    auto* gs = (GlobalState*)graphics::getUserData();
    if (gs) gs->draw();
}

static void update(float ms)
{
    auto* gs = (GlobalState*)graphics::getUserData();
    if (gs) gs->update(ms);
}

int main()
{
    const int W = 1024;
    const int H = 768;

    graphics::createWindow(W, H, "SGG A* Grid Demo");

    graphics::setCanvasSize((float)W, (float)H);
    graphics::setCanvasScaleMode(graphics::CANVAS_SCALE_FIT);

    graphics::setFont("assets/orange_juice.ttf");


    // Single GlobalState instance (stored as user data) :contentReference[oaicite:6]{index=6}
    auto* gs = new GlobalState();
    gs->init();
    graphics::setUserData(gs);

    graphics::setDrawFunction(draw);
    graphics::setUpdateFunction(update);

    graphics::startMessageLoop();

    // Cleanup
    graphics::setUserData(nullptr);
    delete gs;

    graphics::destroyWindow();
    return 0;
}
