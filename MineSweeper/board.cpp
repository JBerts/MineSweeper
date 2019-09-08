#include "pch.h"
#include "board.hpp"

#include <iostream>
#include <random>

using namespace std;

Board::Board(int width, int height, int mineCount)
    : m_width(width),
      m_height(height),
      m_mineCount(mineCount),
      m_tiles(width, vector<Tile>(height))
{
    random_device rd;
    default_random_engine re(rd());
    uniform_int_distribution<int> rndX(0, width - 1);
    uniform_int_distribution<int> rndY(0, height - 1);

    for (int i = 0; i < m_mineCount; ++i)
        m_tiles.at(rndX(re)).at(rndY(re)).mine = true;

    for (int x = 0; x < m_width; ++x)
    {
        for (int y = 0; y < m_height; ++y)
        {
            auto tile = m_tiles.at(x).at(y);
            if (tile.mine)
            {
                if (x > 0)
                {
                    ++m_tiles.at(x - 1).at(y).adjecentMineCount;
                    if (y > 0)
                        ++m_tiles.at(x - 1).at(y - 1).adjecentMineCount;
                    if (y < m_height - 1)
                        ++m_tiles.at(x - 1).at(y + 1).adjecentMineCount;
                }

                if (x < m_width - 1)
                {
                    ++m_tiles.at(x + 1).at(y).adjecentMineCount;
                    if (y > 0)
                        ++m_tiles.at(x + 1).at(y - 1).adjecentMineCount;
                    if (y < m_height - 1)
                        ++m_tiles.at(x + 1).at(y + 1).adjecentMineCount;
                }
                if (y > 0)
                    ++m_tiles.at(x).at(y - 1).adjecentMineCount;
                if (y < m_height - 1)
                    ++m_tiles.at(x).at(y + 1).adjecentMineCount;
            }
        }
    }
}

int 
Board::check(int x, int y) const
{
    if (m_tiles.at(x).at(y).mine)
        return -1;

    return m_tiles.at(x).at(y).adjecentMineCount;
}

void 
Board::dump()
{
    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            if (m_tiles[x][y].mine)
                cout << '*';
            else
            {
                if (m_tiles[x][y].adjecentMineCount == 0)
                    cout << ' ';
                else
                    cout << m_tiles[x][y].adjecentMineCount;
            }
        }
        cout << '\n';
    }
    cout << endl;
}
