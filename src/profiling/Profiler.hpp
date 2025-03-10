#ifndef PROFILER_HPP
#define PROFILER_HPP

#include <string>
#include <unordered_map>
#include <deque>

class Profiler {
public:
    Profiler();
    ~Profiler();

    // Frame timing methods
    void beginFrame();
    void endFrame();

    // Section timing methods
    void beginSection(const std::string& name);
    void endSection(const std::string& name);

    // Reporting
    void printStats() const;

private:
    double frameStartTime_;
    std::deque<double> frameTimes_;
    std::unordered_map<std::string, double> sectionStartTimes_;
    std::unordered_map<std::string, std::deque<double>> sectionTimes_;
};

#endif // PROFILER_HPP