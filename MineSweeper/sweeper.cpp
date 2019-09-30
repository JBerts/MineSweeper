#include "pch.h"
#include "sweeper.hpp"

#include <iostream>

using namespace std;

Sweeper::Sweeper(const Board &board)
    : m_board(board),
      m_rnd(board.getWidth() - 1, board.getHeight() - 1),
      m_tiles(m_board.getWidth(), vector<TileInfo>(m_board.getHeight()))
{
}

bool
Sweeper::solve(bool showProgress)
{
    while (m_checked + m_flagged < m_board.size())
    {
        // Make a guess
        int x, y;
        do
        {
            x = m_rnd.x();
            y = m_rnd.y();
        } while (m_tiles[x][y].checked || m_tiles[x][y].flagged);

        if (!check(x, y))
            return false;

        bool stuck = false;
        while (!stuck)
        {
            int blanksSolved = solveBlanks();
            int singlesSolved = solveSingles();
            if (showProgress)
                dump();

            if (blanksSolved == 0 && singlesSolved == 0)
            {
                if (solveRestricted() == 0)
                    stuck = true;
                else if (showProgress)
                    dump();
            }
        }
    }

    return true;
}

bool
Sweeper::check(int x, int y)
{
    ++m_checked;
    auto n = m_board.check(x, y);
    if (n < 0)
        return false;

    m_tiles[x][y].checked = true;
    m_tiles[x][y].adjecentMines = n;
    if (n == 0)
        m_blanks.push_back(make_pair(x, y));
    else
    {
        auto unchecked = getAdjecent(x, y, [](const TileInfo& ti) { return !ti.checked; });
        for (auto [x, y] : unchecked)
            m_tiles[x][y].restricted = true;
    }

    return true;
}

int
Sweeper::solveBlanks()
{
    int checkedBefore = m_checked;
    while (!m_blanks.empty())
    {
        auto [x, y] = m_blanks.back();
        m_blanks.pop_back();
        checkAdjecent(x, y);
    }

    return m_checked - checkedBefore;
}

vector<pair<int, int>>
Sweeper::getAdjecent(int x, int y) const
{
    return getAdjecent(x, y, [](const TileInfo&) { return true; });
}

vector<pair<int, int>>
Sweeper::getAdjecent(int x, int y, function<bool(const TileInfo&)> pred) const
{
    vector<pair<int, int>> adjecent;
    if (x > 0)
    {
        if (pred(m_tiles[x - 1][y]))
            adjecent.push_back(make_pair(x - 1, y));
        if (y > 0 && pred(m_tiles[x - 1][y - 1]))
            adjecent.push_back(make_pair(x - 1, y - 1));
        if (y < m_board.getHeight() - 1 && pred(m_tiles[x - 1][y + 1]))
            adjecent.push_back(make_pair(x - 1, y + 1));
    }
    if (x < m_board.getWidth() - 1)
    {
        if (pred(m_tiles[x + 1][y]))
            adjecent.push_back(make_pair(x + 1, y));
        if (y > 0 && pred(m_tiles[x + 1][y - 1]))
            adjecent.push_back(make_pair(x + 1, y - 1));
        if (y < m_board.getHeight() - 1 && pred(m_tiles[x + 1][y + 1]))
            adjecent.push_back(make_pair(x + 1, y + 1));
    }
    if (y > 0 && pred(m_tiles[x][y - 1]))
        adjecent.push_back(make_pair(x, y - 1));
    if (y < m_board.getHeight() - 1 && pred(m_tiles[x][y + 1]))
        adjecent.push_back(make_pair(x, y + 1));

    return adjecent;
}

int
Sweeper::checkAdjecent(int x, int y)
{
    int checkedBefore = m_checked;
    for (auto[x2, y2] : getAdjecent(x, y))
    {
        if (!m_tiles[x2][y2].checked && !m_tiles[x2][y2].flagged)
            check(x2, y2);
    }

    return m_checked - checkedBefore;
}

int
Sweeper::countAdjecent(int x, int y, function<bool(const TileInfo&)> pred) const
{
    auto adjecent = getAdjecent(x, y, pred);
    return adjecent.size();
}

int
Sweeper::solveSingles()
{
    int checkedBefore = m_checked;
    int flaggedBefore = m_flagged;
    for (int x = 0; x < m_board.getWidth(); x++)
    {
        for (int y = 0; y < m_board.getHeight(); y++)
        {
            auto &ti = m_tiles[x][y];
            if (!ti.checked || ti.adjecentMines == 0)
                continue;

            int flagged = countAdjecent(x, y, [](const TileInfo& ti) { return ti.flagged; });
            int unchecked = countAdjecent(x, y, [](const TileInfo& ti) { return !ti.checked; });

            if (flagged == ti.adjecentMines && unchecked > 0)
                checkAdjecent(x, y);
            else if (unchecked > 0 && flagged + unchecked == ti.adjecentMines)
            {
                for (auto [x2, y2] : getAdjecent(x, y))
                {
                    auto &ti2 = m_tiles[x2][y2];
                    if (!ti2.checked)
                    {
                        ti2.flagged = true;
                        ++m_flagged;
                    }
                }
            }
        }
    }
    return (m_checked - checkedBefore) + (m_flagged - flaggedBefore);
}

int
Sweeper::solveRestricted()
{
    groupRestricted();
    for (auto &restricted : m_restrictedGroups)
    {
        for (auto [x, y] : restricted)
        {

        }
    }
    return 0;
}
    
void
Sweeper::groupRestricted()
{
    m_restrictedGroups.clear();
    int currentGroup = -1;
    for (int x = 0; x < m_board.getWidth(); x++)
    {
        for (int y = 0; y < m_board.getHeight(); y++)
        {
            auto &ti = m_tiles[x][y];
            if (!ti.checked && ti.restricted && ti.restrictedGroup < 0)
            {
                ti.restrictedGroup = m_restrictedGroups.size();
                m_restrictedGroups.emplace_back();
                m_restrictedGroups.back().push_back(make_pair(x, y));
                followRestricted(x, y);
            }
        }
    }
}

int
Sweeper::followRestricted(int x, int y)
{
    auto &ti = m_tiles[x][y];
    if (ti.checked || !ti.restricted)
        return 0;

    auto adjecent = getAdjecent(
        x, y, 
        [ti](const TileInfo& ti2) 
        { 
            return ti2.restricted && ti.restrictedGroup != ti2.restrictedGroup; 
        });
    for (auto[x2, y2] : adjecent)
    {
        m_tiles[x2][y2].restrictedGroup = ti.restrictedGroup;
        m_restrictedGroups.at(ti.restrictedGroup).push_back(make_pair(x2, y2));
        followRestricted(x2, y2);
    }
    return adjecent.size();
}

void
Sweeper::dump()
{
    for (int y = 0; y < m_board.getHeight(); ++y)
    {
        for (int x = 0; x < m_board.getWidth(); ++x)
        {
            if (m_tiles[x][y].flagged)
                cout << '*';
            else if (m_tiles[x][y].checked)
            {
                if (m_tiles[x][y].adjecentMines == 0)
                    cout << '.';
                else
                    cout << m_tiles[x][y].adjecentMines;
            }
            else
                cout << '?';
        }
        cout << '\n';
    }
    cout << endl;
}
