# Contributing to Modbus-Tools

Contributions and issues are welcome. This guide details local build setups, testing procedures, CI quality gates, and architectural invariants.

---

## Toolchain Requirements

- **C++ Standard**: C++20
- **Qt**: Qt 6.6+ (`Core`, `Concurrent`, `Network`, `SerialPort`, `Widgets`, `LinguistTools`, `Test`)
- **CMake**: 3.25+
- **Compilers**: MSVC 2022 (Windows), GCC 11+ / Clang 16+ (Linux), AppleClang 15+ (macOS)
- **Code Hygiene**: `clang-format` 17+, `clang-tidy` 17+

---

## Environment Setup & Building

Clone with recursive submodules (for third-party dependencies such as `spdlog`):

```bash
git clone --recursive https://github.com/mingyucheng692/Modbus-Tools.git
cd Modbus-Tools
```

### Specifying Qt Location
If Qt is not registered in your system `PATH`, supply `CMAKE_PREFIX_PATH`:

```bash
# Windows (PowerShell)
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.6.3\msvc2022_64"

# Linux / macOS (bash)
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

Compile the deployable package target (standalone portable bundle):

```bash
cmake -S . -B build_release -DMODBUS_TOOLS_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build_release --target release_bundle --parallel
```

---

## Testing & Verification

### Running the Test Suite via CTest

Execute all registered suites with failure diagnostics:

```bash
ctest --test-dir build --output-on-failure
```

Filter specific test suites via regular expression:

```bash
ctest --test-dir build -R "ModbusCodec|CoreLogic" --output-on-failure
```

For multi-config generators (Visual Studio), pass `-C Release` or `-C Debug`:

```bash
ctest --test-dir build -C Release --output-on-failure
```

### Direct GoogleTest Invocation

For rapid test iteration, invoke test binaries directly with GoogleTest flags:

```bash
# Filter specific test fixtures or cases
./build/tests/unit/test_core_logic --gtest_filter=ModbusCodecTest.*
```

> [!NOTE]
> **Windows DLL Path**: On Windows, ensure your Qt `bin` directory (e.g., `C:\Qt\6.x.x\msvc2022_64\bin`) is included in your system `PATH` before invoking `ctest` or test binaries directly in the shell. Otherwise, test binaries may terminate with code `0xc0000135` (`STATUS_DLL_NOT_FOUND`).

---

## Static Analysis & Code Formatting

CI enforces zero-warning static analysis and strict code style gates. Verify locally before submitting:

### Formatting
Format C++ headers and sources using the repository `.clang-format`:

```bash
clang-format -i --style=file app/**/*.cpp core/**/*.{h,cpp} infra/**/*.{h,cpp} ui/**/*.{h,cpp} tests/**/*.{h,cpp}
```

### Clang-Tidy Analysis
Enable static analysis during compilation:

```bash
cmake -S . -B build_tidy -G Ninja -DMODBUS_TOOLS_ENABLE_CLANG_TIDY=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build_tidy --parallel
```

---

## Build Options Reference

| CMake Option | Default | Description |
|:---|:---:|:---|
| `MODBUS_TOOLS_BUILD_TESTS` | `OFF` | Fetch GoogleTest and compile unit/integration test suites |
| `MODBUS_TOOLS_ENABLE_ASAN` | `OFF` | Enable AddressSanitizer (MSVC `/fsanitize:address`, Clang/GCC `-fsanitize=address`) |
| `MODBUS_TOOLS_ENABLE_UBSAN` | `OFF` | Enable UndefinedBehaviorSanitizer (Clang/GCC) |
| `MODBUS_TOOLS_ENABLE_CLANG_TIDY` | `OFF` | Run `clang-tidy` checks during compilation |
| `MODBUS_TOOLS_ENABLE_VERBOSE_RUNTIME_LOGS` | `OFF` | Flush `spdlog` on `info` level (`ON`) or `warn` level (`OFF`) |
| `MODBUS_TOOLS_ENABLE_PCH` | `ON` | Enable precompiled headers for accelerated compilation |

---

## CI Architecture & Quality Gates

GitHub Actions enforces quality gates across Windows, Linux, and macOS:

- **Pull Requests**: Executes regression suites (`test_core_logic`, `test_qt_core`, `test_ui_widgets`).
- **Static Analysis**: Linux CI runs blocking `clang-tidy` with `WarningsAsErrors: '*'`.
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

## Contribution Workflow & Invariants

1. **Architecture Invariants**: Strictly follow decisions recorded in [docs/adr/README.md](docs/adr/README.md):
   - First-party code must compile with `-fno-exceptions`. Use `Result<T, E>` or `std::optional`.
   - Never access `QWidget` instances across threads; use `moveToThread` and Queued signals.
   - Core domain logic in `core/` must not depend on `QtWidgets`.
   - Writable files must resolve through `PathResolver` (exe-dir layout). Direct `applicationDirPath()` or `QStandardPaths` usage is prohibited.
2. **DCO Sign-off**: All commits must include a Developer Certificate of Origin sign-off:
   ```bash
   git commit -s -m "feat(modbus): add support for custom parity validation"
   ```
3. **Commit Messages**: Follow [Conventional Commits](https://www.conventionalcommits.org/) (e.g. `feat(scope): ...`, `fix(scope): ...`, `docs(scope): ...`).
