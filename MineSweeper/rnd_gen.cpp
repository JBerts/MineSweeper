#include "pch.h"
#include "rnd_gen.hpp"

using namespace std;

RndGen::RndGen(int maxX, int maxY)
    : m_re(m_rd()),
      m_rndX(0, maxX),
      m_rndY(0, maxY)
{
}

int 
RndGen::x()
{
    return m_rndX(m_re);
}

int 
RndGen::y()
{
    return m_rndY(m_re);
}

