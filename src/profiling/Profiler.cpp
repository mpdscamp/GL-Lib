#include "Profiler.hpp"
#include "gl/logger.hpp"
#include <GLFW/glfw3.h>
#include <sstream>

Profiler::Profiler()
    : frameStartTime_(0.0)
{
    // Initialize any needed profiling state
}

Profiler::~Profiler()
{
    // Cleanup if needed
}

void Profiler::beginFrame()
{
    frameStartTime_ = glfwGetTime();
}

void Profiler::endFrame()
{
    double frameTime = glfwGetTime() - frameStartTime_;
    frameTimes_.push_back(frameTime);
    // Keep a rolling average of the last 100 frames
    if (frameTimes_.size() > 100) {
        frameTimes_.pop_front();
    }
}

void Profiler::beginSection(const std::string& name)
{
    sectionStartTimes_[name] = glfwGetTime();
}

void Profiler::endSection(const std::string& name)
{
    auto it = sectionStartTimes_.find(name);
    if (it != sectionStartTimes_.end()) {
        double sectionTime = glfwGetTime() - it->second;
        if (sectionTimes_.find(name) == sectionTimes_.end()) {
            sectionTimes_[name] = std::deque<double>();
        }
        sectionTimes_[name].push_back(sectionTime);
        // Keep a rolling average
        if (sectionTimes_[name].size() > 100) {
            sectionTimes_[name].pop_front();
        }
    }
}

void Profiler::printStats() const
{
    if (frameTimes_.empty()) return;

    // Calculate average frame time and FPS
    double avgFrameTime = 0.0;
    for (double time : frameTimes_) {
        avgFrameTime += time;
    }
    avgFrameTime /= frameTimes_.size();
    double fps = 1.0 / avgFrameTime;

    std::stringstream ss;
    ss << "===== Performance Stats =====\n";
    ss << "Avg Frame Time: " << (avgFrameTime * 1000.0) << " ms\n";
    ss << "FPS: " << fps << "\n";

    // Print section times
    for (const auto& [name, times] : sectionTimes_) {
        if (times.empty()) continue;
        double avgTime = 0.0;
        for (double time : times) {
            avgTime += time;
        }
        avgTime /= times.size();
        ss << name << ": " << (avgTime * 1000.0) << " ms ("
            << (avgTime / avgFrameTime * 100.0) << "% of frame)\n";
    }
    ss << "===========================";

    gl::logInfo(ss.str());
}