#!/usr/bin/env bash
# @file deploy-linux.sh
# @brief Build the self-contained Linux tar.gz package (Qt runtime + $ORIGIN rpath).
#
# Copyright (c) 2025 - present mingyucheng692
#
# Licensed under the MIT License. See LICENSE file in the project root for full license information.
#
# Usage: deploy-linux.sh <build_dir> <source_root> <tag>
#   e.g. deploy-linux.sh build_release "$PWD" "v1.2.3"
#
# Output (written into <source_root>):
#   Modbus-Tools-v<version>-linux-x86_64.tar.gz          (naming matches
#     PlatformReleaseAssetStrategy::buildPlatformAssetName, so UpdateChecker
#     resolves fullPackageUrl with zero code changes)
#   Modbus-Tools-v<version>-linux-x86_64.tar.gz.sha256   (sha256sum -c format)
#
# Package layout (exe at prefix root, per install(TARGETS ... RUNTIME DESTINATION .)):
#   Modbus-Tools            rpath $ORIGIN/lib
#   qt.conf                 Plugins/Libraries redirection for the bundled runtime
#   lib/                    Qt libraries, rpath $ORIGIN
#   plugins/                Qt plugins (platforms/tls/...), rpath $ORIGIN/../lib
#
# System libraries (glibc, libxcb*, fontconfig, ...) are intentionally NOT
# collected: the target distro provides them (Ubuntu 22.04+ baseline).
set -euo pipefail

BUILD_DIR="${1:?usage: deploy-linux.sh <build_dir> <source_root> <tag>}"
SRC_ROOT="${2:?usage: deploy-linux.sh <build_dir> <source_root> <tag>}"
TAG="${3:?usage: deploy-linux.sh <build_dir> <source_root> <tag>}"

log() { printf '[deploy-linux] %s\n' "$*"; }
die() { printf '[deploy-linux] ERROR: %s\n' "$*" >&2; exit 1; }

for tool in patchelf ldd qmake sha256sum tar timeout; do
  command -v "$tool" >/dev/null 2>&1 || die "required tool not found in PATH: $tool"
done

# TAG arrives as "v1.2.3"; the asset name keeps the 'v' (same convention as the
# Windows ZIP: Modbus-Tools-v1.2.3-windows-x86_64.zip).
PKG_NAME="Modbus-Tools-${TAG}-linux-x86_64"
STAGE="$(mktemp -d)"
PKG="${STAGE}/${PKG_NAME}"
trap 'rm -rf "${STAGE}"' EXIT

QT_LIBS="$(qmake -query QT_INSTALL_LIBS)"       || die "qmake -query QT_INSTALL_LIBS failed"
QT_PLUGINS="$(qmake -query QT_INSTALL_PLUGINS)" || die "qmake -query QT_INSTALL_PLUGINS failed"
[ -d "$QT_LIBS" ]    || die "Qt lib dir not found: ${QT_LIBS}"
[ -d "$QT_PLUGINS" ] || die "Qt plugin dir not found: ${QT_PLUGINS}"
log "Qt libs:    ${QT_LIBS}"
log "Qt plugins: ${QT_PLUGINS}"

# Step 1: install Production component -------------------------------------
log "Step 1/6: cmake --install (component Production)"
cmake --install "$BUILD_DIR" --prefix "$PKG" --component Production
EXE="${PKG}/Modbus-Tools"
[ -f "$EXE" ] || die "installed binary not found: ${EXE}"
mkdir -p "${PKG}/lib" "${PKG}/plugins"

# Step 2: collect Qt runtime (BFS over ldd) --------------------------------
# System libraries stay on the system; only Qt-owned .so files are bundled.
declare -A COPIED=()
queue=("$EXE")

