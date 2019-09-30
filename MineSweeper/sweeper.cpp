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
    for (int x = 0; x < m_board.getWidth(); x++)
    {
        for (int y = 0; y < m_board.getHeight(); y++)
        {
        	m_tiles[x][y].restrictedGroup = -1;
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
                m_restrictedGroups.back().push_back(make_pair(x, y));
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
    for (auto[x2, y2] : adjecent)
    {
        m_tiles[x2][y2].restrictedGroup = group;
        m_restrictedGroups.at(group).push_back(make_pair(x2, y2));
        followRestricted(x2, y2, group);
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
                cout << "\x1b[" << 42 + ti.restrictedGroup << "m?" << "\x1b[0m";
            else
                cout << '?';
        }
        cout << '\n';
    }
    cout << endl;
}
