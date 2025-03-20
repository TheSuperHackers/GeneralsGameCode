/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/////////////////////////////////////////////////////////////////////////EA-V1
// $File: //depot/GeneralsMD/Staging/code/Tools/assetcull/assetcull.cpp $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/28 14:54:04 $
//
// ©2003 Electronic Arts
//
// simple recursive directory compare tool for asset culling
/////////////////////////////////////////////////////////////////////////TheSuperHackers-V2
// $File: //depot/GeneralsMD/code/Tools/assetcull/assetcull.cpp $
// $Contributor: DevGeniusCode $
// $Revision: #2 $
// $DateTime: 2025/03/14 $
//
// Enhancements:
// - Replaced `_findfirst` and `_findnext` with `std::filesystem` for modern, cross-platform directory iteration.
// - Switched from `fopen` to `std::ifstream` for more efficient and safer file comparisons.
// - Changed from fixed-size buffers (16384 bytes) to iterators for more efficient and modern file comparison.
// - Improved code readability with range-based `for` loops and `std::string`.
//
// Bug Fixes:
// - Ensured proper file permissions `std::filesystem::permissions` before deletion to avoid errors.
//
// Other Improvements:
// - Cleaned up batch file creation logic for better formatting and handling of file paths.
//////////////////////////////////////////////////////////////////////////////


#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

/*
  Usage: assetcull <dir1> <dir2> <bat-file>

  Description:
    All files in <dir1> and <dir2> (and subdirectories) are compared
    binary. If an identical file exists it is removed from <dir1>
    and a corresponding DEL line is written to the given batch file.
*/

static bool filesEqual(const std::string &fn1, const std::string &fn2) {
    // Check if both files exist and are of the same size
    std::ifstream f1(fn1, std::ios::binary);
    std::ifstream f2(fn2, std::ios::binary);

    if (!f1 || !f2) return false;

    // Check file sizes
    f1.seekg(0, std::ios::end);
    f2.seekg(0, std::ios::end);
    if (f1.tellg() != f2.tellg()) return false;

    // Compare files byte-by-byte
    f1.seekg(0, std::ios::beg);
    f2.seekg(0, std::ios::beg);
    return std::equal(std::istreambuf_iterator<char>(f1), std::istreambuf_iterator<char>(),
                      std::istreambuf_iterator<char>(f2));
}

static int recursiveCull(std::ofstream &batchFile, const fs::path &dir1, const fs::path &dir2, const fs::path &relDir) {
    // Ensure both directories exist
    fs::path dir1Path = dir1 / relDir;
    fs::path dir2Path = dir2 / relDir;

    if (!fs::exists(dir1Path) || !fs::exists(dir2Path)) return 0;

    std::vector<fs::path> subdir, dupfiles;
    int deleted = 0;

    // Walk through dir2, collect subdirectories and check for duplicate files
    for (const auto &entry : fs::directory_iterator(dir2Path)) {
        if (entry.is_directory()) {
            // Ignore "." and ".."
            if (entry.path().filename() != "." && entry.path().filename() != "..") {
                subdir.push_back(entry.path().filename());
            }
        } else {
            fs::path file1 = dir1Path / entry.path().filename();
            fs::path file2 = entry.path();
            if (filesEqual(file1.string(), file2.string())) {
                dupfiles.push_back(entry.path().filename());
            }
        }
    }

    // Remove duplicate files and write to batch file
    for (const auto &file : dupfiles) {
        fs::path fullPath = dir1Path / file;
        std::error_code ec;
        fs::permissions(fullPath, fs::perms::owner_write, ec);  // Ensure write permissions
        if (fs::remove(fullPath)) {
            ++deleted;
            batchFile << "attrib -r \"" << fullPath.string() << "\"\n";
            batchFile << "del -r \"" << fullPath.string() << "\"\n";
        } else {
            std::cerr << "Error: Can't delete " << fullPath.string() << std::endl;
        }
    }

    // Recursively walk subdirectories
    for (const auto &sub : subdir) {
        deleted += recursiveCull(batchFile, dir1, dir2, relDir / sub);
    }

    return deleted;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cout << "Usage: assetcull <dir1> <dir2> <bat-file>\n\n"
                  << "Description:\n"
                  << "  All files in <dir1> and <dir2> (and subdirectories) are compared\n"
                  << "  binary. If an identical file exists it is removed from <dir1>\n"
                  << "  and a corresponding DEL line is written to the given batch file.\n";
        return 10;
    }

    std::ofstream batchFile(argv[3]);
    if (!batchFile) {
        std::cout << "Error: Can't create " << argv[3] << std::endl;
        return 10;
    }

    int n = recursiveCull(batchFile, argv[1], argv[2], ".");
    batchFile.close();
    std::cout << "assetcull: " << n << " files culled." << std::endl;

    return 0;
}
