#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <random>

namespace snake {

inline constexpr int kBoardWidth = 30;
inline constexpr int kBoardHeight = 20;
inline constexpr std::size_t kBoardArea =
    static_cast<std::size_t>(kBoardWidth * kBoardHeight);

struct Point {
    int x;
    int y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
};

enum class Direction {
    stopped,
    left,
    right,
    up,
    down
};

class Game {
public:
    Game();
    explicit Game(std::uint32_t seed);

    bool queueDirection(Direction direction);
    bool requestQuit();
    bool update();

    bool finished() const;
    bool gameOver() const;
    bool won() const;
    bool quitRequested() const;
    int score() const;
    Direction direction() const;
    Point fruit() const;
    const std::deque<Point>& snake() const;

private:
    static std::size_t cellIndex(Point point);
    static bool areOpposite(Direction first, Direction second);
    void spawnFruit();

    std::deque<Point> snake_;
    std::array<bool, kBoardArea> occupied_{};
    Point fruit_{0, 0};
    Direction direction_ = Direction::stopped;
    std::optional<Direction> pendingDirection_;
    int score_ = 0;
    bool gameOver_ = false;
    bool won_ = false;
    bool quit_ = false;
    std::mt19937 random_;
};

}  // namespace snake
