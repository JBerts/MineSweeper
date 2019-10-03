#pragma once

#include <functional>
#include <utility>
#include <vector>

#include "board.hpp"
#include "rnd_gen.hpp"

class Sweeper
{
public:
    Sweeper(const Board &board);

    bool solve(bool showProgress = false);
    int solveBlanks();
    int solveSingles();
    int solveRestricted();

    int getCheckedCount() const { return m_checked; }
    int getFlaggedCount() const { return m_flagged; }
    int getGuessedCount() const { return m_guessed; }

    void dump();

private:
    struct TileInfo
    {
        bool flagged = false;
        bool checked = false;
        bool restricted = false;
        bool testMine = false;
        int restrictedGroup = -1;
        int adjecentMines = -1;
        int restrictedSolutionMines = 0;
    };

    bool check(int x, int y);
    int checkAdjecent(int x, int y);
    std::vector<std::pair<int, int>> getAdjecent(int x, int y) const;
    std::vector<std::pair<int, int>> getAdjecent(int x, int y, std::function<bool(const TileInfo&)> pred) const;
    int countAdjecent(int x, int y, std::function<bool(const TileInfo&)> pred) const;
    void groupRestricted();
    int followRestricted(int x, int y, int group);
    int testRestricted(std::vector<bool> &path, const std::vector<std::pair<int, int>> &restricted);
    bool verifyRestricted(const std::vector<std::pair<int, int>> &restricted);

    const Board &m_board;
    RndGen m_rnd;
    std::vector<std::vector<TileInfo>> m_tiles;
    bool m_showProgress;
    int m_checked = 0;
    int m_flagged = 0;
    int m_guessed = 0;
    std::vector<std::pair<int, int>> m_blanks;
    std::vector<std::vector<std::pair<int, int>>> m_restrictedGroups;
};
