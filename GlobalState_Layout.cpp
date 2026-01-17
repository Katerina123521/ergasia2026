#include "GlobalState.h"
#include "UIConstants.h"
#include <algorithm>

void GlobalState::setupLayout()
{
    const float availW = WIN_W - UI_LEFT_W - 2.0f * UI_MARGIN;
    const float availH = WIN_H - UI_TOP_H - 2.0f * UI_MARGIN;

    m_cell = std::min(availW / m_cols, availH / m_rows);

    const float gridW = m_cols * m_cell;
    const float gridH = m_rows * m_cell;

    m_originX = UI_LEFT_W + UI_MARGIN + (availW - gridW) * 0.5f;
    m_originY = UI_TOP_H + UI_MARGIN + (availH - gridH) * 0.5f;

    Node::setLayout(m_originX, m_originY, m_cell);
}
