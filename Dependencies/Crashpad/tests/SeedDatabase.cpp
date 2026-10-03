/* SPDX-License-Identifier: GPL-3.0-or-later */
// Test fixture: three synthetic 200 MiB reports exercise real database pruning.
#include "client/crash_report_database.h"
#include "client/settings.h"

int wmain(int argc, wchar_t** argv)
{
    if (argc != 2)
    {
        return 2;
    }

    auto database = crashpad::CrashReportDatabase::Initialize(base::FilePath(argv[1]));
    if (!database || !database->GetSettings()->SetUploadsEnabled(false))
    {
        return 1;
    }

    for (int index = 0; index < 3; ++index)
    {
        std::unique_ptr<crashpad::CrashReportDatabase::NewReport> report;
        if (database->PrepareNewCrashReport(&report) != crashpad::CrashReportDatabase::kNoError)
        {
            return 1;
        }

        const char end = 0;
        if (report->Writer()->Seek(200 * 1024 * 1024 - 1, SEEK_SET) < 0
            || !report->Writer()->Write(&end, 1))
        {
            return 1;
        }

        crashpad::UUID uuid;
        if (database->FinishedWritingCrashReport(std::move(report), &uuid)
            != crashpad::CrashReportDatabase::kNoError)
        {
            return 1;
        }
    }

    return 0;
}
