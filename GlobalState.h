#pragma once
#include <vector>
#include <string>
#include "VisualAsset.h"
#include "Node.h"
#include "UIWidget.h"
#include "Grid.h"
#include "Pathfinder.h"
#include <unordered_set>
#include <queue>
#include <limits>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>


class GlobalState final
{
public:
    GlobalState();
    ~GlobalState();

    void init();
    void update(float dt);
    void draw() const;

private:
    // Grid settings
    int m_rows = 15;
    int m_cols = 20;

    float m_cell = 40.0f;
    float m_originX = 0.0f;
    float m_originY = 0.0f;

    // Dynamically allocated nodes
    Grid m_grid;

    // Polymorphic collection
    std::vector<VisualAsset*> m_drawables;

    Node* m_start = nullptr;
    Node* m_goal = nullptr;

    std::string m_status;

private:
	// --- Node access helpers ---
    Node* nodeAt(int r, int c) const;
    Node* nodeFromMouse(float mx, float my) const;

	// --- Grid management ---
    void buildGridGraph();
    void clearWallsAndPathKeepEndpoints();
    void resetSearchVisuals();
    void setupUI();
    void setupLayout();

	// --- A* Pathfinder ---
    Pathfinder m_pf;
    bool runAStar(); // returns true if found path
    void startAStar();
    bool stepAStar();
    void cancelAStar();
    enum class AStarRunState { Idle, Running, Paused, Found, NoPath };
    AStarRunState m_aState = AStarRunState::Idle;

	// --- A* step timing ---
    float m_stepDelayMs = 35.0f;
    float m_stepAccumMs = 0.0f;

	// --- Key edge detection ---
    bool m_prevSpace = false;
    bool m_prevR = false;
    bool m_prevD = false;
    
	// --- UI ---
    std::vector<UIWidget*> m_ui;

	// --- Help window ---
    bool m_showHelp = false;

	// --- Legend panel ---
    LegendPanel* m_legend = nullptr;

	// --- Music ---
    bool m_musicOn = true;
    float m_musicVolume = 0.20f;
    std::string m_musicFile = "assets/bgm.ogg";

	// --- Player Draw Mode ---
    bool m_drawMode = false;          // Draw Mode toggle
    bool m_isDrawing = false;         // currently dragging
    Node* m_lastDrawn = nullptr;      // last node added while dragging
    std::vector<Node*> m_playerPath;  // player path nodes (includes start, ends at goal if reached)

    void clearPlayerPath();
    bool isAdjacent(const Node* a, const Node* b) const;
    bool appendPlayerPath(Node* n);
    void beginPlayerDrawing();
    void endPlayerDrawing();

	// --- Player path validation ---
    enum class PlayerPathValidation
    {
        Empty,
        ValidToGoal,
        ValidButNotAtGoal,
        InvalidStart,
        InvalidWall,
        InvalidNonAdjacent,
        InvalidDuplicate
    };

    PlayerPathValidation m_pathValidation = PlayerPathValidation::Empty;
    int m_invalidIndex = -1;      // index in m_playerPath where it fails
    bool m_pathComplete = false;  // ends at goal

    PlayerPathValidation validatePlayerPath();
    void markInvalidNode(int index);
    void clearInvalidMarks();

    // --- Scoring / shortest distance data ---
    std::vector<int>  m_distStart;
    std::vector<int>  m_distGoal;
    std::vector<char> m_onShortest;     // 1 if cell is on any shortest path

    int   m_shortestSteps = -1;         // shortest steps Start->Goal (BFS)
    int   m_playerSteps = -1;           // player's steps (playerPath.size()-1)
    int   m_onShortestCount = 0;        // how many player nodes on shortest corridor
    float m_overlap = 0.0f;             // [0..1]
    float m_efficiency = 0.0f;          // [0..1]
    int   m_score = 0;                  // [0..100]
    bool  m_showShortestHint = true;    // draw outline on shortest-corridor cells

    int idx(const Node* n) const { return n->row * m_cols + n->col; }

    void computeBfsDistances(Node* src, std::vector<int>& dist);
    bool computeShortestCorridor();   // fills m_shortestSteps + m_onShortest
    bool computeScore();              // updates m_score + stats, returns true if scored
    void resetScore();                
    void drawShortestHintOverlay() const;

    // --- Timer bonus only ---
    bool  m_timerEnabled = true;
    bool  m_timerRunning = false;
    float m_attemptTimeMs = 0.0f;   // current attempt time
    float m_lastAttemptMs = 0.0f;   // time of last finished attempt

    int   m_timeBonus = 0;          // 0..20
    int   m_finalPoints = 0;        // base score + time bonus
    bool  m_allowScoreOver100 = false;

    // --- Tunables for par-time ---
    float m_parBaseMs = 800.0f;
    float m_parPerStepMs = 220.0f;
    int   m_maxTimeBonus = 20;
    void startAttemptTimer();
    void stopAttemptTimer();
    void resetAttemptTimer();
    void updateAttemptTimer(float dt);
    std::string formatTime(float ms) const;
    void applyTimerBonus();

	// --- Level loading ---
    std::vector<std::string> m_levelsEasy;
    std::vector<std::string> m_levelsMedium;
    std::vector<std::string> m_levelsHard;

    int m_levelIdxEasy = 0;
    int m_levelIdxMedium = 0;
    int m_levelIdxHard = 0;

    int m_currentDifficulty = 0;     // 0 easy, 1 medium, 2 hard
    std::string m_currentLevelPath;

    bool loadLevelFromFile(const std::string& relPath);
    void loadNextLevel(int difficulty);  // easy/medium/hard
    void rebuildGrid(int newRows, int newCols);

	// --- Fonts + Title ---
    std::string m_fontTitle = "assets/PressStart2P.ttf";
    std::string m_fontUI = "assets/VT323.ttf";
    std::string m_title = "StarPath Quest";

	// --- A* speed control ---
    int m_speedIndex = 3; // start in the middle
    static constexpr float SPEED_LEVELS[7] = { 200, 120, 80, 50, 30, 18, 10 };
};
