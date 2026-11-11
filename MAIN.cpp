#include "Game.hpp"
#include "Renderer.hpp"
#include "Terminal.hpp"

#include <chrono>
#include <csignal>
#include <cctype>
#include <exception>
#include <iostream>
#include <thread>

namespace {

constexpr auto kFrameTime = std::chrono::milliseconds(100);
constexpr int kRequiredColumns = snake::kBoardWidth + 2;
constexpr int kRequiredRows = snake::kBoardHeight + 4;

volatile std::sig_atomic_t interrupted = 0;

void handleSignal(int) {
    interrupted = 1;
}

bool handleKey(snake::Game& game, char key) {
    switch (std::tolower(static_cast<unsigned char>(key))) {
        case 'a':
            return game.queueDirection(snake::Direction::left);
        case 'd':
            return game.queueDirection(snake::Direction::right);
        case 'w':
            return game.queueDirection(snake::Direction::up);
        case 's':
            return game.queueDirection(snake::Direction::down);
        case 'q':
        case 'x':
            return game.requestQuit();
        default:
            return false;
    }
}

int run() {
    snake::Terminal terminal;
    if (!terminal.ready()) {
        std::cerr << terminal.error() << '\n';
        return 1;
    }

    snake::Game game;
    bool dirty = true;
    bool previouslyFit = false;

    while (!game.finished() && !interrupted) {
        const auto frameStarted = std::chrono::steady_clock::now();

        char key = '\0';
        while (true) {
            const snake::InputStatus input = terminal.readKey(key);
            if (input == snake::InputStatus::none) {
                break;
            }
            if (input == snake::InputStatus::closed) {
                game.requestQuit();
                break;
            }
            dirty = handleKey(game, key) || dirty;
        }

        const bool fits = terminal.fits(kRequiredColumns, kRequiredRows);
        if (fits != previouslyFit) {
            dirty = true;
            previouslyFit = fits;
        }

        if (fits) {
            dirty = game.update() || dirty;
            if (dirty) {
                terminal.draw(snake::render(game));
                dirty = false;
            }
        } else if (dirty) {
            const auto [columns, rows] = terminal.size();
            terminal.draw(snake::renderTerminalSizeWarning(columns, rows));
            dirty = false;
        }

        std::this_thread::sleep_until(frameStarted + kFrameTime);
    }

    return 0;
}

}  // namespace

int main() {
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    try {
        return run();
    } catch (const std::exception& error) {
        std::cerr << "Snake stopped: " << error.what() << '\n';
    } catch (...) {
        std::cerr << "Snake stopped because of an unexpected error.\n";
    }
    return 1;
}
