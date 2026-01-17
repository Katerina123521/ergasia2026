#pragma once

class VisualAsset 
	// Abstract base class for visual assets (Nodes, UI widgets, Button etc.)
{
public:
    virtual ~VisualAsset() = default;
    virtual void draw() const = 0;
};

