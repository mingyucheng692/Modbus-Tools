<div align="center">

<img src="assets/logo.svg" alt="Modbus-Tools Logo" width="120">

# Modbus-Tools
Modbus debugging and analysis desktop tool built with Qt and C++.

**English** | [简体中文](README_zh-CN.md) | [繁體中文](README_zh-TW.md)

</div>

## Overview

`Modbus-Tools` focuses on day-to-day debugging work rather than protocol marketing.
It currently provides:

- `Modbus TCP Client` for active Modbus TCP requests
- `Modbus RTU Master` for serial Modbus requests
- `Frame Analyzer` for parsing raw Modbus TCP / RTU frames
- `Link to Analyzer` for pushing live Modbus responses into the analyzer
- Generic `TCP Client / TCP Server / UDP` tooling
- A separate `Serial Debugger` page

The project is intended for debugging, validation, and lab or field troubleshooting. It is not a safety-certified product.

## Core Capabilities

- Visual request building for common Modbus read/write workflows
- Raw frame sending helpers, including `CRC16` append for RTU and `MBAP` encapsulation for TCP
- Polling workflows with response/error tracking
- Analyzer support for field breakdown, endianness handling, and scale-based value conversion
- Three UI languages: English, Simplified Chinese, and Traditional Chinese

## Platform Scope

- Source builds are supported on `Windows`, `Linux`, and `macOS`
- Release packaging is driven by CMake target `release_bundle`
- The built-in updater / auto-update path is `Windows-only` at the moment
- On non-Windows platforms, the main application still builds, but the updater target is disabled by design

## Build

### Download

- Windows users can download prebuilt packages from [Releases](https://github.com/mingyucheng692/Modbus-Tools/releases)

### Build From Source

```bash
git clone --recursive https://github.com/mingyucheng692/Modbus-Tools.git
cd Modbus-Tools
cmake -S . -B build -DMODBUS_TOOLS_BUILD_TESTS=ON
cmake --build build --target Modbus-Tools --config Release --parallel
```

### Release Bundle

```bash
cmake -S . -B build_release -DMODBUS_TOOLS_BUILD_TESTS=OFF
cmake --build build_release --target release_bundle --config Release --parallel
```

### Notes

- Toolchain: `C++20`, `Qt 6`, `CMake 3.16+`
- Optional build switches include `MODBUS_TOOLS_ENABLE_ASAN`, `MODBUS_TOOLS_ENABLE_CLANG_TIDY`, and `MODBUS_TOOLS_ENABLE_VERBOSE_RUNTIME_LOGS`
- Linux CI installs additional Qt/XCB dependencies before building

## Quality

Automated checks are part of normal development, but they are engineering safeguards, not a certification claim.

- Unit and integration tests are organized into focused targets such as `test_core_logic`, `test_qt_core`, `test_ui_widgets`, `test_integration`, `test_modbus_stability`, `test_modbus_concurrency`, and `test_modbus_fuzz`
- CI runs a cross-platform matrix on `Windows`, `Linux`, and `macOS`
- Linux CI also includes non-blocking `clang-tidy`
- The deterministic fuzz target is built and executed on Linux when ASan test runs are enabled

### Run Tests Locally

```bash
cmake -S . -B build -DMODBUS_TOOLS_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

For multi-config generators such as Visual Studio, add `--config Debug` or `--config Release` to the build command and `-C Debug` or `-C Release` to `ctest`.

## Repository Layout

```text
core/   protocol, session, transport, parser, update logic
infra/  IO, logging, platform helpers
ui/     widgets, pages, presentation logic, i18n
tests/  unit, integration, focused regression targets
app/    desktop application entry point
```

## Boundaries

- Modbus protocol support is limited to `master` workflows: `Modbus TCP master (client)` and `Modbus RTU master`
- Generic network tooling is separate from Modbus roles and supports `TCP client`, `TCP server`, and `UDP`
- Auto-update is currently available only on `Windows`
- This repository prioritizes runtime correctness, stability, and debuggability over feature breadth
- The software is provided under the `MIT` license and remains intended for development and troubleshooting use

## Contributing

Issues and pull requests are welcome.

If you contribute code, use `git commit -s` so each commit includes a DCO sign-off:

```text
Signed-off-by: Your Name <your.email@example.com>
```

## Notes

- Usage, warranty, and liability boundaries are documented in [DISCLAIMER.md](DISCLAIMER.md)
- Acknowledgments: thanks to `Qt`, `spdlog`, `fmt`, and `GoogleTest`

## License

Licensed under the [MIT License](LICENSE).
