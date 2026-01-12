#include "GlobalState.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_set>

bool GlobalState::loadLevelFromFile(const std::string& relPath)
{
    std::ifstream f(relPath);
    if (!f.is_open())
    {
        m_status = "Failed to open level file: " + relPath;
        return false;
    }

    // Reset gameplay state
    cancelAStar();
    clearPlayerPath();
    resetScore();
    resetAttemptTimer();

    // --- Read header: "ROWS COLS" ---
    int fileRows = 0, fileCols = 0;
    {
        std::string header;
        if (!std::getline(f, header))
        {
            m_status = "Level file empty: " + relPath;
            return false;
        }
        if (!header.empty() && header.back() == '\r') header.pop_back();

        std::istringstream iss(header);
        if (!(iss >> fileRows >> fileCols) || fileRows <= 0 || fileCols <= 0)
        {
            m_status = "Invalid level header (expected: ROWS COLS): " + relPath;
            return false;
        }
    }

    // --- Rebuild grid if size differs ---
    if (fileRows != m_rows || fileCols != m_cols)
    {
        rebuildGrid(fileRows, fileCols);
    }

    // --- Read exactly m_rows map lines ---
    std::vector<std::string> lines;
    lines.reserve(m_rows);

    std::string line;
    while ((int)lines.size() < m_rows && std::getline(f, line))
    {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue; // ignore blank lines
        lines.push_back(line);
    }

    if ((int)lines.size() != m_rows)
    {
        m_status = "Wrong number of map rows in " + relPath +
            " (expected " + std::to_string(m_rows) +
            ", got " + std::to_string(lines.size()) + ")";
        return false;
    }

    for (int r = 0; r < m_rows; r++)
    {
        if ((int)lines[r].size() != m_cols)
        {
            m_status = "Row " + std::to_string(r) + " wrong length in " + relPath +
                " (expected " + std::to_string(m_cols) +
                ", got " + std::to_string(lines[r].size()) + ")";
            return false;
        }
    }

    Node* newStart = nullptr;
    Node* newGoal = nullptr;

    // --- Apply grid ---
    for (int r = 0; r < m_rows; r++)
    {
        for (int c = 0; c < m_cols; c++)
        {
            Node* n = nodeAt(r, c);
            if (!n) continue;

            char ch = lines[r][c];

            if (ch == '#')
            {
                n->walkable = false;
                n->state = NodeVizState::Wall;
            }
            else if (ch == '.' || ch == ' ')
            {
                n->walkable = true;
                n->state = NodeVizState::Empty;
            }
            else if (ch == 'S')
            {
                n->walkable = true;
                n->state = NodeVizState::Empty;
                newStart = n;
            }
            else if (ch == 'G')
            {
                n->walkable = true;
                n->state = NodeVizState::Empty;
                newGoal = n;
            }
            else
            {
                // unknown char -> treat as empty
                n->walkable = true;
                n->state = NodeVizState::Empty;
            }
        }
    }

    if (!newStart || !newGoal)
    {
        m_status = "Level missing S or G: " + relPath;
        return false;
    }

    // Set start/goal
    m_start = newStart;
    m_goal = newGoal;

    m_start->state = NodeVizState::Start;
    m_goal->state = NodeVizState::Goal;

    m_currentLevelPath = relPath;

    // --- Solvability check (must have a solution) ---
    computeShortestCorridor();
    if (m_shortestSteps < 0)
    {
        m_status = "UNSOLVABLE level (no path S->G): " + relPath;
        return false;
    }

    m_status = "Loaded: " + relPath;
    return true;
}

void GlobalState::loadNextLevel(int difficulty)
{
    m_currentDifficulty = difficulty;

    std::vector<std::string>* list = nullptr;
    int* idx = nullptr;

    if (difficulty == 0) { list = &m_levelsEasy;   idx = &m_levelIdxEasy; }
    if (difficulty == 1) { list = &m_levelsMedium; idx = &m_levelIdxMedium; }
    if (difficulty == 2) { list = &m_levelsHard;   idx = &m_levelIdxHard; }

    if (!list || list->empty())
    {
        m_status = "No level files set for this difficulty.";
        return;
    }

    // cycle through levels
    *idx = (*idx) % (int)list->size();

    // try to load; if fails, advance and try a few times
    for (int tries = 0; tries < (int)list->size(); tries++)
    {
        const std::string& path = (*list)[*idx];
        (*idx) = (*idx + 1) % (int)list->size();

        if (loadLevelFromFile(path))
            return;
    }

    m_status = "All level files failed to load for this difficulty.";
}
