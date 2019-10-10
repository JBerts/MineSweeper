#include "pch.h"
#include "sweeper.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>

using namespace std;

Sweeper::Sweeper(const Board &board)
    : m_board(board),
      m_rnd(board.getWidth() - 1, board.getHeight() - 1),
      m_tiles(m_board.getWidth(), vector<TileInfo>(m_board.getHeight())),
      m_showProgress(false)
{
}

bool
Sweeper::solve(bool showProgress)
{
    m_showProgress = showProgress;
    int unsolved = m_board.size();
    while (unsolved > 0)
    {
        // Make a guess
        int x, y;
        do
        {
            x = m_rnd.x();
            y = m_rnd.y();
        } while (m_tiles[x][y].checked || m_tiles[x][y].flagged);
        ++m_guessed;

        if (!check(x, y))
            return false;

        --unsolved;
        if (showProgress)
            cout << "Pheu, lucky guess, " << unsolved << " left\n";
        if (unsolved == 0)
            break;

        bool stuck = false;
        while (!stuck)
        {
            int blanksSolved = solveBlanks();
            unsolved -= blanksSolved;
            if (showProgress)
            {
                cout << "Solved " << blanksSolved << " blanks, " << unsolved << " left\n";
                dump();
            }
            if (unsolved == 0)
                break;

            int singlesSolved = solveSingles();
            unsolved -= singlesSolved;
            if (showProgress)
            {
                cout << "Solved " << singlesSolved << " singles, " << unsolved << " left\n";
                dump();
            }
            if (unsolved == 0)
                break;

            if (unsolved > 0 && blanksSolved == 0 && singlesSolved == 0)
            {
                int restrictedSolved = solveRestricted();
                unsolved -= restrictedSolved;
                if (showProgress)
                {
                    cout << "Solved " << restrictedSolved << " restricted, " << unsolved << " left\n";
                    dump();
                }
                if (unsolved == 0)
                    break;
                if (restrictedSolved == 0)
                    stuck = true;
            }
        }
    }

    return true;
}

