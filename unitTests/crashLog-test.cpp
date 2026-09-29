/*
 * The crash report: what it holds and where it goes. The handlers themselves only run when the
 * game really crashes, so these tests check the report they write.
 */
#include "crashLog.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace {
std::string readFile(const std::string &path)
{
    std::ifstream f(path, std::ios::binary);
    std::stringstream s;
    s << f.rdbuf();
    return s.str();
}
} // namespace

TEST(CrashLogTests, ReportTellsWhatHappenedAndWhatWasPrinted)
{
    goe::crashLog::install(".");
    goe::crashLog::setDetail("World seed", "23");
    std::cout << "a line the game printed" << std::endl;
    goe::crashLog::note("a noted line");

    const auto text = goe::crashLog::report("bad memory access (SIGSEGV)");
    EXPECT_NE(text.find("What: bad memory access (SIGSEGV)"), std::string::npos);
    EXPECT_NE(text.find("World seed: 23"), std::string::npos);
    EXPECT_NE(text.find("Game tick:"), std::string::npos);
    EXPECT_NE(text.find("Stack trace:"), std::string::npos);
    EXPECT_NE(text.find("a line the game printed"), std::string::npos);
    EXPECT_NE(text.find("a noted line"), std::string::npos);
}

TEST(CrashLogTests, KeepsOnlyTheLastLines)
{
    for (int c = 0; c < 500; c++)
        goe::crashLog::note("line " + std::to_string(c));
    const auto lines = goe::crashLog::lastLines();
    ASSERT_FALSE(lines.empty());
    EXPECT_LT(lines.size(), 100u);
    EXPECT_EQ(lines.back(), "line 499");
}

TEST(CrashLogTests, ReportGoesIntoTheSaveFolder)
{
    const auto dir = std::filesystem::temp_directory_path() / "goe-crashlog-test";
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    goe::crashLog::setFolder(dir.string());

    const auto path = goe::crashLog::writeReport("test crash");
    ASSERT_FALSE(path.empty());
    EXPECT_EQ(std::filesystem::path(path).parent_path(), std::filesystem::absolute(dir));
    EXPECT_EQ(std::filesystem::path(path).filename().string().rfind("crash-", 0), 0u);
    EXPECT_NE(readFile(path).find("What: test crash"), std::string::npos);

    goe::crashLog::setFolder(".");
    std::filesystem::remove_all(dir);
}
