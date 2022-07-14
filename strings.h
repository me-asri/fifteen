#ifndef FIFTEEN_STRINGS_H
#define FIFTEEN_STRINGS_H

namespace strings
{
    inline constexpr char ALERT_HELP[] = "Move the blank tile using W A S D keys or edit a tile directly by clicking on that tile\n"
                                         "Press the \"Shuffle\" button to randomize the puzzle\n\n"
                                         "Press the \"Solve\" button to solve the current puzzle using IDA* with selected heuristic\n"
                                         "- Use Linear Conflict heuristic for faster results";
    inline constexpr char ALERT_ALREADY_SOLVED[] = "Puzzle already solved";
    inline constexpr char ALERT_UNSOLVABLE_PUZZLE[] = "Puzzle unsolvable";
    inline constexpr char ALERT_SOLVE_FAILED[] = "Failed to solve puzzle";
    inline constexpr char ALERT_NOT_INT_VALUE[] = "Value must be an integer";
    inline constexpr char ALERT_OOB_VALUE[] = "Value must be between 1 and %u inclusive or empty for blank tile";

    inline constexpr char BUTTON_SOLVE[] = "Solve";
    inline constexpr char BUTTON_SOLVING[] = "Solving...";
    inline constexpr char BUTTON_SHUFFLE[] = "Shuffle";
    inline constexpr char BUTTON_ABORT[] = "Abort";
    inline constexpr char BUTTON_RETURN[] = "Return";
    inline constexpr char BUTTON_STEP[] = "Step ";

    inline constexpr char EXCEPT_CANCELLED[] = "Cancelled by user";
    inline constexpr char EXCEPT_MAX_THRESHOLD[] = "Max threshold reached";
    inline constexpr char EXCEPT_UNSOLVABLE_PUZZLE[] = "Unsolvable puzzle";
}

#endif