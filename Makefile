# Top-level convenience targets. The CMake/Ninja build is driven by
# scripts/docker-build.sh; this Makefile only wraps the steps that turn the
# build output plus the assets/ tree into a shippable installer.
#
# Common invocations:
#   make installer           build Zulu.big, build the exe, package Zulu_Setup.exe
#   make installer-release   build the installer and upload it to GCS as a
#                            versioned, publicly-downloadable object
#   make zulu-big            just (re)pack assets/ into build/installer-tmp/Zulu.big
#   make clean-installer     remove the staged tmp dir and the built setup exe

BIG          ?= big
NSIS         ?= makensis
DOCKER_BUILD ?= ./scripts/docker-build.sh
GCLOUD       ?= gcloud
GCS_BUCKET   ?= zulu-installer

ASSETS_DIR := assets
BUILD_DIR  := build/docker
TMP_DIR    := build/installer-tmp

BIG_NAME      := Zulu.big
EXE_NAME      := generalszh_zulu.exe
LAUNCHER_NAME := ZuluLauncher.exe
SOURCE_EXE      := $(BUILD_DIR)/GeneralsMD/generalszh.exe
SOURCE_LAUNCHER := $(BUILD_DIR)/launcher/ZuluLauncher.exe

NSI            := installer/Zulu.nsi
INSTALLER_OUT  := installer/Zulu_Setup.exe
LATEST_JSON    := $(TMP_DIR)/latest.json

# Single source of truth for the release version is APPVERSION inside the
# NSI script. Keep this Make-side parser tolerant of whitespace/quoting so
# the user can edit the .nsi without having to also touch the Makefile.
APPVERSION   := $(shell sed -n 's/^!define[[:space:]]\+APPVERSION[[:space:]]\+"\([^"]*\)".*/\1/p' $(NSI))
RELEASE_NAME      = Zulu_Setup_v$(APPVERSION).exe
GCS_URI           = gs://$(GCS_BUCKET)/$(RELEASE_NAME)
PUBLIC_URL        = https://storage.googleapis.com/$(GCS_BUCKET)/$(RELEASE_NAME)
LATEST_GCS_URI    = gs://$(GCS_BUCKET)/latest.json
LATEST_PUBLIC_URL = https://storage.googleapis.com/$(GCS_BUCKET)/latest.json

# Decompose APPVERSION ("major.minor.build") for the cmake build so the
# binary's TheVersion (and therefore every replay it records) reports the
# same version as the installer. Empty trailing components default to 0
# so APPVERSION="1.1" still parses as 1.1.0. Forwarded to docker-build.sh
# as ZULU_VERSION_* env vars; cmake is force-reconfigured below so the
# new values are baked into BuildVersion.h on every Make-driven build.
ZULU_VERSION_MAJOR    := $(or $(word 1,$(subst ., ,$(APPVERSION))),0)
ZULU_VERSION_MINOR    := $(or $(word 2,$(subst ., ,$(APPVERSION))),0)
ZULU_VERSION_BUILDNUM := $(or $(word 3,$(subst ., ,$(APPVERSION))),0)

TMP_BIG      := $(TMP_DIR)/$(BIG_NAME)
TMP_EXE      := $(TMP_DIR)/$(EXE_NAME)
TMP_LAUNCHER := $(TMP_DIR)/$(LAUNCHER_NAME)

ASSET_FILES := $(shell find $(ASSETS_DIR) -type f 2>/dev/null)

# Empty-archive seed bytes for `big add` to work against. The Go BIGF library
# refuses to open a missing or zero-byte file, so we hand-roll a 16-byte
# header: magic "BIGF", little-endian archive_size = 16, big-endian count = 0,
# big-endian first-data offset = 16.
EMPTY_BIG_BYTES := 'BIGF\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10'

.PHONY: installer installer-release zulu-big zulu-exe zulu-launcher clean-installer

installer: $(INSTALLER_OUT)

zulu-big: $(TMP_BIG)

zulu-exe: $(TMP_EXE)

zulu-launcher: $(TMP_LAUNCHER)

