#include <algorithm>
#include <chrono>
#include <csignal>
#include <cctype>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

namespace {

constexpr int kWidth = 30;
constexpr int kHeight = 20;
constexpr auto kFrameTime = std::chrono::milliseconds(100);

volatile std::sig_atomic_t interrupted = 0;

void handleSignal(int) {
    interrupted = 1;
}

class Terminal {
public:
    Terminal() : configured_(isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &original_) == 0) {
        if (configured_) {
            termios raw = original_;
            raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
            raw.c_cc[VMIN] = 0;
            raw.c_cc[VTIME] = 0;
            configured_ = tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == 0;
        }

        std::cout << "\x1b[2J\x1b[H\x1b[?25l" << std::flush;
    }

    ~Terminal() {
        if (configured_) {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_);
        }
        std::cout << "\x1b[?25h\x1b[0m" << std::flush;
    }

    Terminal(const Terminal&) = delete;
    Terminal& operator=(const Terminal&) = delete;

    bool readKey(char& key) const {
        fd_set input;
        FD_ZERO(&input);
        FD_SET(STDIN_FILENO, &input);
        timeval timeout{0, 0};

        const int ready = select(STDIN_FILENO + 1, &input, nullptr, nullptr, &timeout);
        return ready > 0 && read(STDIN_FILENO, &key, 1) == 1;
    }

private:
    termios original_{};
    bool configured_;
};

struct Point {
    int x;
    int y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

enum class Direction {
    stopped,
    left,
    right,
    up,
    down
};

bool areOpposite(Direction first, Direction second) {
    return (first == Direction::left && second == Direction::right) ||
           (first == Direction::right && second == Direction::left) ||
           (first == Direction::up && second == Direction::down) ||
           (first == Direction::down && second == Direction::up);
}

class Game {
public:
    Game() : snake_{{kWidth / 2, kHeight / 2}}, random_(std::random_device{}()) {
        spawnFruit();
    }

    void handleKey(char key) {
        switch (std::tolower(static_cast<unsigned char>(key))) {
            case 'a':
                changeDirection(Direction::left);
                break;
            case 'd':
                changeDirection(Direction::right);
                break;
            case 'w':
                changeDirection(Direction::up);
                break;
            case 's':
                changeDirection(Direction::down);
                break;
            case 'q':
            case 'x':
                quit_ = true;
                break;
            default:
                break;
        }
    }

    void update() {
        if (direction_ == Direction::stopped || gameOver_ || quit_) {
            return;
        }

        Point next = snake_.front();
        switch (direction_) {
            case Direction::left:
                --next.x;
                break;
            case Direction::right:
                ++next.x;
                break;
            case Direction::up:
                --next.y;
                break;
            case Direction::down:
                ++next.y;
                break;
            case Direction::stopped:
                break;
        }

        next.x = (next.x + kWidth) % kWidth;
        next.y = (next.y + kHeight) % kHeight;

        const bool eating = next == fruit_;
        const auto collisionEnd = snake_.end() - (eating ? 0 : 1);
        if (std::find(snake_.begin(), collisionEnd, next) != collisionEnd) {
            gameOver_ = true;
            return;
        }

        snake_.insert(snake_.begin(), next);
        if (eating) {
            score_ += 10;
            spawnFruit();
        } else {
            snake_.pop_back();
        }
    }

    void draw() const {
        std::ostringstream screen;
        screen << "\x1b[H";
        screen << '+' << std::string(kWidth, '-') << "+\n";

        for (int y = 0; y < kHeight; ++y) {
            screen << '|';
            for (int x = 0; x < kWidth; ++x) {
                const Point cell{x, y};
                if (cell == snake_.front()) {
                    screen << 'O';
                } else if (cell == fruit_) {
                    screen << '@';
                } else if (std::find(snake_.begin() + 1, snake_.end(), cell) != snake_.end()) {
                    screen << 'o';
                } else {
                    screen << ' ';
                }
            }
            screen << "|\n";
        }

        screen << '+' << std::string(kWidth, '-') << "+\n";
        screen << "Score: " << score_ << "   WASD: move   Q/X: quit\n";

        if (won_) {
            screen << "You filled the board. You win!\n";
        } else if (gameOver_) {
            screen << "Game over! Final score: " << score_ << "\n";
        } else if (direction_ == Direction::stopped) {
            screen << "Press W, A, S, or D to start.\n";
        } else {
            screen << "                                  \n";
        }

        std::cout << screen.str() << std::flush;
    }

    bool finished() const {
        return gameOver_ || won_ || quit_;
    }

private:
    void changeDirection(Direction next) {
        if (snake_.size() > 1 && areOpposite(direction_, next)) {
            return;
        }
        direction_ = next;
    }

    void spawnFruit() {
        std::vector<Point> emptyCells;
        emptyCells.reserve(kWidth * kHeight - snake_.size());

        for (int y = 0; y < kHeight; ++y) {
            for (int x = 0; x < kWidth; ++x) {
                Point candidate{x, y};
                if (std::find(snake_.begin(), snake_.end(), candidate) == snake_.end()) {
                    emptyCells.push_back(candidate);
                }
            }
        }

        if (emptyCells.empty()) {
            won_ = true;
            return;
        }

        std::uniform_int_distribution<std::size_t> pick(0, emptyCells.size() - 1);
        fruit_ = emptyCells[pick(random_)];
    }

    std::vector<Point> snake_;
    Point fruit_{0, 0};
    Direction direction_ = Direction::stopped;
    int score_ = 0;
    bool gameOver_ = false;
    bool won_ = false;
    bool quit_ = false;
    std::mt19937 random_;
};

}  // namespace

int main() {
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    Terminal terminal;
    Game game;
    game.draw();

    while (!game.finished() && !interrupted) {
        const auto frameStarted = std::chrono::steady_clock::now();

        char key = '\0';
        while (terminal.readKey(key)) {
            game.handleKey(key);
        }

        game.update();
        game.draw();
        std::this_thread::sleep_until(frameStarted + kFrameTime);
    }

    if (!interrupted) {
        game.draw();
    }
    std::cout << '\n';
    return 0;
}
