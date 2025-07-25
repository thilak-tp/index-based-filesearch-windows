#include "benchmark.h"
void LatencyBenchmark::start() {
    startTime = Clock::now();
}

void LatencyBenchmark::stop() {
    TimePoint endTime = Clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();
    latencies.push_back(duration);
}

void LatencyBenchmark::reset() {
    latencies.clear();
}

void LatencyBenchmark::report(const std::string& label) const {
    if (latencies.empty()) {
        std::cout << label << ": No data collected.\n";
        return;
    }

    long long minLatency = std::numeric_limits<long long>::max();
    long long maxLatency = std::numeric_limits<long long>::min();
    long long total = 0;

    for (auto latency : latencies) {
        if (latency < minLatency) minLatency = latency;
        if (latency > maxLatency) maxLatency = latency;
        total += latency;
    }

    double average = static_cast<double>(total) / latencies.size();

    std::cout << label << " Report (ns):\n";
    std::cout << "  Runs     : " << latencies.size() << "\n";
    std::cout << "  Min      : " << minLatency << "\n";
    std::cout << "  Max      : " << maxLatency << "\n";
    std::cout << "  Average  : " << average << "\n";
}


