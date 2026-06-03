// Zulu launcher: lightweight pre-game shim that checks for updates against
// a JSON manifest in our public GCS bucket and, if a newer release is
// available, downloads the matching installer to %TEMP% and hands off to
// it. On no-update / network failure / user decline it just launches the
// installed game with whatever args were forwarded by the shortcut.
//
// Why this exists:
//   * The game .exe lives in Program Files, so overwriting it needs admin.
//     Putting that admin prompt in the launcher means the game itself can
//     keep running unprivileged.
//   * The installed .exe carries its semver in a VS_VERSION_INFO resource
//     (see GeneralsMD/Code/Main/ZuluVersion.rc.in), so we don't need a
//     sidecar version file to know what's on disk.
//
// "Don't downgrade" rule:
//   We only update when latest > installed (component-wise major.minor.build).
//   That keeps dev builds (which can be ahead of any released installer) from
//   being rolled back to the latest released installer when they run the
//   launcher.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>
#include <shellapi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BuildVariant.h"

// VC6 SDK predates these definitions.
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif

// VC6's wincrypt.h hides the legacy CryptoAPI types behind a _WIN32_WINNT
// guard that isn't tripped in this build, so we forward-declare just the
// types/constants we touch and resolve the entry points out of advapi32
// at runtime. Same pattern StatsUploader.cpp uses for GetAdaptersInfo.
// The "handle" types are ULONG_PTR-sized in modern SDKs but only 32-bit
// on 32-bit Windows (which is the only target VC6 can produce), so plain
// unsigned long is ABI-compatible and avoids needing basetsd.h.
typedef unsigned long LocalHCRYPTPROV;
typedef unsigned long LocalHCRYPTHASH;
typedef unsigned long LocalHCRYPTKEY;
typedef DWORD         LocalALG_ID;

#define LOCAL_CALG_SHA_256        0x0000800c
#define LOCAL_PROV_RSA_AES        24
#define LOCAL_CRYPT_VERIFYCONTEXT 0xF0000000
#define LOCAL_HP_HASHVAL          2

typedef BOOL (WINAPI *FnCryptAcquireContextA)(LocalHCRYPTPROV *, LPCSTR, LPCSTR, DWORD, DWORD);
typedef BOOL (WINAPI *FnCryptReleaseContext)(LocalHCRYPTPROV, DWORD);
typedef BOOL (WINAPI *FnCryptCreateHash)(LocalHCRYPTPROV, LocalALG_ID, LocalHCRYPTKEY, DWORD, LocalHCRYPTHASH *);
typedef BOOL (WINAPI *FnCryptHashData)(LocalHCRYPTHASH, const BYTE *, DWORD, DWORD);
typedef BOOL (WINAPI *FnCryptGetHashParam)(LocalHCRYPTHASH, DWORD, BYTE *, DWORD *, DWORD);
typedef BOOL (WINAPI *FnCryptDestroyHash)(LocalHCRYPTHASH);

// Manifest URL is variant-aware: dev launchers read latest-dev.json so dev
// installs don't get dragged onto the public release line and vice versa.
// Both manifests live under the same kAllowedURLPrefix, so the post-download
// URL pin still applies to both code paths.
static const char *kLatestJsonURLRelease =
    "https://storage.googleapis.com/zulu-installer/latest.json";
static const char *kLatestJsonURLDev =
    "https://storage.googleapis.com/zulu-installer/latest-dev.json";
// Defense-in-depth: refuse to download/run anything whose URL isn't a public
// object in our installer bucket. HTTPS to that bucket already authenticates
// the origin; this pin just prevents a tampered manifest from pointing the
// elevated installer hand-off at an arbitrary external host.
static const char *kAllowedURLPrefix =
    "https://storage.googleapis.com/zulu-installer/";
static const char *kGameExeName  = "generalszh_zulu.exe";
static const char *kInstallerLeafRelease = "Zulu_Setup_update.exe";
static const char *kInstallerLeafDev     = "Zulu_Setup_Dev_update.exe";
static const char *kAppName       = "Zulu";

static bool isDevBuild() {
    return strcmp(ZULU_BUILD_VARIANT_KIND, "dev") == 0;
}

struct SemVer {
    unsigned major;
    unsigned minor;
    unsigned build;
};

static int semVerCompare(const SemVer &a, const SemVer &b) {
    if (a.major != b.major) return (a.major < b.major) ? -1 : 1;
    if (a.minor != b.minor) return (a.minor < b.minor) ? -1 : 1;
    if (a.build != b.build) return (a.build < b.build) ? -1 : 1;
    return 0;
}

