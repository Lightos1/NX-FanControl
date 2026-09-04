#pragma once

#include <utility>

template <typename F>
struct ScopeExit {
    F f;
    bool active = true;
    explicit ScopeExit(F&& f) : f(std::forward<F>(f)) {}
    ~ScopeExit() { if (active) f(); }
    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;
};

struct ScopeExitHelper {
    template <typename F>
    ScopeExit<F> operator+(F&& f) { return ScopeExit<F>(std::forward<F>(f)); }
};

#define CONCAT_(a, b) a##b
#define CONCAT(a, b) CONCAT_(a, b)
#define ON_SCOPE_EXIT auto CONCAT(scopeExit_, __LINE__) = ScopeExitHelper{} + [&]()
