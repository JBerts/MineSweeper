#pragma once

#include <random>

class RndGen
{
public:
    RndGen(int maxX, int maxY);

    int x();
    int y();

private:
    std::random_device m_rd;
    std::default_random_engine m_re;
    std::uniform_int_distribution<int> m_rndX;
    std::uniform_int_distribution<int> m_rndY;
};

