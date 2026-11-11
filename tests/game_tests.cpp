#include "Game.hpp"
#include "Renderer.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

snake::Direction opposite(snake::Direction direction) {
    switch (direction) {
        case snake::Direction::left:
            return snake::Direction::right;
        case snake::Direction::right:
            return snake::Direction::left;
        case snake::Direction::up:
            return snake::Direction::down;
        case snake::Direction::down:
            return snake::Direction::up;
        case snake::Direction::stopped:
            return snake::Direction::stopped;
    }
    return snake::Direction::stopped;
}

void moveTowardFruit(snake::Game& game) {
    while (game.score() == 0 && game.snake().front().x != game.fruit().x) {
        const snake::Direction horizontal =
            game.snake().front().x < game.fruit().x
                ? snake::Direction::right
                : snake::Direction::left;
        game.queueDirection(horizontal);
        game.update();
    }

    while (game.score() == 0 && game.snake().front().y != game.fruit().y) {
        const snake::Direction vertical =
            game.snake().front().y < game.fruit().y
                ? snake::Direction::down
                : snake::Direction::up;
        game.queueDirection(vertical);
        game.update();
    }
}

void testInitialState() {
    snake::Game game(1);
    require(game.score() == 0, "score should start at zero");
    require(game.snake().size() == 1, "snake should start with one segment");
    require(game.fruit() != game.snake().front(), "fruit must not overlap the snake");
    require(!game.finished(), "new game should be active");
}

void testOnlyOneTurnPerTick() {
    snake::Game game(2);
    const snake::Point start = game.snake().front();
    require(game.queueDirection(snake::Direction::right), "first turn should be queued");
    require(!game.queueDirection(snake::Direction::up), "second turn in one tick should be ignored");
    game.update();
    require(game.direction() == snake::Direction::right, "first queued turn should win");
    require(game.snake().front() == snake::Point{start.x + 1, start.y},
            "snake should move in the first queued direction");
}

void testEdgeWrapping() {
    snake::Game game(3);
    game.queueDirection(snake::Direction::left);
    for (int step = 0; step <= snake::kBoardWidth / 2; ++step) {
        game.update();
    }
    require(game.snake().front().x == snake::kBoardWidth - 1,
            "left edge should wrap to the right edge");
}

void testEatingAndReverseProtection() {
    snake::Game game(4);
    moveTowardFruit(game);
    require(game.score() == 10, "eating fruit should add ten points");
    require(game.snake().size() == 2, "eating fruit should grow the snake");

    const snake::Direction current = game.direction();
    require(!game.queueDirection(opposite(current)),
            "a direct reversal should be rejected after growth");
    game.update();
    require(game.direction() == current, "rejected reversal must not change direction");
    require(!game.gameOver(), "continuing forward after a rejected reversal should be safe");
}

void testRendering() {
    snake::Game game(5);
    const std::string frame = snake::render(game);
    require(frame.find('O') != std::string::npos, "render should contain the head");
    require(frame.find('@') != std::string::npos, "render should contain the fruit");
    require(frame.find("Score: 0") != std::string::npos, "render should contain the score");
}

}  // namespace

int main() {
    testInitialState();
    testOnlyOneTurnPerTick();
    testEdgeWrapping();
    testEatingAndReverseProtection();
    testRendering();
    std::cout << "All game tests passed.\n";
    return 0;
}
