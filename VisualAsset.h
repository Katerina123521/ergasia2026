#pragma once

class VisualAsset
{
public:
    virtual ~VisualAsset() = default;
    virtual void update(float dt) { (void)dt; }
    virtual void draw() const = 0;
};

