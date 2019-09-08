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
Sweeper::solve()
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
            stuck = (blanksSolved == 0) && (singlesSolved == 0);
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
Sweeper::getAdjecent(int x, int y)
{
    vector<pair<int, int>> adjecent;
    if (x > 0)
    {
        adjecent.push_back(make_pair(x - 1, y));
        if (y > 0)
            adjecent.push_back(make_pair(x - 1, y - 1));
        if (y < m_board.getHeight() - 1)
            adjecent.push_back(make_pair(x - 1, y + 1));
    }
    if (x < m_board.getWidth() - 1)
    {
        adjecent.push_back(make_pair(x + 1, y));
        if (y > 0)
            adjecent.push_back(make_pair(x + 1, y - 1));
        if (y < m_board.getHeight() - 1)
            adjecent.push_back(make_pair(x + 1, y + 1));
    }
    if (y > 0)
        adjecent.push_back(make_pair(x, y - 1));
    if (y < m_board.getHeight() - 1)
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
Sweeper::countAdjecent(int x, int y, function<bool(const TileInfo&)> pred)
{
    int count = 0;
    for (auto[x2, y2] : getAdjecent(x, y))
    {
        if (pred(m_tiles[x2][y2]))
            ++count;
    }

    return count;
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
    return m_checked - checkedBefore + m_flagged - flaggedBefore;
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
                    cout << ' ';
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
