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

    std::this_thread::sleep_for(std::chrono::seconds(5)); // collect several frames for trace
    running = false;
    movie.Stop();

    updateThread.join();
    renderThread.join();

    ui::TraceProfiler::Instance().EndSession();
    return 0;
}