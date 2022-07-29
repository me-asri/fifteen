#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>
#include <functional>
#include <sstream>

#include <Fl/Fl.H>
#include <Fl/Fl_Widget.H>
#include <Fl/fl_ask.H>

#include "main.fl.h"
#include "edit.fl.h"

#include "strings.h"
#include "puzzle.h"
#include "heuristic.h"

static const char FLTK_SCHEME[] = "gleam";

class FifteenApp
{
private:
    static const int PUZZLE_SIZE{15};

    enum UiMode
    {
        DEFAULT,
        SOLVING,
        SOLVER
    };

    MainUi ui{};
    UiMode mode{UiMode::DEFAULT};

    Puzzle puzzle{PUZZLE_SIZE};
    std::shared_ptr<Puzzle::Heuristic> heuristic{};

    std::atomic<bool> solving{false};

    std::vector<Puzzle> solverStates{};
    int solverStep{-1};
    long double secElapsed{};

    struct ThreadResult
    {
        enum Result
        {
            SOLVED,
            CANCELLED,
            UNSOLVABLE,
            FAILURE
        };

        FifteenApp *app{};

        const Result result{};
        const std::vector<Puzzle> states{};
        const std::chrono::nanoseconds timeElapsed{};

        ThreadResult(FifteenApp *app, Result result, std::vector<Puzzle> &&states, const std::chrono::nanoseconds timeElapsed)
            : app(app), result(result), states(std::move(states)), timeElapsed(timeElapsed){};
    };

    struct TileData
    {
        FifteenApp *app{};
        int index{};

        TileData(FifteenApp *app, int index) : app(app), index(index){};
    };

public:
    FifteenApp()
    {
        // Set event handler on main window
        ui.window->setHandler(std::bind(&FifteenApp::eventHandler, this, std::placeholders::_1));

        // Set callbacks for UI elements
        setCallbacks();

        // Select linear conflict heuristic as default
        ui.linRadButton->set();
        heuristic = std::make_shared<Heuristic::ManhattanDistanceHeuristic>();

        updateUi(puzzle);

        ui.show();
    };

    ~FifteenApp()
    {
        freeUserData();
    };

private:
    int eventHandler(int event)
    {
        // We only care about keyboard events
        if (event != FL_SHORTCUT)
            return 0;

        if (mode != UiMode::DEFAULT)
            return 0;

        char key{(char)Fl::event_key()};
        switch (key)
        {
        case 'w':
            puzzle.move(Puzzle::Move::UP);
            break;
        case 's':
            puzzle.move(Puzzle::Move::DOWN);
            break;
        case 'a':
            puzzle.move(Puzzle::Move::LEFT);
            break;
        case 'd':
            puzzle.move(Puzzle::Move::RIGHT);
            break;
        default:
            return 0;
        }
        updateUi(puzzle);

        return 1;
    }

    bool editButtonCb(int index, const char *s)
    {
        std::string input{s};

        int value{};
        if (input.length() == 0)
        {
            value = 0;
        }
        else
        {
            try
            {
                value = std::stoi(input);
            }
            catch (const std::exception &e)
            {
                fl_alert(strings::ALERT_NOT_INT_VALUE);
                return false;
            }
            if (value <= 0 || value > puzzle.getSize())
            {
                fl_alert(strings::ALERT_OOB_VALUE, puzzle.getSize());
                return false;
            }
        }

        puzzle.set(index, value);
        updateUi(puzzle);

        return true;
    }

    void editTileCb(int index)
    {
        if (mode != UiMode::DEFAULT)
            return;

        EditDialog editDialog{};

        struct EditData
        {
            FifteenApp *app{};
            EditDialog *dialog{};
            int index{};

            EditData(FifteenApp *app, EditDialog *dialog, int index)
                : app(app), dialog(dialog), index(index){};
        };

        // Set default input value to current tile value if not zero
        int defVal{puzzle.get(index)};
        if (defVal > 0)
            editDialog.valueInput->value(std::to_string(defVal).c_str());

        editDialog.editButton->callback([](Fl_Widget *, void *d)
                                        {
            EditData *data = static_cast<EditData *>(d);
            if (data->app->editButtonCb(data->index, data->dialog->valueInput->value()))
            {
                data->dialog->hide();
                delete data; 
            } },
                                        new EditData(this, &editDialog, index));
        editDialog.cancelButton->callback([](Fl_Widget *, void *d)
                                          { static_cast<EditDialog *>(d)->hide(); },
                                          &editDialog);

        editDialog.show();
        while (editDialog.shown())
            Fl::wait();
    }

