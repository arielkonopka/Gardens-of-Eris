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


#ifndef CRASHLOG_H
#define CRASHLOG_H

#include <string>
#include <string_view>
#include <vector>

/**
 * A crash report for players who have no debugger at hand.
 *
 * install() catches crashes (bad memory access, abort, std::terminate, and on Windows any
 * unhandled exception) and writes crash-<date>-<time>.log into the save folder: what went wrong,
 * the world seed, the game tick, a stack trace, and the last lines the game printed.
 * The game then ends the way it would have without the handler.
 */
namespace goe::crashLog {
/// installs the handlers and starts keeping the last printed lines; call once, early in main
void install(const std::string &folder);
/// where reports go; the save folder, followed when the player changes it
void setFolder(const std::string &folder);

/// a fact shown at the top of every report, such as the world seed; set again to change it
void setDetail(const std::string &name, const std::string &value);
/// keeps one line for the next report (std::cout and std::cerr lines are kept on their own)
void note(std::string_view line);
/// the kept lines, oldest first
std::vector<std::string> lastLines();

/// the report text for a crash described by reason
std::string report(std::string_view reason);
/// writes the report into the folder; returns the file's path, empty when it could not be written
std::string writeReport(std::string_view reason);
} // namespace goe::crashLog

#endif // CRASHLOG_H
