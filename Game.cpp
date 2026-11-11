#include "Game.hpp"

#include <random>
#include <vector>

namespace snake {

Game::Game() : Game(std::random_device{}()) {}

Game::Game(std::uint32_t seed)
    : snake_{{kBoardWidth / 2, kBoardHeight / 2}}, random_(seed) {
    occupied_[cellIndex(snake_.front())] = true;
    spawnFruit();
}

bool Game::queueDirection(Direction next) {
    if (next == Direction::stopped || pendingDirection_.has_value() || finished()) {
        return false;
    }
    if (snake_.size() > 1 && areOpposite(direction_, next)) {
        return false;
    }

    pendingDirection_ = next;
    return true;
}

bool Game::requestQuit() {
    if (quit_) {
        return false;
    }
    quit_ = true;
    return true;
}

bool Game::update() {
    if (finished()) {
        return false;
    }

    bool changed = false;
    if (pendingDirection_) {
        direction_ = *pendingDirection_;
        pendingDirection_.reset();
        changed = true;
    }

    if (direction_ == Direction::stopped) {
        return changed;
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

    next.x = (next.x + kBoardWidth) % kBoardWidth;
    next.y = (next.y + kBoardHeight) % kBoardHeight;

    const bool eating = next == fruit_;
    const Point tail = snake_.back();
    const bool movingIntoVacatedTail = !eating && next == tail;
    if (occupied_[cellIndex(next)] && !movingIntoVacatedTail) {
        gameOver_ = true;
        return true;
    }

    if (!eating) {
        occupied_[cellIndex(tail)] = false;
        snake_.pop_back();
    }

    snake_.push_front(next);
    occupied_[cellIndex(next)] = true;

    if (eating) {
        score_ += 10;
        spawnFruit();
    }
    return true;
}

bool Game::finished() const {
    return gameOver_ || won_ || quit_;
}

bool Game::gameOver() const {
    return gameOver_;
}

bool Game::won() const {
    return won_;
}

bool Game::quitRequested() const {
    return quit_;
}

int Game::score() const {
    return score_;
}

Direction Game::direction() const {
    return direction_;
}

Point Game::fruit() const {
    return fruit_;
}

const std::deque<Point>& Game::snake() const {
    return snake_;
}

std::size_t Game::cellIndex(Point point) {
    return static_cast<std::size_t>(point.y * kBoardWidth + point.x);
}

bool Game::areOpposite(Direction first, Direction second) {
    return (first == Direction::left && second == Direction::right) ||
           (first == Direction::right && second == Direction::left) ||
           (first == Direction::up && second == Direction::down) ||
           (first == Direction::down && second == Direction::up);
}

void Game::spawnFruit() {
    std::vector<Point> emptyCells;
    emptyCells.reserve(kBoardArea - snake_.size());

    for (int y = 0; y < kBoardHeight; ++y) {
        for (int x = 0; x < kBoardWidth; ++x) {
            const Point candidate{x, y};
            if (!occupied_[cellIndex(candidate)]) {
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

}  // namespace snake
