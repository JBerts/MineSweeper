#pragma once

#include <random>

class RndGen
{
public:
    RndGen(int maxX, int maxY);

    int x() const;
    int y() const;

private:
    std::random_device m_rd;
    mutable std::default_random_engine m_re;
    std::uniform_int_distribution<int> m_rndX;
    std::uniform_int_distribution<int> m_rndY;
};

