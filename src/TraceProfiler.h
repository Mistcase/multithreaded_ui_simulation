#pragma once

#include "ui_ids.h"

#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace ui {

struct TraceEvent {
    std::string name;
    std::uint64_t tsMicro = 0;   // begin timestamp (us, relative to session start)
    std::uint64_t durMicro = 0;  // duration in microseconds
    std::uint64_t tid = 0;
    bool isMetadata = false;
    std::string threadName; // only for metadata thread_name
};

class TraceProfiler {
public:
    static TraceProfiler& Instance();

    void BeginSession(const std::string& path = "trace.json");
    void EndSession();

    void RecordEvent(const char* name,
                     std::uint64_t startUs,
                     std::uint64_t durUs,
                     std::uint64_t tid);

    std::uint64_t NowSinceStartUs() const;

    // Register thread and emit thread_name metadata; returns internal tid
    std::uint64_t RegisterThread(const char* name);
    // Get current thread id (register if needed, without renaming)
    std::uint64_t CurrentThreadId();

private:
    TraceProfiler() = default;

    void DumpToFile();
    void EmitThreadNameMetadata(std::uint64_t tid, const std::string& name);

    std::chrono::steady_clock::time_point m_sessionStart{};
    std::string m_filePath;
    bool m_sessionOpen = false;

    std::vector<TraceEvent> m_events;
    mutable std::mutex m_mutex;

    std::unordered_map<std::thread::id, std::uint64_t> m_threadMap;
    std::uint64_t m_nextTid = 0;
};

class TraceScope {
public:
    explicit TraceScope(const char* name);
    ~TraceScope();

private:
    const char* m_name;
    std::chrono::steady_clock::time_point m_begin;
};

#define TRACE_SCOPE(name_literal) ::ui::TraceScope trace_scope_guard_##__LINE__{name_literal}

} // namespace ui