static bool parseSemVer(const char *s, SemVer &out) {
    out.major = out.minor = out.build = 0;
    if (!s) return false;
    int n = sscanf(s, "%u.%u.%u", &out.major, &out.minor, &out.build);
    return n >= 1;
}

// Reads FILEVERSION (e.g. 1.4.601) from an exe's VS_FIXEDFILEINFO block.
// Stored as two DWORDs: MS = major<<16 | minor, LS = build<<16 | revision.
static bool getFileVersion(const char *path, SemVer &out) {
    // VC6's headers type these as LPSTR (non-const) even though the APIs
    // don't actually modify the path. Cast through to silence C2664.
    char *pathArg = const_cast<char *>(path);
    DWORD handle = 0;
    DWORD size = GetFileVersionInfoSizeA(pathArg, &handle);
    if (size == 0) return false;
    void *buf = malloc(size);
    if (!buf) return false;
    if (!GetFileVersionInfoA(pathArg, handle, size, buf)) { free(buf); return false; }
    VS_FIXEDFILEINFO *ffi = NULL;
    UINT len = 0;
    if (!VerQueryValueA(buf, "\\", (LPVOID *)&ffi, &len) || !ffi) {
        free(buf);
        return false;
    }
    out.major = HIWORD(ffi->dwFileVersionMS);
    out.minor = LOWORD(ffi->dwFileVersionMS);
    out.build = HIWORD(ffi->dwFileVersionLS);
    free(buf);
    return true;
}

// Purpose-built scanner for the tiny manifests we publish. Not a general
// JSON parser — assumes well-formed UTF-8 we produced ourselves.
static bool jsonGetString(const char *json, const char *key,
                          char *out, size_t outSize) {
    char pat[64];
    _snprintf(pat, sizeof(pat) - 1, "\"%s\"", key);
    pat[sizeof(pat) - 1] = 0;
    const char *p = strstr(json, pat);
    if (!p) return false;
    p += strlen(pat);
    while (*p && *p != ':') ++p;
    if (*p != ':') return false;
    ++p;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') ++p;
    if (*p != '"') return false;
    ++p;
    size_t i = 0;
    while (*p && *p != '"' && i + 1 < outSize) {
        if (*p == '\\' && p[1]) ++p;
        out[i++] = *p++;
    }
    out[i] = 0;
    return *p == '"';
}

// SHA-256 of an entire file, written as 64 lowercase hex chars + nul into
// outHex. Returns false on any failure (file missing, advapi32 missing the
// SHA-256 algorithm, read error). PROV_RSA_AES is the smallest provider
// that exposes SHA-256; available on XP SP3+ and every supported Windows
// after.
static bool computeFileSha256Hex(const char *path, char *outHex, size_t outSize) {
    if (outSize < 65) return false;

    HMODULE advapi = LoadLibraryA("advapi32.dll");
    if (!advapi) return false;
    FnCryptAcquireContextA acquireCtx = (FnCryptAcquireContextA)GetProcAddress(advapi, "CryptAcquireContextA");
    FnCryptReleaseContext  releaseCtx = (FnCryptReleaseContext) GetProcAddress(advapi, "CryptReleaseContext");
    FnCryptCreateHash      createHash = (FnCryptCreateHash)     GetProcAddress(advapi, "CryptCreateHash");
    FnCryptHashData        hashData   = (FnCryptHashData)       GetProcAddress(advapi, "CryptHashData");
    FnCryptGetHashParam    getHashParam = (FnCryptGetHashParam) GetProcAddress(advapi, "CryptGetHashParam");
    FnCryptDestroyHash     destroyHash = (FnCryptDestroyHash)   GetProcAddress(advapi, "CryptDestroyHash");
    if (!acquireCtx || !releaseCtx || !createHash || !hashData || !getHashParam || !destroyHash) {
        FreeLibrary(advapi);
        return false;
    }

    HANDLE hF = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hF == INVALID_HANDLE_VALUE) { FreeLibrary(advapi); return false; }

    LocalHCRYPTPROV hProv = 0;
    LocalHCRYPTHASH hHash = 0;
    bool ok = false;
    if (acquireCtx(&hProv, NULL, NULL, LOCAL_PROV_RSA_AES, LOCAL_CRYPT_VERIFYCONTEXT)) {
        if (createHash(hProv, LOCAL_CALG_SHA_256, 0, 0, &hHash)) {
            BYTE buf[64 * 1024];
            DWORD got = 0;
            ok = true;
            while (ReadFile(hF, buf, sizeof(buf), &got, NULL) && got > 0) {
                if (!hashData(hHash, buf, got, 0)) { ok = false; break; }
            }
            if (ok) {
                BYTE digest[32];
                DWORD digestLen = sizeof(digest);
                if (getHashParam(hHash, LOCAL_HP_HASHVAL, digest, &digestLen, 0)
                        && digestLen == 32) {
                    static const char hex[] = "0123456789abcdef";
                    int i;
                    for (i = 0; i < 32; ++i) {
                        outHex[i * 2]     = hex[(digest[i] >> 4) & 0xF];
                        outHex[i * 2 + 1] = hex[digest[i] & 0xF];
                    }
                    outHex[64] = 0;
                } else {
                    ok = false;
                }
            }
            destroyHash(hHash);
        }
        releaseCtx(hProv, 0);
    }
    CloseHandle(hF);
    FreeLibrary(advapi);
    return ok;
}

