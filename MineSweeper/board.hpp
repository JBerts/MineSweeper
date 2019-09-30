#pragma once

#include <vector>

class Board
{
public:
	Board(int width, int height, int mineCount);

    int size() const { return m_width * m_height; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    int getNumMines() const { return m_mineCount; }

    int check(int x, int y) const;
    void dump();

private:
    struct Tile
    {
        bool mine = false;
        int adjecentMineCount = 0;
    };

	int m_width;
	int m_height;
    int m_mineCount;
    std::vector<std::vector<Tile>> m_tiles;
};

