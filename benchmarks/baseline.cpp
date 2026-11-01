#include "amr/malloc_resource.hpp"

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;
volatile std::uintptr_t pointer_sink = 0;

struct Result {
    double median_nanoseconds_per_pair;
    double pairs_per_second;
};

std::size_t parse_iterations(int argc, char** argv) {
    constexpr std::size_t default_iterations = 250'000;
    if (argc < 2) {
        return default_iterations;
    }
    if (argv[1][0] == '-') {
        throw std::invalid_argument("iteration count must be positive");
    }

    errno = 0;
    char* end = nullptr;
    const auto parsed = std::strtoull(argv[1], &end, 10);
    if (errno == ERANGE || end == argv[1] || *end != '\0' || parsed == 0 ||
        parsed > std::numeric_limits<std::size_t>::max()) {
        throw std::invalid_argument("usage: amr_baseline [positive-iteration-count]");
    }
    return static_cast<std::size_t>(parsed);
}

template <class Resource>
void run_batch(Resource& resource, std::size_t bytes, std::size_t iterations) {
    std::uintptr_t checksum = 0;
    for (std::size_t i = 0; i < iterations; ++i) {
        void* address = resource.allocate(bytes);
        checksum ^= reinterpret_cast<std::uintptr_t>(address);
        resource.deallocate(address);
    }
    pointer_sink = checksum;
}

template <class Resource>
Result measure(Resource& resource, std::size_t bytes, std::size_t iterations) {
    constexpr std::size_t repetitions = 5;
    run_batch(resource, bytes, std::min<std::size_t>(iterations, 10'000));

    std::vector<double> samples;
    samples.reserve(repetitions);
    for (std::size_t run = 0; run < repetitions; ++run) {
        const auto start = Clock::now();
        run_batch(resource, bytes, iterations);
        const auto elapsed = Clock::now() - start;
        const double nanoseconds =
            std::chrono::duration<double, std::nano>(elapsed).count();
        samples.push_back(nanoseconds / static_cast<double>(iterations));
    }

    std::sort(samples.begin(), samples.end());
    const double median = samples[samples.size() / 2];
    return {median, 1'000'000'000.0 / median};
}

} // namespace

int main(int argc, char** argv) {
    try {
        const std::size_t iterations = parse_iterations(argc, argv);
        constexpr std::array<std::size_t, 4> sizes{32, 128, 512, 4096};
        amr::MallocResource resource;

        std::cout << "System allocator baseline (allocation/free pairs)\n"
                  << "Iterations per sample: " << iterations
                  << "; measured samples: 5; warmup: up to 10000 pairs\n\n"
                  << std::left << std::setw(12) << "bytes"
                  << std::right << std::setw(22) << "median ns/pair"
                  << std::setw(20) << "pairs/second" << '\n';

        for (const auto bytes : sizes) {
            const Result result = measure(resource, bytes, iterations);
            std::cout << std::left << std::setw(12) << bytes
                      << std::right << std::setw(22) << std::fixed
                      << std::setprecision(2) << result.median_nanoseconds_per_pair
                      << std::setw(20) << std::setprecision(0)
                      << result.pairs_per_second << '\n';
        }
    } catch (const std::exception& error) {
        std::cerr << "amr_baseline: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
