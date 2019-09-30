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

    int getChecked() const { return m_checked; }
    int getFlagged() const { return m_flagged; }

    void dump();

private:
    struct TileInfo
    {
        bool flagged = false;
        bool checked = false;
        bool restricted = false;
        int restrictedGroup = -1;
        int adjecentMines = -1;
    };

    bool check(int x, int y);
    int checkAdjecent(int x, int y);
    std::vector<std::pair<int, int>> getAdjecent(int x, int y) const;
    std::vector<std::pair<int, int>> getAdjecent(int x, int y, std::function<bool(const TileInfo&)> pred) const;
    int countAdjecent(int x, int y, std::function<bool(const TileInfo&)> pred) const;
    void groupRestricted();
    int followRestricted(int x, int y);

    const Board &m_board;
    RndGen m_rnd;
    std::vector<std::vector<TileInfo>> m_tiles;
    int m_checked = 0;
    int m_flagged = 0;
    std::vector<std::pair<int, int>> m_blanks;
    std::vector<std::vector<std::pair<int, int>>> m_restrictedGroups;
};
