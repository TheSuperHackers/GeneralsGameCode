/* SPDX-License-Identifier: GPL-3.0-or-later */
// A valid executable that deliberately does not service Crashpad's IPC pipe.
#include <windows.h>
#include <tlhelp32.h>

int main()
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W process = {};
    process.dwSize = sizeof(process);
    HANDLE parent = nullptr;
    if (Process32FirstW(snapshot, &process))
    {
        do
        {
            if (process.th32ProcessID == GetCurrentProcessId())
            {
                parent = OpenProcess(SYNCHRONIZE, FALSE, process.th32ParentProcessID);
                break;
            }
        } while (Process32NextW(snapshot, &process));
    }

    CloseHandle(snapshot);
    if (!parent)
    {
        return 1;
    }

    // Exit with the dedicated test parent so this fixture leaves no process.
    WaitForSingleObject(parent, 20000);
    CloseHandle(parent);
    return 0;
}
