#ifndef FIFTEEN_PUZZLE_H
#define FIFTEEN_PUZZLE_H

#include <vector>
#include <deque>
#include <atomic>
#include <exception>

#include "strings.h"

class Puzzle
{
public:
    enum Move
    {
        UP,
        DOWN,
        LEFT,
        RIGHT
    };

    class Heuristic
    {
    public:
        virtual unsigned int operator()(const Puzzle &p) const = 0;
        virtual ~Heuristic() = default;
    };

    class CancelledException : public std::exception
    {
    public:
        CancelledException()
            : std::exception(){};

        const char *what() { return strings::EXCEPT_CANCELLED; }
    };

    class MaxThresholdException : public std::exception
    {
    public:
        MaxThresholdException()
            : std::exception(){};

        const char *what() { return strings::EXCEPT_MAX_THRESHOLD; }
    };

    class UnsolvableException : public std::exception
    {
    public:
        UnsolvableException()
            : std::exception(){};

        const char *what() { return strings::EXCEPT_UNSOLVABLE_PUZZLE; }
    };

private:
    static unsigned int search(std::deque<Puzzle> &path, unsigned int moveCost, unsigned int threshold, const Heuristic &heuristic, std::atomic<bool> &running);

    int dimension;
    int *tiles;

    int blankRow;
    int blankCol;

    int &at(int row, int col);

public:
    Puzzle(int size);
    Puzzle(const Puzzle &p);
    Puzzle(Puzzle &&p);

    ~Puzzle();

    bool operator==(const Puzzle &p) const;
    Puzzle &operator=(const Puzzle &p);
    Puzzle &operator=(Puzzle &&p);

    int getDimension() const;
    int getSize() const;

    bool move(Move move);
    std::vector<Move> validMoves() const;
    void shuffle();

    int &get(int index) const;
    int &get(int row, int col) const;
    bool set(int index, int value);
    bool set(int row, int col, int value);

    unsigned int inversionCount() const;

    bool isSolved() const;
    bool isSolvable() const;
    std::vector<Puzzle> solve(const Heuristic &heuristic, std::atomic<bool> &running) const;
    std::vector<Puzzle> solve(const Heuristic &heuristic) const;
};

#endif