    void nextButtonCb()
    {
        solverStep++;
        updateStepLabel();

        if (!ui.prevButton->active())
            ui.prevButton->activate();

        updateUi(solverStates[solverStep]);

        if (solverStep + 1 == static_cast<int>(solverStates.size()))
            ui.nextButton->deactivate();
    }

    void prevButtonCb()
    {
        solverStep--;
        updateStepLabel();

        if (!ui.nextButton->active())
            ui.nextButton->activate();

        updateUi(solverStates[solverStep]);

        if (solverStep == 0)
            ui.prevButton->deactivate();
    }

    void hueChangeCb(std::shared_ptr<Puzzle::Heuristic> heuristic)
    {
        this->heuristic = heuristic;
        updateUi(puzzle);
    }

    void solveThreadCb(ThreadResult *threadResult)
    {
        solving = false;

        switch (threadResult->result)
        {
        case ThreadResult::SOLVED:
            solverStates = std::move(threadResult->states);
            solverStep = 0;
            secElapsed = threadResult->timeElapsed.count() / 1000000000.0L;

            setUiMode(UiMode::SOLVER);
            break;
        case ThreadResult::UNSOLVABLE:
            fl_alert(strings::ALERT_UNSOLVABLE_PUZZLE);

            setUiMode(UiMode::DEFAULT);
            break;
        case ThreadResult::FAILURE:
            fl_alert(strings::ALERT_SOLVE_FAILED);

            setUiMode(UiMode::DEFAULT);
            break;
        case ThreadResult::CANCELLED:
            setUiMode(UiMode::DEFAULT);
            break;
        }
    }

    void solveButtonCb()
    {
        if (puzzle.isSolved())
        {
            fl_alert(strings::ALERT_ALREADY_SOLVED);
            return;
        }

        setUiMode(UiMode::SOLVING);
        spawnSolverThread();
    }

