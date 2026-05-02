#!/bin/bash

# Reference:
# ----------
# 1. https://stackoverflow.com/questions/32925844/codesign-and-ambiguos-identity-matches-mac-developer-and-iphone-developer

set -e

# Logging
# -------

if [ -n "$BUILD_LOG" ]; then
	exec > >(tee -a "$BUILD_LOG") 2>&1
fi

# Utilities
# ---------

_color_supported() {
	[[ -z "$BUILD_LOG" ]] && [[ -t 1 ]] && [[ "$TERM" =~ (xterm|screen|tmux|alacritty|kitty) ]]
}

log_step() {
	if _color_supported; then
		printf "\e[1;34m▸\e[0m \e[1m%s\e[0m\n" "$1"
	else
		printf "==> %s\n" "$1"
	fi
}

log_info() {
	if _color_supported; then
		printf "  \e[90m%s\e[0m\n" "$1"
	else
		printf "    %s\n" "$1"
	fi
}

log_success() {
	if _color_supported; then
		printf "\e[1;32m✓\e[0m \e[1m%s\e[0m\n" "$1"
	else
		printf "[OK] %s\n" "$1"
	fi
}

log_error() {
	if _color_supported; then
		printf "\e[1;31m✗\e[0m \e[1m%s\e[0m\n" "$1" >&2
	else
		printf "[ERROR] %s\n" "$1" >&2
	fi
}

# Keep old names as aliases for backwards compatibility in sourced scripts
echo_header() { log_step "$1"; }
echo_success() { log_success "$1"; }

export -f echo_header
export -f echo_success

# Setup
# -----

CPU=$(sysctl -n hw.physicalcpu)

if [ -z "$CPU" ]; then
	CPU=2
fi

if [ -f build_info.config ]; then
	log_step "Loading build_info.config"
	. build_info.config
	sed -e 's/^/  /' build_info.config
fi

if [ ! -z "$V_SDK" ]; then
	export V_SDK="$V_SDK"
fi

# `START_DIR` contains the directory where the script is located
START_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GODOT_DIR=$PWD
TEMPLATES_DIR="$HOME/Library/Application Support/Godot/templates/"

if ! command -v scons &> /dev/null
then
	export PATH=$PATH:/opt/local/bin:/opt/macports/bin
fi

# Building
# --------

export SCONS_FLAGS="$SCONS_FLAGS no_editor_splash=yes CCFLAGS=-D__MACPORTS__"

if [ "$1" == "templates" ]; then
	build_templates="yes"
	shift
fi

if [ -z "$1" ]; then
	target="release_debug"
else
	target=$1
	shift
fi

if [ -z "$target" ]; then
	log_error "Missing 'target' info."
	exit 1
fi

A=$(uname -m)

if [ "${A}" = "x86_64" ]; then
	if [ "$(sysctl -in sysctl.proc_translated)" = "1" ]; then
		log_info "Running on Rosetta translation - force building native arm64"
		A="arm64"
	fi
fi

log_step "Building $target editor for macOS ($A, ${CPU} cores)"
scons -j$CPU platform=osx arch=$A target=$target $SCONS_FLAGS

if [ -x "$(command -v gcp)" ]; then
	cp="gcp -u"
else
	cp="cp"
fi

log_step "Packaging app"
rm -rf "$GODOT_DIR/bin/Godot-master.app"
$cp -rv "$GODOT_DIR/misc/dist/osx_tools.app" "$GODOT_DIR/bin/Godot-master.app"
# replace icon with master variant
sed 's/Godot\.icns/Godot MASTER.icns/' "$GODOT_DIR/misc/dist/osx_tools.app/Contents/Info.plist" > "$GODOT_DIR/bin/Godot-master.app/Contents/Info.plist"
mkdir -p "$GODOT_DIR/bin/Godot-master.app/Contents/MacOS"
$cp -v "$GODOT_DIR/bin/godot.osx.opt.tools.$A" "$GODOT_DIR/bin/Godot-master.app/Contents/MacOS/Godot"
# Bundle any runtime shared libraries staged into bin/ by modules
# (e.g. epicservices drops libEOSSDK-Mac-Shipping.dylib there). The engine
# resolves them via @executable_path / @loader_path rpaths, so
# Contents/MacOS is the right destination. codesign --deep below covers
# the bundled dylibs.
shopt -s nullglob
for dylib in "$GODOT_DIR/bin/"*.dylib; do
	$cp -v "$dylib" "$GODOT_DIR/bin/Godot-master.app/Contents/MacOS/"
done
shopt -u nullglob
if [ ! -z "$EDITOR_BUNDLE_ID" ]; then
	log_info "Bundle identifier: $EDITOR_BUNDLE_ID"
	/usr/libexec/PlistBuddy -c "Set :CFBundleIdentifier $EDITOR_BUNDLE_ID" "$GODOT_DIR/bin/Godot-master.app/Contents/Info.plist"
fi

