// MineSweeper.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <string>

#include "board.hpp"
#include "sweeper.hpp"

using namespace std;

int main()
{
    cout << "Minesweeper!" << endl;
    auto board = Board(16, 16, 20);
    auto solver = Sweeper(board);
    auto ok = solver.solve(true);
    if (ok)
    {
        cout << "\nSuccess :-)\n";
        return 0;
    }
    else
    {
        cout << "\nFail :-(\n";
        cout << "Found " << solver.getFlagged() << " of "
            << board.getNumMines() << " mines.\n";
        cout << "Solution:\n";
        board.dump();
        return 1;
    }
}
