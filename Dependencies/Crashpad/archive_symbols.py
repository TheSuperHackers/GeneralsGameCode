"""Verify binary/PDB identity and archive a Crashpad build for durable storage."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import zipfile


def identity(binary, pdb):
    pe = subprocess.check_output(["llvm-readobj", "--coff-debug-directory", str(binary)], text=True)
    symbols = subprocess.check_output(["llvm-pdbutil", "dump", "-summary", str(pdb)], text=True)
    binary_guid = re.search(r"PDBGUID: (\{[^}]+\})", pe)
    binary_age = re.search(r"PDBAge: (\d+)", pe)
    pdb_guid = re.search(r"GUID: (\{[^}]+\})", symbols)
    pdb_age = re.search(r"Age: (\d+)", symbols)
    if not all([binary_guid, binary_age, pdb_guid, pdb_age]):
        raise ValueError(f"Cannot read CodeView/PDB identity: {binary.name}")
    if (binary_guid[1].lower(), binary_age[1]) != (pdb_guid[1].lower(), pdb_age[1]):
        raise ValueError(f"PDB does not match {binary.name}")
    return {"binary": binary.name, "pdb": pdb.name,
            "guid": pdb_guid[1], "age": int(pdb_age[1])}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("metadata", type=Path, help="The game's *-crashpad-build.json")
    parser.add_argument("archive", type=Path, help="New ZIP file, never overwritten")
    args = parser.parse_args()
    folder = args.metadata.resolve().parent
    manifest = json.loads(args.metadata.read_text())
    files = []
    for kind in ["GAME", "PDB", "BRIDGE", "BRIDGE_PDB", "HANDLER"]:
        record = manifest[kind]
        path = folder / record["file"]
        if path.parent != folder:
            raise ValueError("Manifest paths must name sibling files")
        if hashlib.sha256(path.read_bytes()).hexdigest() != record["sha256"]:
            raise ValueError(f"Build metadata hash does not match {path.name}")
        files.append(path)
    identities = [identity(files[0], files[1]), identity(files[2], files[3])]
    files.append(args.metadata.resolve())
    files.append(folder / "rts_crashpad_reports.exe")
    for pattern in ["msvcp140*.dll", "vcruntime140*.dll", "concrt140.dll"]:
        files.extend(folder.glob(pattern))
    files.extend(path for path in (folder / "crashpad-notices").rglob("*") if path.is_file())
    args.archive.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(args.archive, "x", compression=zipfile.ZIP_DEFLATED) as archive:
        for path in files:
            archive.write(path, path.relative_to(folder))
        archive.writestr("symbol-identities.json", json.dumps(identities, indent=2))
    print(json.dumps({"archive": str(args.archive), "identities": identities}))


if __name__ == "__main__":
    main()