codesign_identity="-"
if [ ! -z "$EDITOR_CODESIGN_IDENTITY" ]; then
	# Resolve the signing identity — may be a direct certificate name or an identity preference (alias)
	resolved_identity="$EDITOR_CODESIGN_IDENTITY"
	if ! security find-identity -v -p codesigning | grep -q "$EDITOR_CODESIGN_IDENTITY"; then
		# Not a direct certificate name — check if it's an identity preference
		pref_alias=$(security get-identity-preference -s "$EDITOR_CODESIGN_IDENTITY" 2>/dev/null \
			| grep '"alis"' | sed 's/.*<blob>="\(.*\)"/\1/')
		if [ -n "$pref_alias" ]; then
			log_info "Resolved identity preference '$EDITOR_CODESIGN_IDENTITY' -> '$pref_alias'"
			resolved_identity="$pref_alias"
		else
			log_error "WARNING: Signing identity '$EDITOR_CODESIGN_IDENTITY' not found as certificate or identity preference"
			log_info "Falling back to ad-hoc signature"
			resolved_identity=""
		fi
	fi

	if [ -n "$resolved_identity" ]; then
		# security find-identity -v -p codesigning only lists valid (non-expired) certificates.
		# If the resolved identity appears there, it's valid — no separate expiry check needed.
		# (Avoid find-certificate -c which can match an older expired cert with the same name.)
		if security find-identity -v -p codesigning | grep -q "$resolved_identity"; then
			codesign_identity="$EDITOR_CODESIGN_IDENTITY"
			log_info "Codesign identity: $EDITOR_CODESIGN_IDENTITY"
		else
			log_error "WARNING: Certificate '$resolved_identity' not found in valid codesigning identities"
			log_info "Falling back to ad-hoc signature"
		fi
	fi
fi
log_step "Signing executable for debugger"
codesign --force --deep --sign "$codesign_identity" --timestamp --entitlements "$GODOT_DIR/misc/dist/osx/editor.entitlements" "$GODOT_DIR/bin/Godot-master.app"

# Verify signature is valid
if ! codesign --verify --deep --strict "$GODOT_DIR/bin/Godot-master.app" 2>/dev/null; then
	verify_err=$(codesign --verify --deep --strict "$GODOT_DIR/bin/Godot-master.app" 2>&1 || true)
	if echo "$verify_err" | grep -q "CSSMERR_TP_CERT_EXPIRED"; then
		log_error "WARNING: Code signature uses an expired certificate — re-signing with ad-hoc"
		codesign --force --deep --sign - --timestamp --entitlements "$GODOT_DIR/misc/dist/osx/editor.entitlements" "$GODOT_DIR/bin/Godot-master.app"
	else
		log_error "WARNING: Code signature verification failed: $verify_err"
	fi
fi

# Add firewall exception to suppress "allow incoming connections" dialog.
# Only attempt in interactive terminals; skip on managed/MDM Macs.
fw_cmd="/usr/libexec/ApplicationFirewall/socketfilterfw"
app_path="$GODOT_DIR/bin/Godot-master.app"
if [ -x "$fw_cmd" ] && [ -t 0 ]; then
	# Check if app is already allowed (no sudo needed for --getappblocked)
	if ! "$fw_cmd" --getappblocked "$app_path" 2>/dev/null | grep -q "permitted"; then
		log_step "Add firewall exception"
		if sudo -n true 2>/dev/null; then
			sudo "$fw_cmd" --remove "$app_path" 2>/dev/null || true
			sudo "$fw_cmd" --add "$app_path" 2>/dev/null || true
			sudo "$fw_cmd" --unblockapp "$app_path" 2>/dev/null || true
		else
			log_info "Skipping firewall exception (sudo requires password — run manually if needed)"
		fi
	fi
fi

log_success "Finished building editor for macOS ($(date +'%h/%d %H:%M'))"

log_step "Running doctests"
"$GODOT_DIR/bin/godot.osx.opt.tools.$A" --doctest-run
log_success "All doctests passed"

if [ "$1" == "templates" ] || [ ! -z "$build_templates" ]; then
	log_step "Building 64-bit release export template for macOS"

	scons -j$CPU platform=osx arch=x86_64 tools=no target=release lto=full $SCONS_FLAGS
	scons -j$CPU platform=osx arch=arm64 tools=no target=release lto=full $SCONS_FLAGS
	strip "$GODOT_DIR/bin/godot.osx.opt.x86_64"
	strip "$GODOT_DIR/bin/godot.osx.opt.arm64"
	lipo -create bin/godot.osx.opt.x86_64 bin/godot.osx.opt.arm64 -output bin/godot.osx.opt.64
	mv "$GODOT_DIR/bin/godot.osx.opt.64" "$TEMPLATES_DIR"
	rm "$GODOT_DIR/bin/godot.osx.opt.x86_64" "$GODOT_DIR/bin/godot.osx.opt.arm64"

	log_success "Finished building export templates for macOS ($(date +'%h/%d %H:%M'))"
fi

# Sync project files across machines
source "${START_DIR}/build_functions.sh"
sync_extra