copy_qt_dep() {
  local dep="$1" name
  case "$(basename "$dep")" in
    libQt6*.so*) : ;;                       # Qt libs incl. transitive (XcbQpa, ...)
    *) [[ "$dep" == "$QT_LIBS"/* ]] || return 0 ;;
  esac
  name="$(basename "$dep")"
  [ -n "${COPIED[$name]:-}" ] && return 0
  COPIED[$name]=1
  cp -L "$dep" "${PKG}/lib/${name}"
  patchelf --set-rpath '$ORIGIN' "${PKG}/lib/${name}"
  queue+=("${PKG}/lib/${name}")            # follow transitive deps
}

log "Step 2/6: collect Qt runtime (ldd BFS)"
patchelf --set-rpath '$ORIGIN/lib' "$EXE"
while [ "${#queue[@]}" -gt 0 ]; do
  cur="${queue[0]}"
  queue=("${queue[@]:1}")
  while IFS= read -r dep; do
    [ -n "$dep" ] && copy_qt_dep "$dep"
  done < <(ldd "$cur" | awk '$3 ~ /^\// {print $3}')
done
[ "${#COPIED[@]}" -gt 0 ] || die "no Qt libraries collected - Qt prefix filter mismatch?"
log "  collected ${#COPIED[@]} Qt libraries"

# Step 3: deploy Qt plugins --------------------------------------------------
# tls/ is mandatory: QNetworkAccessManager HTTPS (update check) loads the
# OpenSSL backend from plugins/tls at runtime.
log "Step 3/6: deploy Qt plugins"
for sub in platforms imageformats iconengines platforminputcontexts platformthemes tls; do
  src_dir="${QT_PLUGINS}/${sub}"
  if [ ! -d "$src_dir" ]; then
    log "  skip ${sub} (not present in Qt installation)"
    continue
  fi
  mkdir -p "${PKG}/plugins/${sub}"
  cp -a "${src_dir}/." "${PKG}/plugins/${sub}/"
  while IFS= read -r plg; do
    while IFS= read -r dep; do
      [ -n "$dep" ] && copy_qt_dep "$dep"
    done < <(ldd "$plg" | awk '$3 ~ /^\// {print $3}')
  done < <(find "${PKG}/plugins/${sub}" -maxdepth 1 -type f -name '*.so*')
done
find "${PKG}/plugins" -type f -name '*.so*' -exec patchelf --set-rpath '$ORIGIN/../lib' {} +

# Step 4: qt.conf ------------------------------------------------------------
log "Step 4/6: write qt.conf"
cat > "${PKG}/qt.conf" <<'EOF'
[Paths]
Plugins = plugins
Libraries = lib
EOF

# Step 5: verification --------------------------------------------------------
# Fail at packaging time if anything still points into the Qt install prefix
# (would break on user machines) or if a dependency cannot be resolved.
log "Step 5/6: verify (ldd not-found / absolute-path leak)"
check_ldd() {
  local f="$1" out
  out="$(ldd "$f")" || die "ldd failed on: ${f}"
  if grep -F "not found" <<<"$out" >/dev/null; then
    die "unresolved dependency for ${f}:\n${out}"
  fi
  if grep -F "$QT_LIBS" <<<"$out" >/dev/null; then
    die "absolute Qt install path leaked into ${f}:\n${out}"
  fi
}
check_ldd "$EXE"
for f in "${PKG}"/lib/*.so* "${PKG}"/plugins/*/*.so*; do
  [ -e "$f" ] && check_ldd "$f"
done
log "  all ELF files resolve within the bundle"

# Step 6: smoke test + packaging ---------------------------------------------
# The app has no --version flag; instead launch it under offscreen for at most
# 10s: exit 0 (clean early exit) or 124 (timeout, still running = healthy) are
# both OK; loader errors / crashes (127/134/139...) fail the packaging.
log "Step 6/6: smoke test (QT_QPA_PLATFORM=offscreen, timeout 10s)"
set +e
( cd / && QT_QPA_PLATFORM=offscreen timeout 10 "$EXE" ) >/dev/null 2>&1
smoke_rc=$?
set -e
case "$smoke_rc" in
  0|124) log "  smoke test OK (exit ${smoke_rc})" ;;
  *)     die "smoke test failed (exit ${smoke_rc}) - rpath/plugin chain broken?" ;;
esac

log "Packaging tar.gz + sha256"
tar -czf "${SRC_ROOT}/${PKG_NAME}.tar.gz" -C "$STAGE" "$PKG_NAME"
( cd "$SRC_ROOT" && sha256sum "${PKG_NAME}.tar.gz" > "${PKG_NAME}.tar.gz.sha256" )
log "DONE: ${SRC_ROOT}/${PKG_NAME}.tar.gz"
log "      ${SRC_ROOT}/${PKG_NAME}.tar.gz.sha256"
