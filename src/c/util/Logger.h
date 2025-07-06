#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <set>
#include <streambuf>
#include <iostream>
#include <memory>
#include <sstream>
#include <mutex>
#include <ctime>
#include <iomanip>

class Logger {
public:
    Logger() {
        redirectStreams();
    }

    ~Logger() {
        restoreStreams();
    }

    void render(const char* title = "Log Viewer") {
        std::lock_guard<std::mutex> lock(mutex);

        ImGui::Begin(title);

        // Prefix filter UI
        ImGui::Text("Prefix Filters:");
        ImGui::SameLine();
        if (ImGui::Button(allPrefixesEnabled ? "Hide All" : "Show All")) {
            allPrefixesEnabled = !allPrefixesEnabled;
            activePrefixes.clear();
            if (allPrefixesEnabled) {
                for (const auto& p : prefixes)
                    activePrefixes.insert(p);
            }
        }

        for (const auto& prefix : prefixes) {
            bool selected = activePrefixes.count(prefix);
            if (ImGui::Checkbox(prefix.c_str(), &selected)) {
                if (selected)
                    activePrefixes.insert(prefix);
                else
                    activePrefixes.erase(prefix);
                allPrefixesEnabled = (activePrefixes.size() == prefixes.size());
            }
        }

        // Controls
        ImGui::Checkbox("Auto Scroll", &scrollToBottom);
        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            logLines.clear();
            logIsError.clear();
            logPrefixes.clear();
        }
        ImGui::SameLine();
        if (ImGui::Button("Copy")) {
            std::stringstream all;
            for (const auto& line : logLines)
                all << line << '\n';
            ImGui::SetClipboardText(all.str().c_str());
        }

        ImGui::Separator();
        ImGui::BeginChild("LogRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
        for (size_t i = 0; i < logLines.size(); ++i) {
            if (!allPrefixesEnabled) {
                bool match = false;
                for (const auto& p : logPrefixes[i]) {
                    if (activePrefixes.count(p)) {
                        match = true;
                        break;
                    }
                }
                if (!match)
                    continue;
            }

            const std::string& line = logLines[i];
            ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
            if (line.find("[INFO]") != std::string::npos)
                color = ImVec4(0.6f, 0.9f, 0.6f, 1);
            else if (line.find("[WARN]") != std::string::npos)
                color = ImVec4(1.0f, 0.8f, 0.3f, 1);
            else if (logIsError[i] || line.find("[ERROR]") != std::string::npos)
                color = ImVec4(1, 0.4f, 0.4f, 1);

            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(line.c_str());
            ImGui::PopStyleColor();
        }
        ImGui::PopStyleVar();

        if (scrollToBottom)
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
        ImGui::End();
    }

private:
    std::vector<std::string> logLines;
    std::vector<bool> logIsError;
    std::vector<std::vector<std::string>> logPrefixes;

    std::set<std::string> prefixes;
    std::set<std::string> activePrefixes;
    bool allPrefixesEnabled = true;

    bool scrollToBottom = true;
    std::mutex mutex;

    std::streambuf* originalCoutBuf = nullptr;
    std::streambuf* originalCerrBuf = nullptr;

    class RedirectBuf : public std::streambuf {
    public:
        RedirectBuf(Logger& logger, std::streambuf* realBuf, bool isError)
            : logger(logger), realBuf(realBuf), isError(isError) {}

    protected:
        std::streamsize xsputn(const char* s, std::streamsize count) override {
            realBuf->sputn(s, count);
            buffer.append(s, count);
            flushBuffer();
            return count;
        }

        int overflow(int ch) override {
            realBuf->sputc(ch);
            if (ch != EOF) {
                buffer += static_cast<char>(ch);
                if (ch == '\n') flushBuffer();
            }
            return ch;
        }

    private:
        void flushBuffer() {
            size_t pos;
            while ((pos = buffer.find('\n')) != std::string::npos) {
                std::string line = buffer.substr(0, pos);
                logger.addLogLine(line, isError);
                buffer.erase(0, pos + 1);
            }
        }

        std::string buffer;
        Logger& logger;
        std::streambuf* realBuf;
        bool isError;
    };

    std::unique_ptr<RedirectBuf> coutBuf;
    std::unique_ptr<RedirectBuf> cerrBuf;

    void redirectStreams() {
        originalCoutBuf = std::cout.rdbuf();
        originalCerrBuf = std::cerr.rdbuf();
        coutBuf = std::make_unique<RedirectBuf>(*this, originalCoutBuf, false);
        cerrBuf = std::make_unique<RedirectBuf>(*this, originalCerrBuf, true);
        std::cout.rdbuf(coutBuf.get());
        std::cerr.rdbuf(cerrBuf.get());
    }

    void restoreStreams() {
        std::cout.rdbuf(originalCoutBuf);
        std::cerr.rdbuf(originalCerrBuf);
    }

    void addLogLine(const std::string& line, bool isError) {
        std::lock_guard<std::mutex> lock(mutex);

        std::ostringstream withTimestamp;
        withTimestamp << getCurrentTime() << " " << line;
        std::string finalLine = withTimestamp.str();

        logLines.push_back(finalLine);
        logIsError.push_back(isError);

        auto parsed = extractPrefixes(line);
        logPrefixes.push_back(parsed);

        for (const auto& p : parsed) {
            if (prefixes.insert(p).second)
                activePrefixes.insert(p); // default to visible
        }
    }

    std::vector<std::string> extractPrefixes(const std::string& line) {
        std::vector<std::string> result;
        size_t start = 0;
        while ((start = line.find('[', start)) != std::string::npos) {
            size_t end = line.find(']', start);
            if (end != std::string::npos) {
                result.push_back(line.substr(start, end - start + 1));
                start = end + 1;
            } else break;
        }
        return result;
    }

    std::string getCurrentTime() {
        std::time_t now = std::time(nullptr);
        std::tm local{};
#if defined(_WIN32)
        localtime_s(&local, &now);
#else
        local = *std::localtime(&now);
#endif
        std::ostringstream ss;
        ss << "[" << std::setw(2) << std::setfill('0') << local.tm_hour << ":"
           << std::setw(2) << std::setfill('0') << local.tm_min << ":"
           << std::setw(2) << std::setfill('0') << local.tm_sec << "]";
        return ss.str();
    }
};