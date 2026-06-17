#!/usr/bin/env python3
"""Bulk-upload local maps to the cncstats CDN.

Mirrors the in-game UploadAllMapAssetsIfMissing path (StatsUploader.cpp):
for every .map under the maps directory, compute the engine map CRC, ask
the server whether it already has that CRC, and if not POST the .map plus
any sidecars sitting next to it. cncstats keys everything by CRC, so a
client that later gets handed this CRC (e.g. via the radarvan map vote)
can pull the map with get_map_file?crc=<crc>&kind=map.

Auth key resolution order (first hit wins):
  1. --key
  2. $CNCSTATS_ZULU_CLIENT_KEY
  3. gcloud secret  cncstats_zuluclientkey  (matches scripts/docker-build.sh)
  4. build/docker/generated/CncStatsClientKey.h  (baked at configure time)

Examples:
  ./tools/upload_maps.py                  # upload ~/Maps, skipping known CRCs
  ./tools/upload_maps.py --dry-run -v     # show what would be sent
  ./tools/upload_maps.py --maps-dir /path/to/Maps --force
"""

import argparse
import os
import re
import subprocess
import sys
import urllib.error
import urllib.request

DEFAULT_MAPS_DIR = os.path.expanduser("~/Maps")
DEFAULT_CHECK_URL = "https://cncstats.computersrfun.org/map_exists"
DEFAULT_UPLOAD_URL = "https://cncstats.computersrfun.org/add_map"
DEFAULT_BUILD_TAG = "mapupload-script"

# Sidecar kind -> filename relative to the .map's directory, and the
# contents-mask bit the engine assigns it. The .map itself ("map") is
# always uploaded; sidecars only when present on disk. Bits/names mirror
# FileTransfer.cpp Get*FromMap and StatsUploader.cpp UploadAllMapAssetsIfMissing.
#   preview = "<mapstem>.tga", everything else is a fixed name in the folder.
SIDECARS = [
    ("preview", 2, None),            # <mapstem>.tga
    ("ini", 4, "map.ini"),
    ("str", 8, "map.str"),
    ("solo", 16, "solo.ini"),
    ("assets", 32, "assetusage.txt"),
    ("readme", 64, "readme.txt"),
]


def map_crc(data: bytes) -> int:
    """Engine CRC (Core/.../Common/crc.h): rotate-left, add byte, add the
    bit shifted off the top. 32-bit wraparound at every step."""
    crc = 0
    for b in data:
        hibit = 1 if (crc & 0x80000000) else 0
        crc = ((crc << 1) + b + hibit) & 0xFFFFFFFF
    return crc


def resolve_key(explicit, repo_root):
    if explicit:
        return explicit, "--key"
    env = os.environ.get("CNCSTATS_ZULU_CLIENT_KEY")
    if env:
        return env.strip(), "env CNCSTATS_ZULU_CLIENT_KEY"
    try:
        out = subprocess.run(
            ["gcloud", "secrets", "versions", "access", "latest",
             "--secret=cncstats_zuluclientkey"],
            capture_output=True, text=True, timeout=30)
        if out.returncode == 0 and out.stdout.strip():
            return out.stdout.strip(), "gcloud secret cncstats_zuluclientkey"
    except (FileNotFoundError, subprocess.TimeoutExpired):
        pass
    header = os.path.join(repo_root, "build", "docker", "generated",
                          "CncStatsClientKey.h")
    try:
        with open(header) as fh:
            m = re.search(r'#define\s+CNCSTATS_ZULU_CLIENT_KEY\s+"([^"]*)"',
                          fh.read())
            if m and m.group(1):
                return m.group(1), header
    except OSError:
        pass
    return None, None


def server_has_crc(check_url, crc, key, build_tag, timeout):
    """GET map_exists?crc=<crc>. Body is 'true'/'false'; 'false' == missing.
    Returns True when the server already has it. On any error we treat the
    map as present (return True) so a flaky check never forces a re-upload
    storm; pass --force to bypass the check entirely."""
    sep = "&" if "?" in check_url else "?"
    url = f"{check_url}{sep}crc={crc}"
    req = urllib.request.Request(url, method="GET")
    req.add_header("Authorization", f"Bearer {key}")
    req.add_header("X-Zulu-Build", build_tag)
    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            body = resp.read().decode("ascii", "replace").strip().lower()
        return body != "false"
    except urllib.error.HTTPError as e:
        print(f"    map_exists -> HTTP {e.code}; assuming present", file=sys.stderr)
        return True
    except urllib.error.URLError as e:
        print(f"    map_exists failed ({e.reason}); assuming present", file=sys.stderr)
        return True


def upload_asset(upload_url, key, build_tag, crc, map_name, kind, data, timeout):
    req = urllib.request.Request(upload_url, data=data, method="POST")
    req.add_header("Authorization", f"Bearer {key}")
    req.add_header("X-Zulu-Build", build_tag)
    req.add_header("Content-Type", "application/octet-stream")
    req.add_header("X-Game-Seed", "0")
    req.add_header("X-Map-CRC", str(crc))
    req.add_header("X-Map-Name", map_name[:255])
    req.add_header("X-Map-File", kind[:31])
    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            return True, resp.status
    except urllib.error.HTTPError as e:
        return False, e.code
    except urllib.error.URLError as e:
        return False, str(e.reason)


