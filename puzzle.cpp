#include <algorithm>
#include <vector>
#include <deque>
#include <limits>
#include <random>

#include "puzzle.h"

Puzzle::Puzzle(int size)
{
    dimension = std::sqrt(size + 1); // e.g 15-puzzle is 4 * 4

    // Create solved puzzle
    int len{dimension * dimension};
    int lastIndex{len - 1};

    int n = 1;
    tiles = new int[len];
    for (int i{0}; i < lastIndex; i++)
        tiles[i] = n++;

    tiles[lastIndex] = 0;
    blankRow = dimension - 1;
    blankCol = dimension - 1;

    // Make random moves
    shuffle();
}

Puzzle::Puzzle(const Puzzle &p)
{
    dimension = p.dimension;

    blankRow = p.blankRow;
    blankCol = p.blankCol;

    tiles = new int[dimension * dimension];
    std::copy(p.tiles, p.tiles + (dimension * dimension), tiles);
}

Puzzle::Puzzle(Puzzle &&p)
{
    dimension = p.dimension;

    blankRow = p.blankRow;
    blankCol = p.blankCol;

    tiles = p.tiles;
    p.tiles = nullptr;
}

Puzzle::~Puzzle()
{
    if (tiles != nullptr)
        delete[] tiles;
}

int &Puzzle::at(int row, int col)
{
    return tiles[(row * dimension) + col];
}

bool Puzzle::operator==(const Puzzle &p) const
{
    if (dimension != p.dimension)
        return false;

    if (blankRow != p.blankRow || blankCol != p.blankCol)
        return false;

    return std::equal(tiles, tiles + (dimension * dimension), p.tiles);
}

Puzzle &Puzzle::operator=(const Puzzle &p)
{
    dimension = p.dimension;

    blankRow = p.blankRow;
    blankCol = p.blankCol;

    std::copy(p.tiles, p.tiles + (dimension * dimension), tiles);

    return *this;
}

Puzzle &Puzzle::operator=(Puzzle &&p)
{
    dimension = p.dimension;

    blankRow = p.blankRow;
    blankCol = p.blankCol;

    delete tiles;
    tiles = p.tiles;
    p.tiles = nullptr;

    return *this;
}

int Puzzle::getDimension() const
{
    return dimension;
}

int Puzzle::getSize() const
{
    return (dimension * dimension) - 1;
}

bool Puzzle::move(Move move)
{
    switch (move)
    {
    case UP:
        if (blankRow == 0)
            return false;

        std::swap(at(blankRow, blankCol), at(blankRow - 1, blankCol));
        blankRow--;
        return true;
    case DOWN:
        if (blankRow == dimension - 1)
            return false;

        std::swap(at(blankRow, blankCol), at(blankRow + 1, blankCol));
        blankRow++;
        return true;
    case LEFT:
        if (blankCol == 0)
            return false;

        std::swap(at(blankRow, blankCol), at(blankRow, blankCol - 1));
        blankCol--;
        return true;
    case RIGHT:
        if (blankCol == dimension - 1)
            return false;

        std::swap(at(blankRow, blankCol), at(blankRow, blankCol + 1));
        blankCol++;
        return true;
    default:
        return false;
    }
}

std::vector<Puzzle::Move> Puzzle::validMoves() const
{
    std::vector<Move> moves;

    if (blankRow > 0)
        moves.push_back(UP);
    if (blankRow < dimension - 1)
        moves.push_back(DOWN);
    if (blankCol > 0)
        moves.push_back(LEFT);
    if (blankCol < dimension - 1)
        moves.push_back(RIGHT);

    return moves;
}

void Puzzle::shuffle()
{
    std::random_device dev{};
    std::mt19937 rng{dev()};
    std::uniform_int_distribution<int> dist{0, 4};

    // Make random moves
    int times{(getSize() + 1) * 4};
    for (int i{0}; i < times; i++)
    {
        std::vector<Move> moves{validMoves()};
        move(moves[dist(rng) % moves.size()]);
    }
}

int &Puzzle::get(int index) const
{
    return tiles[index];
}

