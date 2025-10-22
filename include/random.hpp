#pragma once

#include <random>
#include <cstdint>
#include <bit>

static std::random_device rd;
static std::mt19937_64 gen(rd());

static std::uniform_int_distribution<uint64_t> u64_dist(
    0, std::numeric_limits<uint64_t>::max()
);

inline uint64_t random_u64() {
    return u64_dist(gen);
}

// Returns a random sparse uint64_t
inline uint64_t random_magic() {
    return u64_dist(gen) & u64_dist(gen) & u64_dist(gen);
}