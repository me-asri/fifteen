#include "heuristic.h"

#include <cmath>

unsigned int Heuristic::ManhattanDistanceHeuristic::operator()(const Puzzle &p) const
{
    unsigned int distance{0};

    int dimension{p.getDimension()};

    for (int n{0}, len{dimension * dimension}; n < len; n++)
    {
        if (p.get(n) == 0)
            continue; // Skip blank tile

        int realValue{p.get(n) - 1};
        if (realValue == n)
            continue; // Skip if placed correctly

        // Manhattan distance of current tile's position and the expected position
        distance += std::abs((realValue / dimension) - (n / dimension)) +
                    std::abs((realValue % dimension) - (n % dimension));
    }

    return distance;
}

unsigned int Heuristic::LinearConflictHeuristic::operator()(const Puzzle &p) const
{
    unsigned int conflicts{0};

    int dimension{p.getDimension()};

    // For each row and column
    for (int i{0}; i < dimension; i++)
    {
        // Calculate conflicts in row
        for (int k{0}, l{dimension - 1}; k < l; k++)
        {
            int kVal = p.get(i, k);
            if (kVal == 0)
                continue;

            int kGoalRow{(kVal - 1) / dimension}, kGoalCol{(kVal - 1) % dimension};
            // Tj is to the right of Tk
            for (int j{k + 1}; j < dimension; j++)
            {
                int jVal = p.get(i, j);
                if (jVal == 0)
                    continue;

                int jGoalRow{(jVal - 1) / dimension}, jGoalCol{(jVal - 1) % dimension};
                // Goal position of both Tj and Tk must be on the same line
                if (jGoalRow != i || kGoalRow != i)
                    continue;
                // If goal position of Tj is to the left of Tk then there's a linear conflict
                if (jGoalCol < kGoalCol)
                    conflicts++;
            }
        }
        // Calculate conflicts in columns
        for (int k{0}, l{dimension - 1}; k < l; k++)
        {
            int kVal = p.get(k, i);
            if (kVal == 0)
                continue;

            int kGoalRow{(kVal - 1) / dimension}, kGoalCol{(kVal - 1) % dimension};
            for (int j{k + 1}; j < dimension; j++)
            {
                int jVal = p.get(j, i);
                if (jVal == 0)
                    continue;

                int jGoalRow{(jVal - 1) / dimension}, jGoalCol{(jVal - 1) % dimension};
                if (jGoalCol != i || kGoalCol != i)
                    continue;
                if (jGoalRow < kGoalRow)
                    conflicts++;
            }
        }
    }

    ManhattanDistanceHeuristic md{};
    return (conflicts * 2) + md(p);
}

unsigned int Heuristic::MisplacedTilesHeuristic::operator()(const Puzzle &p) const
{
    unsigned int misplaced{0};

    int dimension{p.getDimension()};

    for (int n{0}, l{dimension * dimension}; n < l; n++)
    {
        if (p.get(n) == 0)
            continue;

        if (p.get(n) != n + 1)
            misplaced++;
    }

    return misplaced;
}