#pragma once
#include <vector>
#include <string>
#include "VisualAsset.h"
#include "Node.h"
#include "UIWidget.h"
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

    // Ownership: dynamically allocated nodes (assignment requires dynamic memory) :contentReference[oaicite:4]{index=4}
    std::vector<Node*> m_nodes;

    // Polymorphic collection (inheritance + polymorphism) :contentReference[oaicite:5]{index=5}
    std::vector<VisualAsset*> m_drawables;

    Node* m_start = nullptr;
    Node* m_goal = nullptr;

    std::string m_status;

private:
    Node* nodeAt(int r, int c) const;
    Node* nodeFromMouse(float mx, float my) const;

    void buildGridGraph();
    void clearWallsAndPathKeepEndpoints();
    void resetSearchVisuals();

    bool runAStar(); // returns true if found path
    float heuristic(const Node* a, const Node* b) const;

    enum class AStarRunState { Idle, Running, Paused, Found, NoPath };

    AStarRunState m_aState = AStarRunState::Idle;

    // Open list
    std::vector<Node*> m_open;

    // timing for animation
    float m_stepDelayMs = 35.0f;
    float m_stepAccumMs = 0.0f;

    // key edge detection
    bool m_prevSpace = false;
    bool m_prevR = false;

    void startAStar();
    bool stepAStar();
    void cancelAStar();

    std::vector<UIWidget*> m_ui;

    bool m_showHelp = false;   // big overlay
    LegendPanel* m_legend = nullptr;

    std::string m_title = "StarPath Quest";

    bool m_musicOn = true;
    float m_musicVolume = 0.20f;
    std::string m_musicFile = "assets/bgm.ogg";

    bool m_drawMode = false;          // Draw Mode toggle
    bool m_isDrawing = false;         // currently dragging
    Node* m_lastDrawn = nullptr;      // last node added while dragging
    std::vector<Node*> m_playerPath;  // player path nodes (includes start, ends at goal if reached)

    // key edge detect for D (optional)
    bool m_prevD = false;

    void clearPlayerPath();
    bool isAdjacent(const Node* a, const Node* b) const;
    bool appendPlayerPath(Node* n);
    void beginPlayerDrawing();
    void endPlayerDrawing();

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
    int m_invalidIndex = -1;      // index in m_playerPath where it fails (if any)
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

    // UI toggles
    bool  m_showShortestHint = true;    // draw outline on shortest-corridor cells
    bool  m_autoScoreOnGoal = true;     // if true, compute score when player reaches goal

    int idx(const Node* n) const { return n->row * m_cols + n->col; }

    void computeBfsDistances(Node* src, std::vector<int>& dist);
    bool computeShortestCorridor();   // fills m_shortestSteps + m_onShortest
    bool computeScore();              // updates m_score + stats, returns true if scored
    void resetScore();                // clears score HUD values

    void drawShortestHintOverlay() const;


    // --- Timer bonus only ---
    bool  m_timerEnabled = true;
    bool  m_timerRunning = false;
    float m_attemptTimeMs = 0.0f;   // current attempt time
    float m_lastAttemptMs = 0.0f;   // time of last finished attempt

    int   m_timeBonus = 0;          // 0..20
    int   m_finalPoints = 0;        // base score + time bonus (clamped if you want)
    bool  m_allowScoreOver100 = false; // optional

    // Tunables for par-time
    float m_parBaseMs = 800.0f;
    float m_parPerStepMs = 220.0f;
    int   m_maxTimeBonus = 20;
    void startAttemptTimer();
    void stopAttemptTimer();
    void resetAttemptTimer();
    void updateAttemptTimer(float dt);
    std::string formatTime(float ms) const;

    void applyTimerBonus(); // uses m_score, m_shortestSteps, m_lastAttemptMs

    std::vector<std::string> m_levelsEasy;
    std::vector<std::string> m_levelsMedium;
    std::vector<std::string> m_levelsHard;

    int m_levelIdxEasy = 0;
    int m_levelIdxMedium = 0;
    int m_levelIdxHard = 0;

    int m_currentDifficulty = 0;     // 0 easy, 1 medium, 2 hard
    std::string m_currentLevelPath;  // for HUD + best-attempt saving later

    bool loadLevelFromFile(const std::string& relPath);
    void loadNextLevel(int difficulty);  // easy/medium/hard
    void rebuildGrid(int newRows, int newCols);

    std::string m_fontTitle = "assets/PressStart2P.ttf";
    std::string m_fontUI = "assets/VT323.ttf";
    int m_speedIndex = 3; // start in the middle
    static constexpr float SPEED_LEVELS[7] = { 200, 120, 80, 50, 30, 18, 10 };

};
