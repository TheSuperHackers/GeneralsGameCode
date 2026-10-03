/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <windows.h>
#include <cstdio>
#include <vector>
#include "client/crash_report_database.h"
#include "client/prune_crash_reports.h"
#include "client/settings.h"
#include "base/strings/utf_string_conversions.h"

int wmain(int argc, wchar_t** argv)
{
    if (argc < 3 || argc > 4)
    {
        std::fputs("Usage: rts_crashpad_reports <database> list|prune|export [export-directory]\n", stderr);
        return 2;
    }

    // Open an existing database only. Export never enables uploads or marks a
    // report as uploaded, and never edits Crashpad's internal files directly.
    auto database = crashpad::CrashReportDatabase::InitializeWithoutCreating(base::FilePath(argv[1]));
    if (!database)
    {
        return 1;
    }

    if (wcscmp(argv[2], L"prune") == 0 && argc == 3)
    {
        crashpad::BinaryPruneCondition retention(crashpad::BinaryPruneCondition::OR,
            new crashpad::AgePruneCondition(30),
            new crashpad::DatabaseSizePruneCondition(500 * 1024));
        std::printf("Deleted %zu reports\n", crashpad::PruneCrashReportDatabase(database.get(), &retention));
        return 0;
    }

    const bool exporting = wcscmp(argv[2], L"export") == 0 && argc == 4;
    if (!exporting && (wcscmp(argv[2], L"list") != 0 || argc != 3))
    {
        return 2;
    }

    std::vector<crashpad::CrashReportDatabase::Report> pending;
    std::vector<crashpad::CrashReportDatabase::Report> completed;
    if (database->GetPendingReports(&pending) != crashpad::CrashReportDatabase::kNoError
        || database->GetCompletedReports(&completed) != crashpad::CrashReportDatabase::kNoError)
    {
        return 1;
    }

    if (exporting && !CreateDirectoryW(argv[3], nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        return 1;
    }

    bool uploads = true;
    if (!database->GetSettings()->GetUploadsEnabled(&uploads))
    {
        return 1;
    }

    std::printf("uploads-enabled=%d\n", uploads);
    pending.insert(pending.end(), completed.begin(), completed.end());
    for (const auto& report : pending)
    {
        std::printf("%s %llu\n", report.uuid.ToString().c_str(),
                    static_cast<unsigned long long>(report.total_size));
        if (exporting)
        {
            // These are finalized, immutable report files enumerated by the
            // database API. CopyFile holds the source open during the copy;
            // concurrent pruning before it opens is reported as an error.
            const base::FilePath destination = base::FilePath(argv[3])
                .Append(base::UTF8ToWide(report.uuid.ToString() + ".dmp"));
            if (!CopyFileW(report.file_path.value().c_str(), destination.value().c_str(), TRUE))
            {
                std::fprintf(stderr, "Export failed: Windows error %lu\n", GetLastError());
                return 1;
            }
        }
    }

    return 0;
}
