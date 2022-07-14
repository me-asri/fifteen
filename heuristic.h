#ifndef FIFTEEN_HEURISTIC_H
#define FIFTEEN_HEURISTIC_H

#include "puzzle.h"

namespace Heuristic
{
    class ManhattanDistanceHeuristic : public Puzzle::Heuristic
    {
    public:
        unsigned int operator()(const Puzzle &p) const;
    };

    class LinearConflictHeuristic : public Puzzle::Heuristic
    {
    public:
        unsigned int operator()(const Puzzle &p) const;
    };

    class MisplacedTilesHeuristic : public Puzzle::Heuristic
    {
    public:
        unsigned int operator()(const Puzzle &p) const;
    };
};

#endif