bool
Sweeper::check(int x, int y)
{
    if (m_tiles[x][y].checked)
        return true;

    auto n = m_board.check(x, y);
    ++m_checked;
    if (n < 0)
        return false;

    m_tiles[x][y].checked = true;
    m_tiles[x][y].restricted = false;
    m_tiles[x][y].adjecentMines = n;
    if (n == 0)
        m_blanks.push_back(make_pair(x, y));
    else
    {
        auto adjecent = getAdjecent(
                x, y,
                [](const TileInfo& ti)
                { return !ti.checked && !ti.flagged; });
        for (auto [x, y] : adjecent)
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

    if (y > 0 && pred(m_tiles[x][y - 1]))
        adjecent.push_back(make_pair(x, y - 1));
    if (y < m_board.getHeight() - 1 && pred(m_tiles[x][y + 1]))
        adjecent.push_back(make_pair(x, y + 1));
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
            int unchecked = countAdjecent(x, y, [](const TileInfo& ti) { return !ti.checked && !ti.flagged; });

            if (flagged == ti.adjecentMines && unchecked > 0)
                checkAdjecent(x, y);
            else if (unchecked > 0 && flagged + unchecked == ti.adjecentMines)
            {
                for (auto [x2, y2] : getAdjecent(x, y))
                {
                    auto &ti2 = m_tiles[x2][y2];
                    if (!ti2.checked && !ti2.flagged)
                    {
                        ti2.flagged = true;
                        ti2.restricted = false;
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
    int checkedBefore = m_checked;

    groupRestricted();
    if (m_showProgress)
    {
        cout << "Found " << m_restrictedGroups.size()
             << " groups of restricted\n";
        dump();
    }
    for (auto &restricted : m_restrictedGroups)
    {
        if (m_showProgress)
        {
          cout << "Hmm, " << fixed << setprecision(0) << pow(2, restricted.size())
               << " possible solutions to check. This might take a while...\n";
        }

        vector<bool> path;
        int valid = testRestricted(path, restricted);
        if (m_showProgress)
            cout << "\nValid solutions: " << valid << "\n";
        for (auto [x, y] : restricted)
        {
            if (m_tiles[x][y].restrictedSolutionMines == 0)
                check(x, y);
        }
    }

    return m_checked - checkedBefore;
}
    
void
Sweeper::groupRestricted()
{
    m_restrictedGroups.clear();
    for (int x = 0; x < m_board.getWidth(); x++)
    {
        for (int y = 0; y < m_board.getHeight(); y++)
        {
            m_tiles[x][y].restrictedGroup = -1;
            m_tiles[x][y].restrictedSolutionMines = 0;
        }
    }

    for (int x = 0; x < m_board.getWidth(); x++)
    {
        for (int y = 0; y < m_board.getHeight(); y++)
        {
            auto &ti = m_tiles[x][y];
            if (!ti.checked && !ti.flagged && ti.restricted && ti.restrictedGroup < 0)
            {
                ti.restrictedGroup = m_restrictedGroups.size();
                m_restrictedGroups.emplace_back();
                m_restrictedGroups.back().emplace_back(x, y);
                followRestricted(x, y, ti.restrictedGroup);
            }
        }
    }
}

int
Sweeper::followRestricted(int x, int y, int group)
{
    auto adjecent = getAdjecent(
        x, y,
        [group](const TileInfo& ti)
        {
            return !ti.checked && !ti.flagged && ti.restricted && ti.restrictedGroup != group;
        });
    for (auto [x2, y2] : adjecent)
    {
        auto &ti = m_tiles[x2][y2];
        if (ti.checked || ti.flagged || !ti.restricted || ti.restrictedGroup == group)
            continue;

        ti.restrictedGroup = group;
        m_restrictedGroups.at(group).emplace_back(x2, y2);
        followRestricted(x2, y2, group);
    }
    return adjecent.size();
}

int
Sweeper::testRestricted(vector<bool> &path, const vector<pair<int, int>> &restricted)
{
    int valid = 0;
    if (path.size() == restricted.size())
    {
        auto pit = path.begin();
        for(auto rit = restricted.begin(); rit != restricted.end(); ++rit)
        {
            auto [x, y] = *rit;
            m_tiles[x][y].testMine = *pit++;
        }
        valid += verifyRestricted(restricted) ? 1 : 0;
    }
    else
    {
        path.push_back(true);
        valid += testRestricted(path, restricted);
        path.pop_back();
        path.push_back(false);
        valid += testRestricted(path, restricted);
        path.pop_back();
    }

    return valid;
}

bool
Sweeper::verifyRestricted(const vector<pair<int, int>> &restricted)
{
    if (m_showProgress)
    {
        cout << "\b";
        static int i = 0;
        switch (i++ % 3)
        {
        case 0: cout << '-'; break;
        case 1: cout << '/'; break;
        case 2: cout << '\\'; break;
        }
    }
    bool ok = true;
    for (auto [x, y] : restricted)
    {
        auto adjChecked = getAdjecent(x, y, [](const TileInfo &ti){ return ti.checked; });
        for (auto [x2, y2] : adjChecked)
        {
            int flagged = countAdjecent(x2, y2, [](const TileInfo& ti) { return ti.flagged; });
            int testMines = countAdjecent(x2, y2, [](const TileInfo &ti){ return ti.testMine; });
            if (flagged + testMines != m_tiles[x2][y2].adjecentMines)
            {
                ok = false;
                break;
            }
        }
        if (!ok)
            break;
    }

    if (ok)
    {
        for (auto [x, y] : restricted)
            m_tiles[x][y].restrictedSolutionMines += m_tiles[x][y].testMine;
    }
    return ok;
}

void
Sweeper::dump()
{
    for (int y = 0; y < m_board.getHeight(); ++y)
    {
        for (int x = 0; x < m_board.getWidth(); ++x)
        {
            auto &ti = m_tiles[x][y];

            if (ti.flagged)
                cout << '*';
            else if (ti.checked)
            {
                if (ti.adjecentMines == 0)
                    cout << '.';
                else
                    cout << ti.adjecentMines;
            }
            else if (ti.restricted)
            {
                cout << "\x1b[37m\x1b[" << 42 + ti.restrictedGroup << "m";
                cout << ti.restrictedSolutionMines;
                cout << "\x1b[0m";
            }
            else
                cout << '?';
        }
        cout << '\n';
    }
    cout << endl;
}
