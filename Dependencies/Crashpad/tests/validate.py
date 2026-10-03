"""Run only the dedicated Crashpad test executable, using fresh data directories."""
import argparse
import concurrent.futures
import hashlib
import json
from pathlib import Path
import re
import shutil
import struct
import subprocess
import tempfile
import time


def exception(path):
    data = path.read_bytes()
    assert data[:4] == b"MDMP", path
    streams, directory = struct.unpack_from("<II", data, 8)
    for index in range(streams):
        kind, size, offset = struct.unpack_from("<III", data, directory + index * 12)
        if kind == 6:
            return {"thread": struct.unpack_from("<I", data, offset)[0],
                    "code": struct.unpack_from("<I", data, offset + 8)[0],
                    "address": struct.unpack_from("<Q", data, offset + 24)[0]}
    raise AssertionError(f"No exception stream in {path}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    parser.add_argument("output", type=Path, help="An existing directory under build/")
    args = parser.parse_args()
    executable = args.executable.resolve()
    assert executable.name == "rts_crashpad_test.exe"
    runtime_directory = executable.parents[2] / executable.parent.name
    report_tool = runtime_directory / "rts_crashpad_reports.exe"
    seed_tool = executable.parent / "rts_crashpad_seed.exe"
    wrong_handler = executable.parent / "rts_crashpad_wrong_handler.exe"
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    assert "build" in output.parts, "Keep test reports under build/"
    run = Path(tempfile.mkdtemp(prefix="crashpad test caf\u00e9 \u6f22\u5b57 ", dir=output))
    installed = run / "installed"
    installed.mkdir()
    for name in [executable.name, "rts_crashpad_test.pdb", "crashpad_handler.exe"]:
        shutil.copy2(executable.parent / name, installed / name)
    # Use the current adapter even if an unchanged import library meant the
    # test executable did not relink and run its post-build copy command.
    shutil.copy2(runtime_directory / "rts_crashpad.dll", installed / "rts_crashpad.dll")
    executable = installed / executable.name
    shutil.copy2(report_tool, installed / report_tool.name)
    report_tool = installed / report_tool.name
    results = []

    def execute(mode, directory=None):
        directory = directory or run / mode
        directory.mkdir(exist_ok=True)
        start = time.monotonic()
        process = subprocess.run([str(executable), str(directory), mode],
                                 capture_output=True, timeout=25)
        elapsed = time.monotonic() - start
        dumps = list(directory.rglob("*.dmp"))
        record = {"mode": mode, "exit": process.returncode & 0xffffffff,
                  "seconds": round(elapsed, 3), "reports": len(dumps),
                  "stdout": process.stdout.decode(errors="replace"),
                  "stderr": process.stderr.decode(errors="replace")}
        if dumps and mode != "shutdown":
            record["exception"] = exception(dumps[0])
        print(json.dumps(record), flush=True)
        results.append(record)
        return record, dumps

    for mode, code in [("main", 0xc0000005), ("worker", 0xc0000005),
                       ("stack", 0xc00000fd), ("heap", 0xc0000374),
                       ("explicit", 0x517a7ed), ("duplicate", 0x517a7ed)]:
        record, dumps = execute(mode)
        assert len(dumps) == 1, record
        assert record["exception"]["code"] == code, record
        threads = re.findall(r"fault-thread=(\d+)", record["stdout"])
        assert record["exception"]["thread"] == int(threads[-1]), record

    record, dumps = execute("shutdown")
    assert record["exit"] == 0 and not dumps, record
    record, dumps = execute("dead-handler")
    assert record["exit"] == 1 and not dumps and record["seconds"] < 22, record

    handler = installed / "crashpad_handler.exe"
    saved = handler.with_suffix(".saved")
    handler.rename(saved)
    try:
        record, dumps = execute("missing-handler")
        assert record["exit"] == 3 and "filter-restored=1" in record["stdout"] and not dumps, record
        handler.write_text("This is not a Windows executable.")
        record, dumps = execute("invalid-handler")
        assert record["exit"] == 3 and "filter-restored=1" in record["stdout"] and not dumps, record
        handler.unlink()
        shutil.copy2(wrong_handler, handler)
        record, dumps = execute("unresponsive-handler")
        assert record["exit"] == 3 and "filter-restored=1" in record["stdout"] and not dumps, record
        assert 4 < record["seconds"] < 10, record
        handler.unlink()
    finally:
        saved.rename(handler)

    blocked = run / "blocked-database"
    blocked.mkdir()
    (blocked / "CrashDumps").write_text("A file prevents database creation.")
    record, dumps = execute("blocked-database", blocked)
    assert record["exit"] == 3 and not dumps, record

    shared = run / "concurrent"
    shared.mkdir()
    with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
        list(pool.map(lambda _: execute("explicit", shared), range(3)))
    assert len(list(shared.rglob("*.dmp"))) == 3

    source = run / "main" / "CrashDumps" / "Crashpad"
    exported = run / "exported"
    listing = subprocess.check_output([str(report_tool), str(source), "export", str(exported)], text=True)
    assert "uploads-enabled=0" in listing
    original = next(source.rglob("*.dmp"))
    copied = next(exported.glob("*.dmp"))
    assert hashlib.sha256(original.read_bytes()).digest() == hashlib.sha256(copied.read_bytes()).digest()

    retained = run / "retention"
    database = retained / "CrashDumps" / "Crashpad"
    database.parent.mkdir(parents=True)
    subprocess.run([str(seed_tool), str(database)], check=True, timeout=30)
    assert len(list(database.rglob("*.dmp"))) == 3
    record, dumps = execute("shutdown", retained)
    assert record["exit"] == 0 and len(dumps) == 2, record
    assert sum(path.stat().st_size for path in dumps) < 500 * 1024 * 1024
    (run / "results.json").write_text(json.dumps(results, indent=2), encoding="utf-8")
    print("PASS: " + str(run).encode("ascii", "backslashreplace").decode())


if __name__ == "__main__":
    main()
