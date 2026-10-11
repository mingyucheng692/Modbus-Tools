# Contributing to Modbus-Tools

Contributions and issues are welcome. This document details engineering workflows, local build setups, testing policies, and architectural invariants.

---

## Contribution Workflow

To maintain production stability and high code hygiene, all contributions must adhere to the following workflow:

1. **Discuss Before Implementing**: For significant architectural changes, new protocol features, or major refactorings, open a GitHub Issue first to align with maintainers before writing code.
2. **Small, Atomic Pull Requests**: Keep pull requests focused on a single logical change. Do not combine refactorings with new features or bug fixes.
3. **Commit Messages & DCO**: Follow [Conventional Commits](https://www.conventionalcommits.org/) and include a Developer Certificate of Origin sign-off (`-s`):
   ```bash
   git commit -s -m "feat(modbus): add support for custom parity validation"
   ```

---

## Toolchain Requirements

- **C++ Standard**: C++20
- **Qt**: Qt 6.6+ (`Core`, `Concurrent`, `Network`, `SerialPort`, `Widgets`, `LinguistTools`, `Test`)
- **CMake**: 3.25+
- **Compilers**: MSVC 2022 (Windows), GCC 11+ / Clang 16+ (Linux)
- **macOS**: Best-effort. AppleClang 15+ is compile-checked in CI only; no prebuilt packages or runtime validation.
- **Code Hygiene**: `clang-format` 20.1.8 (the version CI pins; output differs across major versions), `clang-tidy` 17+

---

## Environment Setup & Building

Clone with recursive submodules (for third-party dependencies such as `spdlog`):

```bash
git clone --recursive https://github.com/mingyucheng692/Modbus-Tools.git
cd Modbus-Tools

# If previously cloned without --recursive, or after pulling commits with submodule changes:
git submodule update --init --recursive
```

### Specifying Qt Location
If Qt is not registered in your system `PATH`, supply `CMAKE_PREFIX_PATH`:

```bash
# Windows (PowerShell)
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.6.3\msvc2022_64"

# Linux (bash)
export CMAKE_PREFIX_PATH=/opt/Qt/6.6.3/gcc_64
```

### Development Build (Application + Tests)

Single-config generators (`Ninja`):

```bash
cmake -S . -B build -G Ninja -DMODBUS_TOOLS_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Multi-config generators (`Visual Studio 17 2022`):

```bash
cmake -S . -B build -G "Visual Studio 17 2022" -DMODBUS_TOOLS_BUILD_TESTS=ON
cmake --build build --config Release --parallel
```

### Production Release Bundle

Single-config generators (`Ninja`):

```bash
cmake -S . -B build_release -G Ninja -DMODBUS_TOOLS_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build_release --target release_bundle --parallel
```

Multi-config generators (`Visual Studio 17 2022`):

```bash
cmake -S . -B build_release -G "Visual Studio 17 2022" -DMODBUS_TOOLS_BUILD_TESTS=OFF
cmake --build build_release --config Release --target release_bundle --parallel
```

---

## Testing & Verification

### Running the Test Suite via CTest

Execute all registered suites with failure diagnostics:

```bash
# Single-config (Ninja)
ctest --test-dir build --output-on-failure

# Multi-config (Visual Studio)
ctest --test-dir build -C Release --output-on-failure
```

Tests are registered per GoogleTest case (`Suite.Test`), so filter by suite name. List them with `-N`:

```bash
ctest --test-dir build -C Release -N
ctest --test-dir build -C Release -R "^RetryStrategy\." --output-on-failure
```

### Direct GoogleTest Invocation

Test executables (`test_core_logic`, `test_qt_core`, `test_ui_widgets`, `test_integration`, `test_modbus_fuzz`) are emitted next to the application, in `build/app/`. Invoke them directly with GoogleTest flags for rapid iteration:

```bash
# Single-config generators (Linux GCC / Windows Ninja):
./build/app/test_core_logic --gtest_filter=RetryStrategy.*

# Multi-config generators (Windows Visual Studio):
.\build\app\Release\test_core_logic.exe --gtest_filter=RetryStrategy.*
```

> [!NOTE]
> **Windows DLL Path**: On Windows, ensure your Qt `bin` directory (e.g., `C:\Qt\6.x.x\msvc2022_64\bin`) is included in your system `PATH` before invoking `ctest` or test binaries directly in the shell. Otherwise, test binaries may terminate with code `0xc0000135` (`STATUS_DLL_NOT_FOUND`).

---

## Static Analysis & Code Formatting

CI runs `clang-tidy` with warnings as errors, and checks `clang-format` on the lines a pull request changes. Verify locally before submitting.

### Formatting
`.clang-format` describes the target style. The existing tree is not uniformly formatted, so do not reformat whole files: only lines you add or modify are checked. Do not mix unrelated reformatting into a change.

```bash
# Check the lines changed relative to main (same script CI runs; requires bash):
bash scripts/check-format-diff.sh origin/main

# Reformat only your changed lines (git-clang-format ships with LLVM/clang-format packages):
git clang-format origin/main
```

### Clang-Tidy Analysis
Mirror the CI job: Linux, `RelWithDebInfo`, and only the `core`, `infra` and `ui` targets.

```bash
cmake -S . -B build_tidy -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DMODBUS_TOOLS_BUILD_TESTS=OFF -DMODBUS_TOOLS_ENABLE_CLANG_TIDY=ON
cmake --build build_tidy --target core infra ui --parallel
```

---

## Build Options Reference

| CMake Option | Default | Description |
|:---|:---:|:---|
| `MODBUS_TOOLS_BUILD_TESTS` | `OFF` | Fetch GoogleTest and compile unit/integration test suites |
| `MODBUS_TOOLS_ENABLE_ASAN` | `OFF` | Enable AddressSanitizer (MSVC `/fsanitize=address`, Clang/GCC `-fsanitize=address`) |
| `MODBUS_TOOLS_ENABLE_UBSAN` | `OFF` | Enable UndefinedBehaviorSanitizer (Clang/GCC) |
| `MODBUS_TOOLS_ENABLE_CLANG_TIDY` | `OFF` | Run `clang-tidy` checks during compilation |
| `MODBUS_TOOLS_ENABLE_VERBOSE_RUNTIME_LOGS` | `OFF` | Flush `spdlog` on `info` level (`ON`) or `warn` level (`OFF`) |
| `MODBUS_TOOLS_ENABLE_PCH` | `ON` | Enable precompiled headers for accelerated compilation |

---

## CI Architecture & Quality Gates

GitHub Actions enforces quality gates on Windows and Linux, with an additional compile check on macOS:

- **Pull Requests**: Builds the lite test targets (`test_core_logic`, `test_qt_core`, `test_ui_widgets`).
- **Formatting**: Pull requests run `clang-format` 20.1.8 on changed lines only (`scripts/check-format-diff.sh`).
- **Static Analysis**: Linux CI runs blocking `clang-tidy` with `WarningsAsErrors: '*'` on the `core`, `infra` and `ui` targets.
- **Fuzz Testing**: Deterministic parser fuzz suite (`test_modbus_fuzz`) runs under Linux ASan on manual/scheduled workflows.
- **Architectural Linting**: CI verifies that platform APIs and macros do not leak into `core/`, and verifies `forceReset()` is absent from production code.

---

## Repository Structure

```text
Modbus-Tools/
├── app/        Desktop executable entry point (main.cpp, resources)
├── core/       Domain logic: Modbus protocol stack, codecs, session, state machines, updater logic
├── infra/      Infrastructure: I/O channels (Serial/TCP/UDP), SettingsService, logger, PathResolver
├── ui/         Presentation layer: views, widgets, presenters, coordinators, i18n
├── updater/    Windows standalone native Win32 updater (zero Qt dependencies)
├── tests/      GoogleTest suites: unit, integration, mocks, fuzz targets
├── cmake/      CMake modules, compiler configuration, translation helpers
├── scripts/    Developer automation scripts
└── docs/       User guides, images, and Architecture Decision Records (ADR)
```

---

## Architectural Invariants & ADRs

All contributions must strictly uphold the invariants recorded in [docs/adr/README.md](docs/adr/README.md):

- **No Exceptions**: First-party code compiles with `-fno-exceptions`. Use `Result<T, E>` or `std::optional`.
- **UI Thread Safety**: Never access `QWidget` instances across threads; use `moveToThread` and Queued signal-slot mechanisms.
- **Domain Independence**: Core domain logic in `core/` must never include or depend on `QtWidgets`.
- **Portable Storage**: All writable paths must resolve via `PathResolver` (exe-dir portable layout). Direct `applicationDirPath()` or `QStandardPaths` usage is prohibited.

---

## Pre-Submission Checklist

Before submitting a pull request, verify that:

- [ ] All unit and regression tests pass (`ctest --test-dir build --output-on-failure`; add `-C Release` for multi-config generators).
- [ ] Changed lines are clang-format clean (`bash scripts/check-format-diff.sh origin/main`).
- [ ] Static analysis passes with zero warnings (`MODBUS_TOOLS_ENABLE_CLANG_TIDY=ON`).
- [ ] Commit messages follow Conventional Commits with DCO sign-off (`git commit -s`).
- [ ] No architectural invariants from ADRs are violated.
