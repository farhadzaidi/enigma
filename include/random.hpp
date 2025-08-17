#pragma once

#include <random>
#include <cstdint>
#include <bit>

static std::random_device rd;
static std::mt19937_64 gen(rd());

static std::uniform_int_distribution<uint64_t> uint64_dist(
    0, std::numeric_limits<uint64_t>::max()
);

// Returns a random sparse uin64_t
inline uint64_t random_magic() {
    return uint64_dist(gen) & uint64_dist(gen) & uint64_dist(gen);
}