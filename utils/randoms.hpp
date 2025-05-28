#pragma once

#include <random>
#include <string>

using u32 = uint_least32_t;
using RandomEngine = std::mt19937;

inline RandomEngine& getEngine() {
    static thread_local RandomEngine engine(std::random_device{}());
    return engine;
}

inline u32 randomInt(int min, int max) {
    std::uniform_int_distribution<u32> distribution(min, max);
    return distribution(getEngine());
}

inline std::string randomString(std::size_t length = 32) {
    static const std::string allowedChars = "123456789ABCDFGHJKLMNPQRSTVWXZabcdfghjklmnpqrstvwxz";
    static thread_local std::uniform_int_distribution<std::size_t> dist(0, allowedChars.size() - 1);

    std::string result(length, '\0');
    for (char& c : result) {
        c = allowedChars[dist(getEngine())];
    }

    return result;
}
