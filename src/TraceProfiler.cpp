#include "TraceProfiler.h"

#include <fstream>
#include <thread>

namespace ui {

TraceProfiler& TraceProfiler::Instance() {
    static TraceProfiler instance;
    return instance;
}

void TraceProfiler::BeginSession(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    events_.clear();
    filePath_ = path;
    sessionStart_ = std::chrono::steady_clock::now();
    sessionOpen_ = true;
}

void TraceProfiler::EndSession() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!sessionOpen_) {
        return;
    }
    DumpToFile();
    sessionOpen_ = false;
}

void TraceProfiler::RecordEvent(const char* name,
                                std::uint64_t startUs,
                                std::uint64_t durUs,
                                std::uint64_t tid) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!sessionOpen_) {
        return;
    }
    events_.push_back(TraceEvent{std::string(name), startUs, durUs, tid});
}

std::uint64_t TraceProfiler::NowSinceStartUs() const {
    const auto now = std::chrono::steady_clock::now();
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(now - sessionStart_).count());
}

std::uint64_t TraceProfiler::RegisterThread(const char* name) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto id = std::this_thread::get_id();
    auto it = threadMap_.find(id);
    if (it != threadMap_.end()) {
        return it->second;
    }
    const std::uint64_t tid = nextTid_++;
    threadMap_[id] = tid;
    if (sessionOpen_ && name) {
        EmitThreadNameMetadata(tid, name);
    }
    return tid;
}

std::uint64_t TraceProfiler::CurrentThreadId() {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto id = std::this_thread::get_id();
    auto it = threadMap_.find(id);
    if (it != threadMap_.end()) {
        return it->second;
    }
    const std::uint64_t tid = nextTid_++;
    threadMap_[id] = tid;
    return tid;
}

void TraceProfiler::EmitThreadNameMetadata(std::uint64_t tid, const std::string& name) {
    events_.push_back(TraceEvent{
        "thread_name",
        0,
        0,
        tid,
        true,
        name,
    });
}

void TraceProfiler::DumpToFile() {
    std::ofstream out(filePath_, std::ios::trunc);
    if (!out.is_open()) {
        return;
    }

    out << "{ \"traceEvents\": [";
    bool first = true;
    for (const auto& e : events_) {
        if (!first) {
            out << ",";
        }
        first = false;
        out << "{";
        out << "\"name\":\"" << e.name << "\",";
        out << "\"cat\":\"trace\",";
        if (e.isMetadata) {
            out << "\"ph\":\"M\",";
            out << "\"ts\":0,";
            out << "\"pid\":0,";
            out << "\"tid\":" << e.tid << ",";
            out << "\"args\":{\"name\":\"" << e.threadName << "\"}";
        } else {
            out << "\"ph\":\"X\",";
            out << "\"ts\":" << e.tsMicro << ",";
            out << "\"dur\":" << e.durMicro << ",";
            out << "\"pid\":0,";
            out << "\"tid\":" << e.tid;
        }
        out << "}";
    }
    out << "] }";
}

TraceScope::TraceScope(const char* name)
    : name_(name)
    , begin_(std::chrono::steady_clock::now()) {
}

TraceScope::~TraceScope() {
    const auto end = std::chrono::steady_clock::now();
    const auto endUs = TraceProfiler::Instance().NowSinceStartUs();
    const auto durUs =
        std::chrono::duration_cast<std::chrono::microseconds>(end - begin_).count();
    const auto startUs = (endUs > static_cast<std::uint64_t>(durUs))
                             ? endUs - static_cast<std::uint64_t>(durUs)
                             : 0;
    const auto tid = TraceProfiler::Instance().CurrentThreadId();
    TraceProfiler::Instance().RecordEvent(
        name_, startUs, static_cast<std::uint64_t>(durUs), tid);
}

} // namespace ui