def sidecar_path(map_path, kind, fixed_name):
    folder = os.path.dirname(map_path)
    if kind == "preview":
        stem = os.path.splitext(os.path.basename(map_path))[0]
        return os.path.join(folder, stem + ".tga")
    return os.path.join(folder, fixed_name)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--maps-dir", default=DEFAULT_MAPS_DIR,
                    help=f"directory to scan recursively (default: {DEFAULT_MAPS_DIR})")
    ap.add_argument("--key", help="cncstats bearer key (overrides env/gcloud/header)")
    ap.add_argument("--check-url", default=DEFAULT_CHECK_URL)
    ap.add_argument("--upload-url", default=DEFAULT_UPLOAD_URL)
    ap.add_argument("--build-tag", default=DEFAULT_BUILD_TAG,
                    help="X-Zulu-Build header value (dashboard filtering)")
    ap.add_argument("--force", action="store_true",
                    help="upload even if map_exists says the server has the CRC")
    ap.add_argument("--no-sidecars", action="store_true",
                    help="upload only the .map, skip preview/ini/str/solo/assets/readme")
    ap.add_argument("--dry-run", action="store_true",
                    help="print what would be uploaded without sending")
    ap.add_argument("--timeout", type=float, default=60.0, help="per-request seconds")
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()

    repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

    if not os.path.isdir(args.maps_dir):
        print(f"error: maps dir not found: {args.maps_dir}", file=sys.stderr)
        return 2

    key, key_src = (None, None)
    if not args.dry_run or args.key:
        key, key_src = resolve_key(args.key, repo_root)
        if not key:
            print("error: no cncstats key (set --key / $CNCSTATS_ZULU_CLIENT_KEY, "
                  "configure gcloud, or build once to bake the header)",
                  file=sys.stderr)
            return 2
        if args.verbose:
            print(f"auth key from: {key_src}")

    map_files = []
    for root, _dirs, files in os.walk(args.maps_dir):
        for f in files:
            if f.lower().endswith(".map"):
                map_files.append(os.path.join(root, f))
    map_files.sort()

    if not map_files:
        print(f"no .map files under {args.maps_dir}")
        return 0

    print(f"found {len(map_files)} map(s) under {args.maps_dir}")

    n_uploaded = n_skipped = n_failed = n_assets = 0
    for map_path in map_files:
        rel = os.path.relpath(map_path, args.maps_dir)
        map_name = rel.replace(os.sep, "\\")
        try:
            with open(map_path, "rb") as fh:
                data = fh.read()
        except OSError as e:
            print(f"[skip] {rel}: {e}", file=sys.stderr)
            n_failed += 1
            continue

        crc = map_crc(data)
        print(f"[map ] {rel}  crc={crc}  ({len(data)} bytes)")

        if not args.force and not args.dry_run:
            if server_has_crc(args.check_url, crc, key, args.build_tag, args.timeout):
                if args.verbose:
                    print("    server already has this crc; skipping")
                n_skipped += 1
                continue

        if not data:
            print(f"    [skip] empty .map, nothing to upload", file=sys.stderr)
            n_skipped += 1
            continue

        # Gather assets to send: the .map plus any present sidecars. The
        # engine (uploadOneAssetIfPresent) only ever uploads assets with
        # size > 0, and the server rejects empty bodies with HTTP 400, so
        # skip zero-byte sidecars rather than letting them fail the map.
        assets = [("map", map_path, data)]
        if not args.no_sidecars:
            for kind, _bit, fixed in SIDECARS:
                p = sidecar_path(map_path, kind, fixed)
                if os.path.isfile(p) and os.path.getsize(p) > 0:
                    try:
                        with open(p, "rb") as fh:
                            assets.append((kind, p, fh.read()))
                    except OSError as e:
                        print(f"    [skip sidecar {kind}] {e}", file=sys.stderr)

        map_failed = False
        for kind, path, blob in assets:
            label = os.path.relpath(path, args.maps_dir)
            if args.dry_run:
                print(f"    would upload {kind:7} {label} ({len(blob)} bytes)")
                n_assets += 1
                continue
            ok, status = upload_asset(args.upload_url, key, args.build_tag, crc,
                                      map_name, kind, blob, args.timeout)
            if ok:
                print(f"    uploaded {kind:7} {label} -> HTTP {status}")
                n_assets += 1
            else:
                print(f"    FAILED   {kind:7} {label} -> {status}", file=sys.stderr)
                map_failed = True

        if args.dry_run:
            continue
        if map_failed:
            n_failed += 1
        else:
            n_uploaded += 1

    print("\n--- summary ---")
    if args.dry_run:
        print(f"dry run: {len(map_files)} maps, {n_assets} assets would be uploaded")
    else:
        print(f"maps uploaded: {n_uploaded}   skipped (already on server): {n_skipped}"
              f"   failed: {n_failed}")
        print(f"assets uploaded: {n_assets}")
    return 1 if n_failed else 0


if __name__ == "__main__":
    sys.exit(main())
