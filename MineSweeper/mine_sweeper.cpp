// MineSweeper.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#include "board.hpp"
#include "sweeper.hpp"

using namespace std;

int main()
{
    cout << "Minesweeper!" << endl;
    auto board = Board(16, 16, 40);

    auto solver = Sweeper(board);
    bool ok = solver.solve();
    cout << (ok ? "Success!" : "Failed!") << endl;
    solver.dump();
    board.dump();
}