# Stage the BIG by seeding an empty archive then adding every file in
# assets/, mapping each relative path to a backslash-separated archive path
# (Data/INI/Foo.ini -> Data\INI\Foo.ini). We rebuild from scratch each time
# so removed assets do not linger inside the archive.
$(TMP_BIG): $(ASSET_FILES) | $(TMP_DIR)
	@rm -f $@
	@printf $(EMPTY_BIG_BYTES) > $@
	@for f in $(ASSET_FILES); do \
		rel=$${f#$(ASSETS_DIR)/}; \
		archive_path=$$(printf '%s' "$$rel" | tr '/' '\\'); \
		echo "  big add $$archive_path"; \
		$(BIG) add $@ "$$f" "$$archive_path" || exit $$?; \
	done

# Drive the docker build for the Zero Hour exe and copy the result into the
# tmp dir under its installer name. We always invoke docker-build.sh so the
# build system itself decides whether anything is stale; pinning to a single
# Make timestamp would lie about the underlying source tree.
#
# --cmake forces a configure pass so the ZULU_VERSION_* env vars below
# are picked up into BuildVersion.h. Without --cmake, ninja keeps the
# previous configure's BuildVersion.h and the version doesn't update.
.PHONY: docker-build-z_generals
docker-build-z_generals:
	ZULU_VERSION_MAJOR=$(ZULU_VERSION_MAJOR) \
	ZULU_VERSION_MINOR=$(ZULU_VERSION_MINOR) \
	ZULU_VERSION_BUILDNUM=$(ZULU_VERSION_BUILDNUM) \
	$(DOCKER_BUILD) --cmake --target z_generals

$(TMP_EXE): docker-build-z_generals | $(TMP_DIR)
	cp "$(SOURCE_EXE)" "$@"

# Launcher build, mirroring the game-exe rule. Same ZULU_VERSION_* env
# vars so the launcher's embedded VS_VERSION_INFO matches the game's.
.PHONY: docker-build-z_launcher
docker-build-z_launcher:
	ZULU_VERSION_MAJOR=$(ZULU_VERSION_MAJOR) \
	ZULU_VERSION_MINOR=$(ZULU_VERSION_MINOR) \
	ZULU_VERSION_BUILDNUM=$(ZULU_VERSION_BUILDNUM) \
	$(DOCKER_BUILD) --cmake --target z_launcher

$(TMP_LAUNCHER): docker-build-z_launcher | $(TMP_DIR)
	cp "$(SOURCE_LAUNCHER)" "$@"

# Hand the staged paths to NSIS via /D overrides (paths are relative to the
# .nsi file's directory, i.e. installer/). After packaging, drop the staged
# big/exe/launcher so they don't sit around taking disk; keep $(TMP_DIR)
# itself in case something else stashes things there.
$(INSTALLER_OUT): $(TMP_BIG) $(TMP_EXE) $(TMP_LAUNCHER) $(NSI)
	$(NSIS) \
		-DBIG_SOURCE="../$(TMP_BIG)" \
		-DEXE_SOURCE="../$(TMP_EXE)" \
		-DLAUNCHER_SOURCE="../$(TMP_LAUNCHER)" \
		$(NSI)
	@rm -f "$(TMP_BIG)" "$(TMP_EXE)" "$(TMP_LAUNCHER)"

$(TMP_DIR):
	@mkdir -p $@

clean-installer:
	rm -rf "$(TMP_DIR)" "$(INSTALLER_OUT)"

# Build the installer (via the regular pipeline) and publish it to GCS under
# a version-stamped object name so each release has a stable shareable URL.
# The ACL grant marks the uploaded object world-readable; if the bucket has
# uniform bucket-level access enabled the per-object grant will fail, in
# which case allUsers must be granted Storage Object Viewer at the bucket
# level for the public URL to work.
installer-release: $(INSTALLER_OUT) | $(TMP_DIR)
	@test -n "$(APPVERSION)" || { \
		echo "ERROR: could not parse APPVERSION from $(NSI)"; exit 1; }
	$(GCLOUD) storage cp "$(INSTALLER_OUT)" "$(GCS_URI)"
	@$(GCLOUD) storage objects update "$(GCS_URI)" \
		--add-acl-grant=entity=AllUsers,role=READER \
		|| echo "[note] per-object ACL grant failed; if the bucket uses uniform bucket-level access, ensure allUsers is granted Storage Object Viewer at the bucket level."
	@# Manifest the launcher reads on every cold start. Cache-Control is
	@# overridden because GCS's default for public objects (max-age=3600)
	@# would mask freshly published releases for up to an hour. The launcher
	@# additionally sets INTERNET_FLAG_RELOAD client-side as belt-and-braces.
	@SIZE=$$(stat -c%s "$(INSTALLER_OUT)"); \
	SHA=$$(sha256sum "$(INSTALLER_OUT)" | cut -d' ' -f1); \
	printf '%s\n' \
	    '{' \
	    '  "version": "$(APPVERSION)",' \
	    '  "url": "$(PUBLIC_URL)",' \
	    "  \"size\": $$SIZE," \
	    "  \"sha256\": \"$$SHA\"" \
	    '}' > "$(LATEST_JSON)"
	$(GCLOUD) storage cp "$(LATEST_JSON)" "$(LATEST_GCS_URI)"
	@$(GCLOUD) storage objects update "$(LATEST_GCS_URI)" \
		--content-type=application/json \
		--cache-control="no-cache, max-age=0" \
		|| echo "[note] could not set metadata on latest.json (uniform bucket-level access?)."
	@$(GCLOUD) storage objects update "$(LATEST_GCS_URI)" \
		--add-acl-grant=entity=AllUsers,role=READER \
		|| echo "[note] per-object ACL grant failed for latest.json; rely on bucket-level allUsers grant."
	@rm -f "$(LATEST_JSON)"
	@echo
	@echo "Uploaded installer:  $(GCS_URI)"
	@echo "Public installer URL: $(PUBLIC_URL)"
	@echo "Update manifest:     $(LATEST_GCS_URI)"
	@echo "Manifest URL:        $(LATEST_PUBLIC_URL)"
