/*
 * Copyright (c) 2026, Ariel Konopka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include "crashLog.h"
#include "gameClock.h"
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <deque>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <streambuf>
#include <thread>
#include <vector>
#include <version>
#ifdef __cpp_lib_stacktrace
#include <stacktrace>
#endif
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace goe::crashLog {
namespace {
constexpr std::size_t keptLines = 55;

std::mutex stateMutex; // guards everything below
std::deque<std::string> lines;
std::map<std::string, std::string> details;
std::string folder = ".";

std::atomic<bool> crashing{false};

/// passes everything on to the stream it replaced, and keeps each finished line for the report
class teeBuffer : public std::streambuf
{
public:
    explicit teeBuffer(std::streambuf *original)
        : original(original)
    {}

protected:
    int overflow(int c) override
    {
        if (c == traits_type::eof())
            return traits_type::not_eof(c);
        const char ch = (char) c;
        this->xsputn(&ch, 1);
        return c;
    }

    std::streamsize xsputn(const char *s, std::streamsize n) override
    {
        std::lock_guard<std::mutex> lock(this->m);
        const auto written = this->original->sputn(s, n);
        for (std::streamsize c = 0; c < n; c++) {
            if (s[c] == '\n') {
                note(this->pending);
                this->pending.clear();
            } else if (this->pending.size() < 400) {
                this->pending += s[c];
            }
        }
        return written;
    }

    int sync() override { return this->original->pubsync(); }

private:
    std::streambuf *original;
    std::mutex m; // the game prints from more than one thread
    std::string pending;
};

std::string signalName(int sig)
{
    switch (sig) {
    case SIGSEGV:
        return "bad memory access (SIGSEGV)";
    case SIGFPE:
        return "arithmetic error (SIGFPE)";
    case SIGILL:
        return "illegal instruction (SIGILL)";
    case SIGABRT:
        return "abort (SIGABRT)";
#ifdef SIGBUS
    case SIGBUS:
        return "bus error (SIGBUS)";
#endif
    default:
        return "signal " + std::to_string(sig);
    }
}

/// writes the report once, whatever reports the crash first
void reportOnce(std::string_view reason)
{
    if (crashing.exchange(true))
        return;
    const auto path = writeReport(reason);
    if (!path.empty())
        std::cerr << "\nThe game crashed. A report was written to " << path << "\n";
#ifdef _WIN32
    // the release build has no console to read, so say it in a box
    const std::string text = path.empty() ? "Gardens of Eris crashed."
                                          : "Gardens of Eris crashed.\nA report was saved to:\n" + path
                                                + "\n\nPlease send it along with what you were doing.";
    MessageBoxA(nullptr, text.c_str(), "Gardens of Eris", MB_OK | MB_ICONERROR);
#endif
}

void onSignal(int sig)
{
    reportOnce(signalName(sig));
    // the handler was installed to run once; the default action ends the game as before
    std::signal(sig, SIG_DFL);
    std::raise(sig);
}

void onTerminate()
{
    std::string reason = "std::terminate";
    if (auto e = std::current_exception()) {
        try {
            std::rethrow_exception(e);
        } catch (const std::exception &ex) {
            reason = std::string("uncaught exception: ") + ex.what();
        } catch (...) {
            reason = "uncaught exception of unknown type";
        }
    }
    reportOnce(reason);
    std::abort();
}

#ifdef _WIN32
LONG WINAPI onWindowsException(EXCEPTION_POINTERS *info)
{
    const auto code = info->ExceptionRecord->ExceptionCode;
    std::string what = std::format("Windows exception 0x{:08X}", (unsigned long) code);
    if (code == EXCEPTION_ACCESS_VIOLATION)
        what += " (bad memory access)";
    else if (code == EXCEPTION_STACK_OVERFLOW)
        what += " (stack overflow)";
    reportOnce(what);
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif

void installHandlers()
{
    std::set_terminate(onTerminate);
#ifdef _WIN32
    SetUnhandledExceptionFilter(onWindowsException);
    // leaves room on this thread's stack for the report after a stack overflow
    ULONG guarantee = 64 * 1024;
    SetThreadStackGuarantee(&guarantee);
    std::signal(SIGABRT, onSignal);
#else
    // a separate stack, so a stack overflow can still be reported
    static std::vector<char> altStack(256 * 1024);
    stack_t ss{};
    ss.ss_sp = altStack.data();
    ss.ss_size = altStack.size();
    sigaltstack(&ss, nullptr);
    struct sigaction sa{};
    sa.sa_handler = onSignal;
    sa.sa_flags = SA_ONSTACK | SA_RESETHAND;
    sigemptyset(&sa.sa_mask);
    for (int sig : {SIGSEGV, SIGBUS, SIGFPE, SIGILL, SIGABRT})
        sigaction(sig, &sa, nullptr);
#endif
}
} // namespace

void install(const std::string &dir)
{
    setFolder(dir);
    static std::once_flag once;
    std::call_once(once, []() {
        static teeBuffer out(std::cout.rdbuf());
        static teeBuffer err(std::cerr.rdbuf());
        std::cout.rdbuf(&out);
        std::cerr.rdbuf(&err);
        installHandlers();
    });
}

void setFolder(const std::string &dir)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    folder = dir.empty() ? "." : dir;
}

void setDetail(const std::string &name, const std::string &value)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    details[name] = value;
}

void note(std::string_view line)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    lines.emplace_back(line);
    if (lines.size() > keptLines)
        lines.pop_front();
}

std::vector<std::string> lastLines()
{
    std::lock_guard<std::mutex> lock(stateMutex);
    return {lines.begin(), lines.end()};
}

std::string report(std::string_view reason)
{
    std::ostringstream r;
    r << "Gardens of Eris crash report\n";
    r << "When: " << std::format("{:%Y-%m-%d %H:%M:%S} UTC", std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now())) << "\n";
    r << "What: " << reason << "\n";
    r << "Game tick: " << gameClock::now() << "\n";
    r << "Thread: " << std::this_thread::get_id() << "\n";
    // a crash may come while another thread holds the lock; the report goes out without the rest then
    std::unique_lock<std::mutex> lock(stateMutex, std::try_to_lock);
    if (lock.owns_lock())
        for (const auto &[name, value] : details)
            r << name << ": " << value << "\n";
    r << "\nStack trace:\n";
#ifdef __cpp_lib_stacktrace
    r << std::stacktrace::current(1) << "\n";
#else
    r << "(not available in this build)\n";
#endif
    r << "\nLast lines the game printed:\n";
    if (lock.owns_lock())
        for (const auto &line : lines)
            r << line << "\n";
    return r.str();
}

std::string writeReport(std::string_view reason)
{
    std::string dir;
    {
        std::unique_lock<std::mutex> lock(stateMutex, std::try_to_lock);
        dir = lock.owns_lock() ? folder : ".";
    }
    const auto text = report(reason);
    const auto name = std::format("crash-{:%Y%m%d-%H%M%S}.log", std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()));
    std::error_code ec;
    for (const auto &where : {std::filesystem::path(dir), std::filesystem::path(".")}) {
        const auto path = where / name;
        std::ofstream f(path, std::ios::binary);
        if (f << text && f.flush())
            return std::filesystem::absolute(path, ec).string();
    }
    return {};
}
} // namespace goe::crashLog
