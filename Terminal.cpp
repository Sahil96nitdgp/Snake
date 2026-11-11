#include "Terminal.hpp"

#include <iostream>

#ifdef _WIN32
#define NOMINMAX
#include <conio.h>
#include <windows.h>
#else
#include <cerrno>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace snake {

struct Terminal::Impl {
    bool configured = false;
    std::string error;

#ifdef _WIN32
    HANDLE input = INVALID_HANDLE_VALUE;
    HANDLE output = INVALID_HANDLE_VALUE;
    DWORD originalInputMode = 0;
    DWORD originalOutputMode = 0;
#else
    termios original{};
#endif
};

Terminal::Terminal() : impl_(std::make_unique<Impl>()) {
#ifdef _WIN32
    impl_->input = GetStdHandle(STD_INPUT_HANDLE);
    impl_->output = GetStdHandle(STD_OUTPUT_HANDLE);
    if (impl_->input == INVALID_HANDLE_VALUE || impl_->output == INVALID_HANDLE_VALUE ||
        !GetConsoleMode(impl_->input, &impl_->originalInputMode) ||
        !GetConsoleMode(impl_->output, &impl_->originalOutputMode)) {
        impl_->error = "Snake must be run in an interactive terminal.";
        return;
    }

    const DWORD inputMode = impl_->originalInputMode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    const DWORD outputMode = impl_->originalOutputMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(impl_->input, inputMode) ||
        !SetConsoleMode(impl_->output, outputMode)) {
        SetConsoleMode(impl_->input, impl_->originalInputMode);
        SetConsoleMode(impl_->output, impl_->originalOutputMode);
        impl_->error = "Could not configure the Windows console.";
        return;
    }
#else
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
        impl_->error = "Snake must be run in an interactive terminal.";
        return;
    }
    if (tcgetattr(STDIN_FILENO, &impl_->original) != 0) {
        impl_->error = "Could not read the terminal configuration.";
        return;
    }

    termios raw = impl_->original;
    raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0) {
        impl_->error = "Could not configure terminal input.";
        return;
    }
#endif

    impl_->configured = true;
    std::cout << "\x1b[2J\x1b[H" << std::flush;
}

Terminal::~Terminal() {
    if (!impl_ || !impl_->configured) {
        return;
    }

#ifdef _WIN32
    SetConsoleMode(impl_->input, impl_->originalInputMode);
    SetConsoleMode(impl_->output, impl_->originalOutputMode);
#else
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &impl_->original);
#endif

    std::cout << "\x1b[0m\n" << std::flush;
}

bool Terminal::ready() const {
    return impl_->configured;
}

const std::string& Terminal::error() const {
    return impl_->error;
}

InputStatus Terminal::readKey(char& key) const {
#ifdef _WIN32
    if (!_kbhit()) {
        return InputStatus::none;
    }
    key = static_cast<char>(_getch());
    return InputStatus::key;
#else
    fd_set input;
    FD_ZERO(&input);
    FD_SET(STDIN_FILENO, &input);
    timeval timeout{0, 0};

    const int result = select(STDIN_FILENO + 1, &input, nullptr, nullptr, &timeout);
    if (result == 0 || (result < 0 && errno == EINTR)) {
        return InputStatus::none;
    }
    if (result < 0) {
        return InputStatus::closed;
    }

    const ssize_t bytesRead = read(STDIN_FILENO, &key, 1);
    if (bytesRead == 1) {
        return InputStatus::key;
    }
    if (bytesRead < 0 && (errno == EAGAIN || errno == EINTR)) {
        return InputStatus::none;
    }
    return InputStatus::closed;
#endif
}

std::pair<int, int> Terminal::size() const {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (!GetConsoleScreenBufferInfo(impl_->output, &info)) {
        return {0, 0};
    }
    return {
        info.srWindow.Right - info.srWindow.Left + 1,
        info.srWindow.Bottom - info.srWindow.Top + 1
    };
#else
    winsize dimensions{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &dimensions) != 0) {
        return {0, 0};
    }
    return {dimensions.ws_col, dimensions.ws_row};
#endif
}

bool Terminal::fits(int minimumColumns, int minimumRows) const {
    const auto [columns, rows] = size();
    return columns >= minimumColumns && rows >= minimumRows;
}

void Terminal::draw(const std::string& contents) const {
    std::cout << "\x1b[H\x1b[J" << contents << std::flush;
}

}  // namespace snake