int &Puzzle::get(int row, int col) const
{
    return get((row * dimension) + col);
}

bool Puzzle::set(int index, int value)
{
    if (value == 0)
    {
        std::swap(tiles[index], at(blankRow, blankCol));
        blankRow = index / dimension;
        blankCol = index % dimension;
    }
    else
    {
        bool found = false;

        int len{dimension * dimension};
        int n{0};
        for (n = 0; n < len; n++)
        {
            if (tiles[n] == value)
            {
                found = true;
                break;
            }
        }
        if (!found)
            return false;

        std::swap(tiles[index], tiles[n]);
        if (tiles[n] == 0)
        {
            blankRow = n / dimension;
            blankCol = n % dimension;
        }
    }

    return true;
}

bool Puzzle::set(int row, int col, int value)
{
    return set((row * dimension) + col, value);
}

unsigned int Puzzle::inversionCount() const
{
    unsigned int count{0};

    int len{dimension * dimension};
    int last{len - 1};

    for (int i{0}; i < last; i++)
    {
        // Skip blank tile
        if (tiles[i] == 0)
            continue;

        for (int j{i + 1}; j < len; j++)
        {
            // Skip blank tile
            if (tiles[j] == 0)
                continue;

            if (tiles[i] > tiles[j])
                count++;
        }
    }

    return count;
}

bool Puzzle::isSolvable() const
{
    int invCount = inversionCount();

    if (dimension % 2 == 0)
    {
        // If puzzle size is even, we have to check the row of blank tile.
        // If blank tile on even row, inversion count should be odd
        // else it should be even
        if (blankRow % 2 == 0)
            return (invCount % 2 != 0);
        else
            return (invCount % 2 == 0);
    }
    else
    {
        // If puzzle size is odd, then puzzle is solvable if iversion count is even
        return (invCount % 2 == 0);
    }
}

bool Puzzle::isSolved() const
{
    if (blankRow != dimension - 1 || blankCol != dimension - 1)
        return false;

    return (inversionCount() == 0);
}

std::vector<Puzzle> Puzzle::solve(const Heuristic &heuristic, std::atomic<bool> &running) const
{
    if (!isSolvable())
        throw UnsolvableException();

    std::deque<Puzzle> path;
    path.push_back(*this); // Add first state

    unsigned int threshold = heuristic(*this);

    while (true)
    {
        if (!running)
            throw CancelledException();

        unsigned int result = search(path, 0, threshold, heuristic, running);

        if (result == 0)
            break;

        if (result == std::numeric_limits<unsigned int>::max())
            throw MaxThresholdException();

        threshold = result;
    }

    // Move states from deque to vector for ease of use
    std::vector<Puzzle> pathVec;
    pathVec.reserve(path.size());
    std::move(std::begin(path), std::end(path), std::back_inserter(pathVec));

    running = false;
    return pathVec;
}

std::vector<Puzzle> Puzzle::solve(const Heuristic &heuristic) const
{
    std::atomic<bool> running{true};
    return solve(heuristic, running);
}

unsigned int Puzzle::search(std::deque<Puzzle> &path, unsigned int moveCost, unsigned int threshold, const Heuristic &heuristic, std::atomic<bool> &running)
{
    const Puzzle &state = *path.rbegin(); // Get last state

    unsigned int h = heuristic(state);
    // A heuristic value of 0 means we have reached the goal
    if (h == 0)
        return 0; // Found

    if (!running)
        throw CancelledException();

    unsigned int cost = moveCost + h;
    if (cost > threshold)
        return cost;

    unsigned int min = std::numeric_limits<unsigned int>::max();
    for (Move &nextMove : state.validMoves())
    {
        Puzzle nextState{state};
        nextState.move(nextMove);

        if (std::find(path.begin(), path.end(), nextState) == path.end())
        {
            path.push_back(std::move(nextState));

            unsigned int temp = search(path, moveCost + 1, threshold, heuristic, running);
            if (temp == 0)
                return 0; // Found
            if (temp < min)
                min = temp;

            // Pop state from path if not leading to goal
            path.pop_back();
        }
    }

    return min;
}