    void spawnSolverThread()
    {
        solving = true;
        std::thread thread([&]()
                           {
            ThreadResult::Result result{ThreadResult::Result::SOLVED};
            std::vector<Puzzle> states{};
            auto startTime{std::chrono::high_resolution_clock::now()};

            try
            {
                states = std::move(puzzle.solve(*heuristic, solving));
            }
            catch (Puzzle::CancelledException &)
            {
                result = ThreadResult::Result::CANCELLED;
            }
            catch (Puzzle::UnsolvableException &)
            {
                result = ThreadResult::Result::UNSOLVABLE;
            }
            catch (...)
            {
                result = ThreadResult::Result::FAILURE;
            }

            auto endTime = std::chrono::high_resolution_clock::now();
            auto timeElapsed{std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime)};

            ThreadResult *threadResult{new ThreadResult(this, result, std::move(states), timeElapsed)};

            Fl::awake([](void *d){
                ThreadResult *threadResult{static_cast<ThreadResult *>(d)};
                threadResult->app->solveThreadCb(threadResult);
                delete threadResult;
            }, threadResult); });
        thread.detach();
    }

    void shuffleButtonCb()
    {
        switch (mode)
        {
        // Shuffle
        case UiMode::DEFAULT:
            // Create a new puzzle if not solvable
            if (!puzzle.isSolvable())
                puzzle = Puzzle(PUZZLE_SIZE);
            else
                puzzle.shuffle();

            updateUi(puzzle);

            break;
        // Abort
        case UiMode::SOLVING:
            solving = false;
            setUiMode(UiMode::DEFAULT);

            break;
        // Return
        case UiMode::SOLVER:
            setUiMode(UiMode::DEFAULT);

            updateUi(puzzle);

            solverStep = -1;
            solverStates.clear();

            break;
        default:
            break;
        }
    };

    void setUiMode(UiMode mode)
    {
        this->mode = mode;

        switch (mode)
        {
        case DEFAULT:
            ui.solveButton->activate();
            ui.solveButton->label(strings::BUTTON_SOLVE);

            ui.shuffleButton->label(strings::BUTTON_SHUFFLE);

            ui.prevButton->deactivate();
            ui.nextButton->deactivate();

            ui.heuGroup->activate();

            break;
        case SOLVING:
            ui.elapsedOutput->value(nullptr);

            ui.solveButton->deactivate();
            ui.solveButton->label(strings::BUTTON_SOLVING);

            ui.shuffleButton->label(strings::BUTTON_ABORT);

            ui.heuGroup->deactivate();

            break;
        case SOLVER:
            updateElapsedLabel();

            updateStepLabel();
            ui.solveButton->deactivate();

            ui.shuffleButton->label(strings::BUTTON_RETURN);

            ui.nextButton->activate();
        }
    }

    void updateElapsedLabel()
    {
        std::ostringstream secStringStream{};
        secStringStream.precision(4);
        secStringStream << std::fixed << secElapsed;
        secStringStream << 's';
        ui.elapsedOutput->value(secStringStream.str().c_str());
    }

    void updateStepLabel()
    {
        std::ostringstream stepStringStream{};
        stepStringStream << strings::BUTTON_STEP << solverStep << '/' << (solverStates.size() - 1);
        ui.solveButton->copy_label(stepStringStream.str().c_str());
    }

    void freeUserData()
    {
        Fl_Widget *const *tiles{ui.puzzleGroup->array()};
        int count{ui.puzzleGroup->children()};

        for (int n{0}; n < count; n++)
        {
            Fl_Widget *const tile{tiles[n]};

            tile->callback(static_cast<Fl_Callback *>(nullptr));
            TileData *data{static_cast<TileData *>(tile->user_data())};
            delete data;
        }
    }

    void setCallbacks()
    {
        ui.solveButton->callback([](Fl_Widget *, void *d)
                                 { static_cast<FifteenApp *>(d)->solveButtonCb(); },
                                 this);

        ui.helpButton->callback([](Fl_Widget *, void *d)
                                { fl_alert(strings::ALERT_HELP); });

        ui.shuffleButton->callback([](Fl_Widget *, void *d)
                                   { static_cast<FifteenApp *>(d)->shuffleButtonCb(); },
                                   this);

        ui.linRadButton->callback([](Fl_Widget *, void *d)
                                  { static_cast<FifteenApp *>(d)->hueChangeCb(std::make_shared<Heuristic::LinearConflictHeuristic>()); },
                                  this);
        ui.manRadButton->callback([](Fl_Widget *, void *d)
                                  { static_cast<FifteenApp *>(d)->hueChangeCb(std::make_shared<Heuristic::ManhattanDistanceHeuristic>()); },
                                  this);
        ui.misRadButton->callback([](Fl_Widget *, void *d)
                                  { static_cast<FifteenApp *>(d)->hueChangeCb(std::make_shared<Heuristic::MisplacedTilesHeuristic>()); },
                                  this);

        ui.nextButton->callback([](Fl_Widget *, void *d)
                                { static_cast<FifteenApp *>(d)->nextButtonCb(); },
                                this);
        ui.prevButton->callback([](Fl_Widget *, void *d)
                                { static_cast<FifteenApp *>(d)->prevButtonCb(); },
                                this);

        Fl_Widget *const *tiles{ui.puzzleGroup->array()};
        int count{ui.puzzleGroup->children()};

        for (int n{0}; n < count; n++)
        {
            Fl_Widget *const tile{tiles[n]};

            tile->callback([](Fl_Widget *, void *d)
                           {
                TileData *data{static_cast<TileData *>(d)};
                data->app->editTileCb(data->index); },
                           new TileData(this, n));
        }
    }

    void updateUi(const Puzzle &puzzle)
    {
        Fl_Widget *const *tiles{ui.puzzleGroup->array()};
        int count{ui.puzzleGroup->children()};

        for (int n{0}; n < count; n++)
        {
            Fl_Widget *const tile{tiles[n]};

            int value{puzzle.get(n)};
            if (value > 0)
            {
                std::string tileStr{std::to_string(value)};
                tile->copy_label(tileStr.c_str());
            }
            else
            {
                tile->label(NULL);
            }
        }

        std::string hueStr{std::to_string((*heuristic)(puzzle))};
        ui.heuOutput->value(hueStr.c_str());

        std::string invStr{std::to_string(puzzle.inversionCount())};
        ui.invOutput->value(invStr.c_str());
    }
};

int main()
{
    FifteenApp app{};

    Fl::scheme(FLTK_SCHEME);

    // Enable multithreading support in FLTK
    Fl::lock();

    return Fl::run();
}