static void applyTimeouts(HINTERNET hI, DWORD millis) {
    InternetSetOptionA(hI, INTERNET_OPTION_CONNECT_TIMEOUT, &millis, sizeof(millis));
    InternetSetOptionA(hI, INTERNET_OPTION_RECEIVE_TIMEOUT, &millis, sizeof(millis));
    InternetSetOptionA(hI, INTERNET_OPTION_SEND_TIMEOUT, &millis, sizeof(millis));
}

static DWORD urlFlags(const char *url) {
    DWORD f = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE
            | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_KEEP_CONNECTION;
    if (strncmp(url, "https://", 8) == 0) f |= INTERNET_FLAG_SECURE;
    return f;
}

// HTTPS GET into a heap-allocated nul-terminated buffer. Caller frees.
// Returns NULL on any failure (offline, 404, TLS error). Callers fall
// through and just launch the game; an update check should never block play.
static char *httpGet(const char *url, DWORD *outSize) {
    HINTERNET hI = InternetOpenA("ZuluLauncher",
        INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hI) return NULL;
    applyTimeouts(hI, 10000);

    HINTERNET hU = InternetOpenUrlA(hI, url, NULL, 0, urlFlags(url), 0);
    if (!hU) { InternetCloseHandle(hI); return NULL; }

    DWORD cap = 4096;
    DWORD len = 0;
    char *buf = (char *)malloc(cap);
    if (!buf) { InternetCloseHandle(hU); InternetCloseHandle(hI); return NULL; }

    for (;;) {
        if (len + 4096 + 1 > cap) {
            cap *= 2;
            char *nb = (char *)realloc(buf, cap);
            if (!nb) {
                free(buf);
                InternetCloseHandle(hU); InternetCloseHandle(hI);
                return NULL;
            }
            buf = nb;
        }
        DWORD got = 0;
        if (!InternetReadFile(hU, buf + len, 4096, &got)) {
            free(buf);
            InternetCloseHandle(hU); InternetCloseHandle(hI);
            return NULL;
        }
        if (got == 0) break;
        len += got;
    }
    buf[len] = 0;
    if (outSize) *outSize = len;
    InternetCloseHandle(hU);
    InternetCloseHandle(hI);
    return buf;
}

// Streams url -> filePath. Deletes the file on any failure so we don't
// leave a half-written .exe in %TEMP%.
static bool httpDownloadToFile(const char *url, const char *filePath) {
    HINTERNET hI = InternetOpenA("ZuluLauncher",
        INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hI) return false;
    applyTimeouts(hI, 30000);

    HINTERNET hU = InternetOpenUrlA(hI, url, NULL, 0, urlFlags(url), 0);
    if (!hU) { InternetCloseHandle(hI); return false; }

    HANDLE hF = CreateFileA(filePath, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hF == INVALID_HANDLE_VALUE) {
        InternetCloseHandle(hU); InternetCloseHandle(hI);
        return false;
    }

    char chunk[16 * 1024];
    for (;;) {
        DWORD got = 0;
        if (!InternetReadFile(hU, chunk, sizeof(chunk), &got)) {
            CloseHandle(hF); DeleteFileA(filePath);
            InternetCloseHandle(hU); InternetCloseHandle(hI);
            return false;
        }
        if (got == 0) break;
        DWORD written = 0;
        if (!WriteFile(hF, chunk, got, &written, NULL) || written != got) {
            CloseHandle(hF); DeleteFileA(filePath);
            InternetCloseHandle(hU); InternetCloseHandle(hI);
            return false;
        }
    }
    CloseHandle(hF);
    InternetCloseHandle(hU);
    InternetCloseHandle(hI);
    return true;
}

