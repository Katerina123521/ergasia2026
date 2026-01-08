# A* Pathfinder (SGG)

A small interactive A* pathfinding “game” built with the **Simple Game Graphics (SGG)** library.

You can place walls, set Start/Goal, and watch A* explore the grid step-by-step.  
Next, the project will evolve into a game where the player draws a path and earns points based on how close it is to the optimal A* route.

## Features (current)
- Grid-based map with **walls / start / goal**
- **A\*** pathfinding visualization
  - step-by-step (open/closed per frame)
  - run/pause + single-step mode
- UI buttons (run/pause, step, reset, clear, random walls, speed +/-)
- On-screen legend (colors + controls)
- Optional background music toggle

## Controls
- **LMB**: toggle wall  
- **RMB**: set Start  
- **Shift + RMB**: set Goal  
- **SPACE**: run/pause A* (step-by-step)  
- **R**: reset search (keeps walls + start/goal)

## Assets
Place assets in an `assets/` folder (relative to the working directory).

Recommended:
- `assets/orbitron.ttf` (font)
- `assets/bgm.ogg` (optional background music)
- `assets/hit1.wav` (optional button click sound)

## Build / Run (Visual Studio 2022)
1. Open the `.sln` in Visual Studio.
2. Build as **x64**.
3. Make sure the **Working Directory** is set so assets are found:
   - Project Properties → Debugging → Working Directory = `$(ProjectDir)`
4. Run.

## Collaboration
- Please do **not** commit build output (`x64/Debug`, `.exe`, `.dll`, `.lib`, `.pdb`, etc.).
- Use branches and PRs for bigger features (levels, scoring mode, UI redesign).

---

## TODO (planned work)

### Gameplay (new “player vs algorithm” mode)
- [x] Add **Draw Mode**: player draws a path from Start to Goal by dragging/clicking cells.
- [x] Validate path: must be continuous and cannot go through walls.
- [ ] Compute **A* optimal path** and compare to player path:
  - [ ] Score based on overlap with A* path (Jaccard/overlap ratio)
  - [ ] Penalty for extra steps (path length difference)
  - [ ] Bonus for matching key turns/waypoints
- [ ] Show feedback after submission:
  - [ ] overlay player path vs A* path
  - [ ] score breakdown (overlap %, extra steps, mistakes)
- [ ] Add timer (optional) and combo/bonus system for replayability.

### Levels system
- [ ] Load levels from text files, e.g. `assets/levels/level01.txt`
  - `.` empty, `#` wall, `S` start, `G` goal
- [ ] Level select screen (or Next/Prev level buttons)
- [ ] Track best score per level (local file save optional)

### UI improvements
- [ ] Reduce sidebar sizes / improve responsive layout so grid stays large
- [ ] Add clear UI sections:
  - Controls panel (left)
  - Legend + stats panel (right)
  - Top HUD: title, status, score, current level
- [ ] Add a small **stats panel**:
  - expanded nodes, final path length, runtime steps
- [ ] Add “tooltip” text for buttons or a Help overlay rework.

### Algorithm extensions / comparison
- [ ] Add second algorithm for comparison (toggle):
  - Dijkstra or BFS (unweighted)
- [ ] Add “Compare” view:
  - show both paths
  - show explored nodes count for each

### Polish / quality
- [ ] Add better random wall generator (maze-like option)
- [ ] Add sound feedback for:
  - button clicks
  - success/failure
  - drawing path
- [ ] Add subtle background texture (optional)
- [ ] Improve input handling:
  - prevent accidental edits while algorithm is running (or auto-cancel safely)
  - drag-to-paint walls (optional)
- [ ] Refactor code structure:
  - separate `UI`, `Grid`, `Pathfinding`, `Levels`, `Scoring`
- [ ] Add basic tests/debug mode:
  - no-path cases
  - start==goal
  - edge clicking and reset behavior

### Submission readiness
- [ ] Ensure dynamic memory + inheritance + STL usage remain clear in code
- [ ] Clean repo for submission rules (no binaries, only required headers if needed)
- [ ] Prepare a short demo script for the oral exam (what to show + what to explain)
