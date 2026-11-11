#pragma once

#include <memory>
#include <string>
#include <utility>

namespace snake {

enum class InputStatus {
    none,
    key,
    closed
};

class Terminal {
public:
    Terminal();
    ~Terminal();

    Terminal(const Terminal&) = delete;
    Terminal& operator=(const Terminal&) = delete;

    bool ready() const;
    const std::string& error() const;
    InputStatus readKey(char& key) const;
    std::pair<int, int> size() const;
    bool fits(int minimumColumns, int minimumRows) const;
    void draw(const std::string& contents) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace snake