static void getInstallDir(char *out, DWORD outSize) {
    GetModuleFileNameA(NULL, out, outSize);
    char *lastSlash = strrchr(out, '\\');
    if (lastSlash) *lastSlash = 0;
}

// Returns the substring of GetCommandLine() AFTER the launcher's own exe.
// Shortcuts will pass "-mod Zulu.big" (or whatever the user adds); we
// forward that verbatim to the game.
static const char *extractArgsAfterExe(const char *cmdLine) {
    const char *p = cmdLine;
    while (*p == ' ' || *p == '\t') ++p;
    if (*p == '"') {
        ++p;
        while (*p && *p != '"') ++p;
        if (*p == '"') ++p;
    } else {
        while (*p && *p != ' ' && *p != '\t') ++p;
    }
    while (*p == ' ' || *p == '\t') ++p;
    return p;
}

static bool launchGame(const char *gameExe, const char *extraArgs) {
    char cmd[4096];
    if (extraArgs && *extraArgs) {
        _snprintf(cmd, sizeof(cmd) - 1, "\"%s\" %s", gameExe, extraArgs);
    } else {
        _snprintf(cmd, sizeof(cmd) - 1, "\"%s\"", gameExe);
    }
    cmd[sizeof(cmd) - 1] = 0;

    char workDir[MAX_PATH];
    strncpy(workDir, gameExe, sizeof(workDir));
    workDir[sizeof(workDir) - 1] = 0;
    char *lastSlash = strrchr(workDir, '\\');
    if (lastSlash) *lastSlash = 0;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    BOOL ok = CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL,
        workDir, &si, &pi);
    if (!ok) return false;
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    char installDir[MAX_PATH];
    getInstallDir(installDir, sizeof(installDir));

    char gameExe[MAX_PATH];
    _snprintf(gameExe, sizeof(gameExe) - 1, "%s\\%s", installDir, kGameExeName);
    gameExe[sizeof(gameExe) - 1] = 0;

    const char *fwdArgs = extractArgsAfterExe(GetCommandLineA());

    if (GetFileAttributesA(gameExe) == INVALID_FILE_ATTRIBUTES) {
        MessageBoxA(NULL,
            "Could not find generalszh_zulu.exe next to the launcher.\n"
            "Please reinstall Zulu.",
            kAppName, MB_OK | MB_ICONERROR);
        return 1;
    }

    const bool devBuild = isDevBuild();
    const char *manifestUrl = devBuild ? kLatestJsonURLDev : kLatestJsonURLRelease;
    const char *installerLeaf = devBuild ? kInstallerLeafDev : kInstallerLeafRelease;

    SemVer installed = {0, 0, 0};
    bool haveInstalled = getFileVersion(gameExe, installed);

    SemVer latest = {0, 0, 0};
    bool haveLatest = false;
    char latestUrl[2048]; latestUrl[0] = 0;
    char latestVersion[64]; latestVersion[0] = 0;
    char latestSha[80]; latestSha[0] = 0;
    bool haveLatestSha = false;

    char *json = httpGet(manifestUrl, NULL);
    if (json) {
        if (jsonGetString(json, "version", latestVersion, sizeof(latestVersion)) &&
            jsonGetString(json, "url", latestUrl, sizeof(latestUrl))) {
            haveLatest = parseSemVer(latestVersion, latest);
        }
        haveLatestSha = jsonGetString(json, "sha256", latestSha, sizeof(latestSha));
        free(json);
    }

    const bool urlOk = (latestUrl[0] != 0) &&
        (strncmp(latestUrl, kAllowedURLPrefix, strlen(kAllowedURLPrefix)) == 0);

    // Dev gate: any SHA mismatch between the manifest and the installed exe
    // triggers a prompt, since dev builds don't bump semver between rebuilds.
    // Release gate: strict ">" semver comparison so we never downgrade.
    char installedSha[80]; installedSha[0] = 0;
    bool needUpdate = false;
    if (devBuild) {
        bool haveInstalledSha = computeFileSha256Hex(gameExe, installedSha, sizeof(installedSha));
        needUpdate = haveLatestSha && haveInstalledSha && urlOk &&
                     (_stricmp(latestSha, installedSha) != 0);
    } else {
        needUpdate = haveInstalled && haveLatest && urlOk &&
                     semVerCompare(latest, installed) > 0;
    }

    if (needUpdate) {
        char msg[512];
        if (devBuild) {
            _snprintf(msg, sizeof(msg) - 1,
                "A new Zulu dev build is available.\n\n"
                "Installed SHA: %.12s...\n"
                "Latest SHA:    %.12s...\n\n"
                "Download and install the update now?",
                installedSha, latestSha);
        } else {
            _snprintf(msg, sizeof(msg) - 1,
                "A newer Zulu release is available.\n\n"
                "Installed:  %u.%u.%u\n"
                "Latest:     %s\n\n"
                "Download and install the update now?",
                installed.major, installed.minor, installed.build,
                latestVersion);
        }
        msg[sizeof(msg) - 1] = 0;
        int rc = MessageBoxA(NULL, msg, kAppName,
            MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON1);
        if (rc == IDYES) {
            char tempDir[MAX_PATH];
            GetTempPathA(sizeof(tempDir), tempDir);
            char installerPath[MAX_PATH];
            _snprintf(installerPath, sizeof(installerPath) - 1, "%s%s",
                tempDir, installerLeaf);
            installerPath[sizeof(installerPath) - 1] = 0;

            HCURSOR oldCursor = SetCursor(LoadCursorA(NULL, IDC_WAIT));
            bool ok = httpDownloadToFile(latestUrl, installerPath);
            SetCursor(oldCursor);

            if (ok) {
                // /S = NSIS silent; /D=<dir> must be the LAST arg, unquoted,
                // and is honored even in silent mode. The installer's
                // manifest already requests admin, so this triggers UAC.
                char params[MAX_PATH + 64];
                _snprintf(params, sizeof(params) - 1, "/S /D=%s", installDir);
                params[sizeof(params) - 1] = 0;
                SHELLEXECUTEINFOA sei;
                ZeroMemory(&sei, sizeof(sei));
                sei.cbSize = sizeof(sei);
                sei.fMask = SEE_MASK_NOCLOSEPROCESS;
                sei.lpVerb = "open";
                sei.lpFile = installerPath;
                sei.lpParameters = params;
                sei.lpDirectory = tempDir;
                sei.nShow = SW_SHOWNORMAL;
                if (ShellExecuteExA(&sei) && sei.hProcess) {
                    // Wait for the elevated installer to finish so we
                    // can re-launch the (now-updated) game exe with
                    // the same args the user passed to *this* launcher
                    // invocation. The NSIS script's post-install Exec
                    // is intentionally removed: it only knew about the
                    // shortcut's hardcoded LAUNCHARGS and would drop
                    // anything extra (e.g. -zulu_debug) the user added
                    // on the command line. The launcher .exe being
                    // overwritten while we wait is fine — Windows
                    // keeps the in-memory image valid until we exit.
                    SetCursor(LoadCursorA(NULL, IDC_WAIT));
                    WaitForSingleObject(sei.hProcess, INFINITE);
                    SetCursor(LoadCursorA(NULL, IDC_ARROW));
                    DWORD exitCode = 1;
                    GetExitCodeProcess(sei.hProcess, &exitCode);
                    CloseHandle(sei.hProcess);

                    if (exitCode == 0) {
                        // Install succeeded. Fall through to
                        // launchGame below; the path to the game exe
                        // is unchanged across the in-place upgrade so
                        // fwdArgs flows straight into the new binary.
                    } else {
                        MessageBoxA(NULL,
                            "Zulu installer reported a failure. The "
                            "game will be launched at the previously "
                            "installed version.",
                            kAppName, MB_OK | MB_ICONWARNING);
                    }
                } else {
                    MessageBoxA(NULL,
                        "Could not start the Zulu installer. The game will be "
                        "launched at the installed version.",
                        kAppName, MB_OK | MB_ICONWARNING);
                }
            } else {
                MessageBoxA(NULL,
                    "Update download failed. The game will be launched at "
                    "the installed version.",
                    kAppName, MB_OK | MB_ICONWARNING);
            }
            // Fall through and launch the existing installed version.
        }
    }

    if (!launchGame(gameExe, fwdArgs)) {
        MessageBoxA(NULL,
            "Could not launch generalszh_zulu.exe.",
            kAppName, MB_OK | MB_ICONERROR);
        return 1;
    }
    return 0;
}
