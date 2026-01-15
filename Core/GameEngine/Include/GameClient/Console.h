#pragma once
#include <string>
#include <vector>

#include "imgui.h"

class Console
{
public:
	Console();
	~Console() = default;

	void Draw(float openFraction = 0.5f);

	// Manually execute a console command (same logic used when the user presses Enter).
	void ExecCommand(const char* command_line);

	// Add a log entry (printf-style).
	void AddLog(const char* fmt, ...) IM_FMTARGS(2);

	// Clear the entire log buffer.
	void ClearLog();

private:
	static int  TextEditCallbackStub(ImGuiInputTextCallbackData* data);
	int         TextEditCallback(ImGuiInputTextCallbackData* data);

private:
	char                    InputBuf[256]{};
	std::vector<std::string> Items;
	bool                    ScrollToBottom;
	std::vector<std::string> History;
	int                     HistoryPos; // For navigating command history via up/down keys
};

extern Console DevConsole;
