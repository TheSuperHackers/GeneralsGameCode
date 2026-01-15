#include "GameClient/Console.h"
Console DevConsole;

// Set this to your app�s version string:
static const char* VERSION_STRING = "Command and Conquer Generals Next-Gen v0.06";

Console::Console()
	: ScrollToBottom(false)
	, HistoryPos(-1)
{
	ClearLog();
	AddLog("Welcome to the Quake-Style Console!");
}

void Console::Draw(float openFraction)
{
	if (openFraction <= 0.0f)
		return;

	if (openFraction > 1.0f)
		openFraction = 1.0f;

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 screenSize = io.DisplaySize;

	float consoleHeight = screenSize.y * openFraction;
	ImVec2 consolePos = ImVec2(0, 0);
	ImVec2 consoleSize = ImVec2(screenSize.x, consoleHeight);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.05f, 0.8f));

	ImGui::SetNextWindowPos(consolePos);
	ImGui::SetNextWindowSize(consoleSize);
	ImGui::Begin("QuakeConsoleOverlay",
		nullptr,
		ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoSavedSettings
		| ImGuiWindowFlags_NoBringToFrontOnFocus);

	//ImGui::PushFont(g_BigConsoleFont);

	float footerHeight = ImGui::GetTextLineHeight() + 8; // space for input + some padding
	ImGui::BeginChild("ConsoleScrollingRegion", ImVec2(0, -footerHeight), false);

	// Draw existing console lines
	for (int i = 0; i < (int)Items.size(); i++)
	{
		const char* item = Items[i].c_str();

		if (strncmp(item, "ASSERTION FAILURE:", 18) == 0)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.f));
			ImGui::TextUnformatted(item);
			ImGui::PopStyleColor();
		}
		else if (strstr(item, "[error]"))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.4f, 0.4f, 1.f));
			ImGui::TextUnformatted(item);
			ImGui::PopStyleColor();
		}
		else if (strncmp(item, "# ", 2) == 0)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.8f, 0.6f, 1.f));
			ImGui::TextUnformatted(item);
			ImGui::PopStyleColor();
		}
		else
		{
			ImGui::TextUnformatted(item);
		}
	}

	if (ScrollToBottom)
		ImGui::SetScrollHereY(1.0f);
	ScrollToBottom = false;

	ImGui::EndChild();

	{
		// Make an (almost) invisible InputText that still captures user input:
		ImGui::PushItemWidth(-1);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.0f); // fully invisible
		// Focus on this widget so it captures keyboard input:
		if (ImGui::IsWindowAppearing())
			ImGui::SetKeyboardFocusHere();

		// Use same callback flags for history/tab-completion as before:
		if (ImGui::InputText("##HiddenConsoleInput", InputBuf, IM_ARRAYSIZE(InputBuf),
			ImGuiInputTextFlags_EnterReturnsTrue
			| ImGuiInputTextFlags_CallbackHistory
			| ImGuiInputTextFlags_CallbackCompletion,
			&TextEditCallbackStub, (void*)this))
		{
			// If user pressed Enter:
			char* s = InputBuf;
			if (s[0] != '\0')
				ExecCommand(s);
			s[0] = '\0';
		}
		ImGui::PopStyleVar(); // restore alpha
		ImGui::PopItemWidth();

		// Now manually render the input line on the far left:
		// e.g. a Quake-like prompt: ">" or "#" etc.
		ImGui::TextUnformatted("> ");
		ImGui::SameLine();
		ImGui::TextUnformatted(InputBuf);

		// Then place the version text on the far right:
		// We can compute how much space is left and push the text to the right:
		float versionTextWidth = ImGui::CalcTextSize(VERSION_STRING).x;
		float availableWidth = ImGui::GetContentRegionAvail().x;
		ImGui::SameLine(std::max(0.0f, availableWidth - versionTextWidth));
		ImGui::TextUnformatted(VERSION_STRING);
	}

	//ImGui::PopFont();
	ImGui::End(); // end main window

	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	// ------------------------------------------------------------
    // Draw the small orange line where the console ends
    // ------------------------------------------------------------
	ImDrawList* fg = ImGui::GetForegroundDrawList();
	// Coordinates for a line across the entire screen at y = consoleHeight
	ImVec2 start = ImVec2(0.0f, consoleHeight);
	ImVec2 end = ImVec2(screenSize.x, consoleHeight);

	// Use a bright orange color; thickness = 2.0f
	fg->AddLine(start, end, IM_COL32(255, 128, 0, 255), 2.0f);
}

// -----------------------------------------------------------------------------
// ExecCommand, AddLog, ClearLog, etc. remain the same
// -----------------------------------------------------------------------------
void Console::ExecCommand(const char* command_line)
{
	AddLog("# %s", command_line);

	HistoryPos = -1;
	for (int i = (int)History.size() - 1; i >= 0; i--)
	{
		if (strcmp(History[i].c_str(), command_line) == 0)
		{
			History.erase(History.begin() + i);
			break;
		}
	}
	History.push_back(command_line);

	if (strcmp(command_line, "CLEAR") == 0)
	{
		ClearLog();
	}
	else if (strcmp(command_line, "HELP") == 0)
	{
		AddLog("Commands:");
		AddLog("  HELP");
		AddLog("  CLEAR");
		AddLog("  HISTORY");
	}
	else if (strcmp(command_line, "HISTORY") == 0)
	{
		for (int i = (int)History.size() - 1; i >= 0; i--)
			AddLog("%3d: %s", i, History[i].c_str());
	}
	else
	{
		AddLog("Unknown command: '%s'", command_line);
	}

	ScrollToBottom = true;
}

void Console::AddLog(const char* fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, IM_ARRAYSIZE(buffer), fmt, args);
	buffer[IM_ARRAYSIZE(buffer) - 1] = 0;
	va_end(args);

	Items.push_back(buffer);
	ScrollToBottom = true;
}

void Console::ClearLog()
{
	Items.clear();
	ScrollToBottom = true;
}

int Console::TextEditCallbackStub(ImGuiInputTextCallbackData* data)
{
	Console* console = (Console*)data->UserData;
	return console->TextEditCallback(data);
}

int Console::TextEditCallback(ImGuiInputTextCallbackData* data)
{
	switch (data->EventFlag)
	{
	case ImGuiInputTextFlags_CallbackCompletion:
		// Optional: handle tab-completion
		break;
	case ImGuiInputTextFlags_CallbackHistory:
	{
		const int prevHistoryPos = HistoryPos;
		if (data->EventKey == ImGuiKey_UpArrow)
		{
			if (HistoryPos == -1)
				HistoryPos = (int)History.size() - 1;
			else if (HistoryPos > 0)
				HistoryPos--;
		}
		else if (data->EventKey == ImGuiKey_DownArrow)
		{
			if (HistoryPos != -1)
				if (++HistoryPos >= (int)History.size())
					HistoryPos = -1;
		}

		if (prevHistoryPos != HistoryPos && HistoryPos != -1)
		{
			const std::string& historyStr = History[HistoryPos];
			data->DeleteChars(0, data->BufTextLen);
			data->InsertChars(0, historyStr.c_str());
		}
		break;
	}
	}
	return 0;
}