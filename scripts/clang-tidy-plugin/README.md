# GeneralsGameCode Clang-Tidy Custom Checks

Custom clang-tidy checks for the GeneralsGameCode codebase. **This is primarily designed for Windows**, where checks are built directly into clang-tidy. Mac/Linux users can optionally use the plugin version.

## Quick Start

### Windows

**Option 1: Use Pre-built clang-tidy**
- Precompiled binaries can be found at: https://github.com/TheSuperHackers/GeneralsTools

**Option 2: Build It Yourself**

Follow the manual steps in the [Windows: Building Checks Into clang-tidy](#windows-building-checks-into-clang-tidy) section below.

**Usage**
```powershell
llvm-project\build\bin\clang-tidy.exe -p build/clang-tidy `
  --checks='-*,generals-use-is-empty,generals-use-this-instead-of-singleton' `
  file.cpp
```

### macOS/Linux (Plugin Version)

If you prefer the plugin approach on macOS/Linux:

```bash
# Build the plugin
cd scripts/clang-tidy-plugin
mkdir build && cd build
cmake .. -DLLVM_DIR=/path/to/llvm/lib/cmake/llvm
cmake --build . --config Release

# Use with --load flag
clang-tidy -p build/clang-tidy \
  --checks='-*,generals-use-is-empty' \
  -load scripts/clang-tidy-plugin/build/lib/libGeneralsGameCodeClangTidyPlugin.so \
  file.cpp
```

**Note:** On Windows, plugin loading via `--load` is **not supported** due to DLL static initialization limitations. Windows users must use the built-in version.

## Checks

### `generals-use-is-empty`

Finds uses of `getLength() == 0`, `getLength() > 0`, `compare("") == 0`, or `compareNoCase("") == 0` on `AsciiString` and `UnicodeString`, and `Get_Length() == 0` on `StringClass` and `WideStringClass`, and suggests using `isEmpty()`/`Is_Empty()` or `!isEmpty()`/`!Is_Empty()` instead.

**Examples:**

```cpp
// Before (AsciiString/UnicodeString)
if (str.getLength() == 0) { ... }
if (str.getLength() > 0) { ... }
if (str.compare("") == 0) { ... }
if (str.compareNoCase("") == 0) { ... }
if (str.compare(AsciiString::TheEmptyString) == 0) { ... }

// After (AsciiString/UnicodeString)
if (str.isEmpty()) { ... }
if (!str.isEmpty()) { ... }
if (str.isEmpty()) { ... }
if (str.isEmpty()) { ... }
if (str.isEmpty()) { ... }

// Before (StringClass/WideStringClass)
if (str.Get_Length() == 0) { ... }
if (str.Get_Length() > 0) { ... }

// After (StringClass/WideStringClass)
if (str.Is_Empty()) { ... }
if (!str.Is_Empty()) { ... }
```

### `generals-use-this-instead-of-singleton`

Finds uses of singleton global variables (like `TheGameLogic->method()` or `TheGlobalData->member`) inside member functions of the same class type, and suggests using the member directly (e.g., `method()` or `member`) instead of the singleton reference.

**Examples:**

```cpp
// Before
void GameLogic::update() {
  UnsignedInt now = TheGameLogic->getFrame();
  TheGameLogic->setFrame(now + 1);
  TheGameLogic->m_frame = 10;
}

// After
void GameLogic::update() {
  UnsignedInt now = getFrame();
  setFrame(now + 1);
  m_frame = 10;
}
```

## Prerequisites

Before using clang-tidy, you need to generate a compile commands database:

```bash
cmake -B build/clang-tidy -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON -G Ninja
```

This creates `build/clang-tidy/compile_commands.json` which tells clang-tidy how to compile each file.

## Windows: Building Checks Into clang-tidy

Since plugin loading via `--load` doesn't work on Windows, the checks are integrated directly into the clang-tidy source tree. This is the recommended approach for Windows.

### Step 1: Clone and Build LLVM

### Step 2: Copy Plugin Files to LLVM Source Tree

```powershell
# Create the plugin directory in clang-tools-extra
mkdir llvm-project\clang-tools-extra\clang-tidy\plugins\generalsgamecode

# Copy the plugin source files
cp -r scripts\clang-tidy-plugin\*.cpp llvm-project\clang-tools-extra\clang-tidy\plugins\generalsgamecode\
cp -r scripts\clang-tidy-plugin\*.h llvm-project\clang-tools-extra\clang-tidy\plugins\generalsgamecode\
cp -r scripts\clang-tidy-plugin\readability llvm-project\clang-tools-extra\clang-tidy\plugins\generalsgamecode\
```

**Important:** After copying, you need to update the include paths in the headers:
- Change `#include "clang-tidy/ClangTidyCheck.h"` to `#include "../../../ClangTidyCheck.h"`
- Change `#include "clang-tidy/ClangTidyModule.h"` to `#include "../../ClangTidyModule.h"`

### Step 3: Create CMakeLists.txt for the Module

Create `llvm-project/clang-tools-extra/clang-tidy/plugins/generalsgamecode/CMakeLists.txt`:

```cmake
add_clang_library(clangTidyGeneralsGameCodeModule STATIC
  GeneralsGameCodeTidyModule.cpp
  readability/UseIsEmptyCheck.cpp
  readability/UseThisInsteadOfSingletonCheck.cpp

  LINK_LIBS
  clangTidy
  clangTidyUtils
  )

clang_target_link_libraries(clangTidyGeneralsGameCodeModule
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangLex
  clangTooling
  )
```

### Step 4: Register the Module

**Modify `llvm-project/clang-tools-extra/clang-tidy/CMakeLists.txt`:**

Add the subdirectory:
```cmake
add_subdirectory(plugins/generalsgamecode)
```

Add to `ALL_CLANG_TIDY_CHECKS`:
```cmake
set(ALL_CLANG_TIDY_CHECKS
  ...
  clangTidyGeneralsGameCodeModule
  ...
)
```

**Modify `llvm-project/clang-tools-extra/clang-tidy/ClangTidyForceLinker.h`:**

Add the anchor to force linker inclusion:
```cpp
// This anchor is used to force the linker to link the GeneralsGameCodeModule.
extern volatile int GeneralsGameCodeModuleAnchorSource;
static int LLVM_ATTRIBUTE_UNUSED GeneralsGameCodeModuleAnchorDestination =
    GeneralsGameCodeModuleAnchorSource;
```

**Modify `llvm-project/clang-tools-extra/clang-tidy/plugins/generalsgamecode/GeneralsGameCodeTidyModule.cpp`:**

Ensure the anchor is defined:
```cpp
namespace clang::tidy {
// ... module registration ...

// Force linker to include this module
volatile int GeneralsGameCodeModuleAnchorSource = 0;
}
```

### Step 5: Rebuild clang-tidy

```powershell
cd llvm-project\build
ninja clang-tidy
```

### Step 6: Use the Built-in Checks

Once rebuilt, the checks are always available - no `--load` flag needed:

```powershell
llvm-project\build\bin\clang-tidy.exe -p build/clang-tidy `
  --checks='-*,generals-use-is-empty,generals-use-this-instead-of-singleton' `
  file.cpp
```


## macOS/Linux: Building the Plugin

If you're on macOS or Linux and want to use the plugin version:

### Prerequisites

**macOS:**
```bash
brew install llvm@21
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get install llvm-21-dev clang-21 libclang-21-dev
```

### Building the Plugin

```bash
cd scripts/clang-tidy-plugin
mkdir build && cd build
cmake .. -DLLVM_DIR=/path/to/llvm/lib/cmake/llvm -DClang_DIR=/path/to/clang/lib/cmake/clang
cmake --build . --config Release
```

The plugin will be built as a shared library (`.so` on Linux, `.dylib` on macOS) in the `build/lib/` directory.

### Using the Plugin

```bash
clang-tidy -p build/clang-tidy \
  --checks='-*,generals-use-is-empty,generals-use-this-instead-of-singleton' \
  -load scripts/clang-tidy-plugin/build/lib/libGeneralsGameCodeClangTidyPlugin.so \
  file.cpp
```

**Important:** The plugin must be built with the same LLVM version as the `clang-tidy` executable in your PATH. The CMake build will display which LLVM version it found (e.g., `Found LLVM 21.1.7`). Verify this matches your `clang-tidy --version` output.

- Windows plugins not working is a known limitation - see [GitHub issue #159710](https://github.com/llvm/llvm-project/issues/159710) and [LLVM Discourse discussion](https://discourse.llvm.org/t/clang-tidy-is-clang-tidy-out-of-tree-check-plugin-load-mechanism-guaranteed-to-work-with-msvc/84111)
