#include "Movie.h"
#include "TraceProfiler.h"

#include <atomic>
#include <chrono>
#include <thread>

int main() {
    ui::TraceProfiler::Instance().BeginSession("trace.json");
    ui::TraceProfiler::Instance().RegisterThread("main");

    ui::Movie movie;
    std::atomic<bool> running{true};

    // Update thread
    std::thread updateThread([&]() {
        ui::TraceProfiler::Instance().RegisterThread("update");
        while (running && movie.IsRunning()) {
            movie.Update();
            std::this_thread::sleep_for(std::chrono::seconds(1)); // update: 1 Hz
        }
    });

    // Render thread
    std::thread renderThread([&]() {
        ui::TraceProfiler::Instance().RegisterThread("render");
        while (running && movie.IsRunning()) {
            movie.Render();
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // render: 2 Hz
        }
    });

    // Main thread: process window events (required on macOS)
    auto startTime = std::chrono::steady_clock::now();
    while (running && movie.IsRunning()) {
        movie.ProcessEvents();
        
        // Check if 5 seconds have passed
        auto elapsed = std::chrono::steady_clock::now() - startTime;
        if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() >= 5) {
            running = false;
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 Hz for event processing
    }

    movie.Stop();

    updateThread.join();
    renderThread.join();

    ui::TraceProfiler::Instance().EndSession();
    return 0;
}