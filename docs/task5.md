# Task 5: ModbusPage 状态机收口、日志/崩溃底座加固与 DRY/YAGNI 治理计划

> **目标**：基于 `ModbusPage` 生态全链路架构审计结论，遵循 **YAGNI**、**DRY** 与 **MVP Passive View** 原则，在不侵入 `NetworkDebuggerView` / `SerialDebuggerView` / `FrameAnalyzerView` / `UtilitiesView` 及跨页共享控件的前提下，分阶段完成「日志三轨分离与崩溃可观测性底座 → P0 轮询/连接/释放真实 Bug 止血 → P1 状态机单一真相与单线程释放收敛 → P2 Modbus 内部 DRY/YAGNI 瘦身 → P3 UI 解耦与旧命名清扫」闭环，为后续其他页面分批重构树立零缺陷参考样板。
> **前置基线**：Task 4（批次 1~10）完成、被动断线 UI 状态同步修复完成、`qt_safety_check.py` 0 违规、`ctest` 700/700 通过。
> **物理范围（零跨页侵入）**：`app/main.cpp`、`infra/logging/**`、`infra/platform/**`、`infra/io/ChannelBase.cpp`、`infra/config/SettingsService.cpp`、`core/Config.h`、`core/common/SettingsKeys.h`、`core/modbus/session/**`、`core/modbus/dispatch/**`、`core/polling/**`、`ui/logging/**`、`ui/application/modbus/**`、`ui/views/modbus/**`、`ui/widgets/modbus/**`、`ui/common/TrafficEvent.h`、`ui/i18n/*.ts`、`tests/**`。
> **总计**：7 个原子批次 · 24 项小粒度任务（10 `[B]` 缺陷修复 · 12 `[S]` 架构/DRY/YAGNI 收敛 · 2 `[D]` 测试/翻译/门禁闭环），每批均可独立编译并通过 `ctest` 与 `qt_safety_check.py`。

---

## 一、子域审查与收敛总表

| 子域 | 范围与重点 | `[B]` | `[S]` | `[D]` | 任务编号 | 核心收敛动作 |
| :--- | :--- | :---: | :---: | :---: | :--- | :--- |
| **1. 日志底座与崩溃黑匣子** | `infra/logging`、`infra/platform`、`app`、`ChannelBase`、`ModbusClient`、`ModbusWorker`、`RequestExecutor` | 3 | 1 | 0 | `T5-01`..`T5-04` | 同步 `FatalFlushAndAbort` 替换假异步 `flush+abort`；开启 `flush_every(2s)` + `enable_backtrace(64)`；接入 Windows SEH/minidump 与 POSIX 崩溃钩子；补齐 `[%s:%#]`、环境变量优先级、轮询入队降级与异步 `TraceContext` 修复 |
| **2. 日志三轨职能分离** | `ui/logging/LogBridge.*`、`TrafficLogController`、`ModbusPage`、`WorkerReleaseCoordinator` | 0 | 2 | 1 | `T5-05`..`T5-07` | 整体移除 `LogBridge` 切断本地化 UI 文案倒灌 `spdlog`；清空 View 层 `spdlog` 回声；确立“Presenter 意图 + Core 结果”双锚点并在故障点触发 `dump_backtrace()` |
| **3. P0 轮询与错误分类止血** | `PollingController`、`PollingStrategy`、`ModbusPagePresenter`、`ModbusWorkbenchWidget` | 4 | 0 | 0 | `T5-08`..`T5-11` | 修复 `onPollTimerTick` 阻塞自愈与重连后无 `sessionConnected`；修复轮询间隔重置为 1000ms、写轮询失效与迟到响应 `Idle->Escalated` 断言；错误分类改为强类型 `ModbusError` 修复中文 i18n 失效；修复 `isPolling_` 乐观翻转卡死 |
| **4. P0 连接/释放安全与单线程收敛** | `ModbusSessionPresenter`、`ModbusClient`、`ConnectionManager`、`StateMachineBase`、`WorkerReleaseCoordinator` | 3 | 1 | 0 | `T5-12`..`T5-15` | 修复半开踢出后 UI 卡死 `Connected` 与 `connectFinished` 竞态；修复 `~ModbusClient` 死守卫与 `tryTransition` 自迁移 CAS 漏洞；修复 `doomed.reset()` 误删与超时不 finalize；收敛假双线程为真实单线程模型 |
| **5. P1 状态机单一真相收口** | `ConnectionStateMachine`、`RequestStateMachine`、`ModbusSessionPresenter`、`ModbusPagePresenter`、`ModbusPage` | 1 | 2 | 0 | `T5-16`..`T5-18` | 建立 Core 状态变迁单向推送，消除 6 处直接覆写、`stateBeforeSync` 快照与 `disconnectIsExplicit_` 死分支；清理死态 `SessionHealth`/`TransportConnected`/`forceReset` 与礼仪性 `RequestStateMachine` 噪声；修复切模式抢跑失步 |
| **6. P2 Modbus 内部 DRY/YAGNI 瘦身** | `ModbusTypes`、`RequestSubmissionService`、`ModbusTelemetryWidget`、`ModbusWorkbenchWidget`、`SettingsKeys` | 0 | 3 | 0 | `T5-19`..`T5-21` | 消除 `SessionMode` 统一至 `ModbusMode`；删除 177 行 `buildWriteRequest`、dummy 控件 `formatInspectorDetails`、48 个僵尸配置键等全部 0 生产调用死代码；复用 `LogDedupe`/`function_traits`/`buildTcpAdu`/`escapeCsvValue`(CWE-1236) |
| **7. P3 MVP 解耦、命名清扫与门禁** | `ModbusPage`、`ModbusWorkbenchWidget`、`ModbusTelemetryWidget`、i18n 与全量单测 | 0 | 2 | 1 | `T5-22`..`T5-24` | `ModbusPage` 停止直构 `ModbusConfig` 与直写 `ISettingsService`（改走 `SettingsController`）；遥测过滤改结构化角色；清扫旧变量名与僵尸注释；补齐全量单测、翻译与门禁 |

---

## 二、执行批次与依赖流转

```text
  [批次 1: T5-01 ~ T5-04] (底座加固：崩溃捕获/同步落盘/Backtrace + TraceContext/降噪/格式修复)
           │
           ▼  (为后续全部重构提供崩溃安全网与高信噪比诊断日志)
  [批次 2: T5-05 ~ T5-07] (日志三轨分离：删除 LogBridge + 清理 View 回声 + 确立双锚点)
           │
           ▼  (UI 遥测与 spdlog 彻底解耦，消除本地化文案污染)
  [批次 3: T5-08 ~ T5-11] (P0 轮询止血：自愈修复 + 间隔/写轮询/迟到响应 + ModbusError 强类型分类 + 乐观状态修复)
           │
           ▼
  [批次 4: T5-12 ~ T5-15] (P0 连接/释放安全：半开 UI 同步 + 析构/CAS/doomed.reset 修复 + 单线程释放收敛)
           │
           ▼  (消灭所有已知现存功能与内存安全 Bug)
  [批次 5: T5-16 ~ T5-18] (P1 状态机单一真相：Core 推送驱动 + 清理 SessionHealth/TransportConnected/RequestFSM 噪声)
           │
           ▼
  [批次 6: T5-19 ~ T5-21] (P2 Modbus 内部 DRY/YAGNI：消 SessionMode + 删死代码/48 僵尸配置 + 补 CWE-1236 与内部复用)
           │
           ▼
  [批次 7: T5-22 ~ T5-24] (P3 UI 解耦与收官：ModbusPage Passive View + 结构化过滤 + 旧命名清扫 + 全量单测与门禁)
```

> **批次原子性与零跨页侵入守则**：
> 1. **严禁触碰后续待重构页面**：Task 5 全程不修改 `NetworkDebuggerView`、`SerialDebuggerView`、`FrameAnalyzerView`、`UtilitiesView`（含 `ModbusFrameBuilderWidget`、`Ieee754ConverterWidget`），不修改 `ui/widgets/*ConnectionWidget.*`、`ByteMonitorWidget.*`、`FrameDecodedTableView.*`、`AnalyzerExporter.*`、`FileExporter.*`、`ModbusDataHelper.*`。
> 2. **遵守构建拓扑与并发红线**：`#ifdef Q_OS_*` 与平台崩溃钩子仅限 `infra/platform/**`；`infra/**` 严禁反向依赖 `core/`（`Config.h`/`SettingsKeys.h` 除外）或 `ui/`；全局维持 `-fno-exceptions`。
> 3. **批次门禁**：每批完成后必须通过 `python .agents/scripts/tests/test_qt_safety_check.py`、`python .agents/scripts/qt_safety_check.py core infra ui app`、增量构建及 `ctest`。

---

## 三、任务详情清单

### 批次 1：日志底座加固与崩溃可观测性（零跨页侵入）

> **本批范围**：`infra/logging/Logger.{h,cpp}`、`infra/platform/**`、`app/main.cpp`、`infra/io/ChannelBase.cpp`、`core/modbus/session/{ModbusClient,ConnectionManager,RequestExecutor}.cpp`、`core/modbus/dispatch/ModbusWorker.cpp`。
> **验收标准**：程序崩溃或 `abort()` 时日志不丢尾部且产出平台崩溃诊断记录；稳态轮询期间 `INFO` 级别零每帧刷屏；异步超时与通道错误携带真实的非零 `trace_id`。

- [x] **T5-01 [B] 同步致命落盘 `FatalFlushAndAbort()` + 定时刷盘 + Backtrace 环形黑匣子**
  - **位置**：`infra/logging/Logger.{h,cpp}`（`L70-79`, `L205-227`）、`infra/io/ChannelBase.cpp`（`L202-210`）、`core/modbus/session/ModbusClient.cpp`（`L124-129`）
  - **实证**：`async_logger::flush()` 仅向后台队列异步投递任务即刻返回，紧随其后的 `std::abort()` 会杀掉后台线程导致临终 `CRITICAL` 日志丢失；Windows 正式版启用 `WIN32_EXECUTABLE ON`，`stderr` 无控制台输出；且未启用定时 `flush_every()`，原生崩溃会蒸发整段 `INFO` 缓冲区；`ModbusClient.cpp:128` 亦未对 `default_logger()` 判空。
  - **改造方案**：
    1. 在 `Logger.cpp` 初始化时启用 `spdlog::flush_every(std::chrono::seconds(2))` 与 `logger->enable_backtrace(64)`；对外提供 `logging::DumpBacktrace() noexcept` 供关键故障点倾倒最近 64 条 `DEBUG` 轨迹。
    2. 新增 `[[noreturn]] void logging::FatalFlushAndAbort(std::string_view message) noexcept`：先输出 `CRITICAL` 并 `dump_backtrace()`，调用 `spdlog::shutdown()` 同步排空异步队列并关闭句柄（若未初始化则兜底 `stderr`），最后执行 `std::abort()`。
    3. 将 `Logger.cpp` (`QtFatalMsg`)、`ChannelBase.cpp` (`assertOwnerThread` 跨线程析构) 与 `ModbusClient.cpp` (`~ModbusClient` UAF 守卫) 三处假 `flush()+abort()` 全部替换为 `logging::FatalFlushAndAbort()`。
  - **验证**：新增 `LoggerInitTest` 校验 `flush_every`、`DumpBacktrace` 在 `INFO` 级别下能将 `DEBUG` 环形缓冲倾倒到文件。

- [x] **T5-02 [S] 接入平台原生崩溃捕获钩子（Windows SEH + Minidump / POSIX `sigaction`）**
  - **位置**：`infra/platform/CrashHandler.h`、`infra/platform/win/Win32CrashHandler.cpp`、`infra/platform/posix/PosixCrashHandler.cpp`、`infra/platform/CMakeLists.txt`、`app/main.cpp`
  - **实证**：全仓无任何 OS 崩溃处理与 `std::set_terminate` 注册，遇到空指针/越界/第三方库 `-fno-exceptions` 触发 terminate 时进程静默闪退，现场零线索。
  - **改造方案**：
    1. 在 `infra/platform/` 新增轻量 `CrashHandler::install(const QString& logDir)`（`#ifdef Q_OS_*` 严格限定在 `infra/platform/**`），预分配静态路径缓冲（崩溃回调内零堆分配、零 `QString`、零非信号安全调用）。
    2. **Windows**：注册 `SetUnhandledExceptionFilter` + `_set_invalid_parameter_handler` + `_set_purecall_handler` + `std::set_terminate`；崩溃时调用 `DbgHelp.dll` 的 `MiniDumpWriteDump`（`MiniDumpNormal`）在 `<logDir>` 生成 `crash_YYYYMMDD-HHMMSS_pid.dmp`，并用原始 `CreateFileW`/`WriteFile` 同步追加异常码、异常地址、模块版本与线程 ID 到 `crash_*.txt`。
    3. **Linux/macOS**：注册 `sigaction`（`SIGSEGV`/`SIGABRT`/`SIGFPE`/`SIGILL`/`SIGBUS`），使用 async-signal-safe `write()` + `backtrace()`/`backtrace_symbols_fd()` 输出崩溃摘要与堆栈帧至 `<logDir>/crash_*.txt` 后恢复默认信号重发。
    4. 在 `app/main.cpp` 完成 `logging::Init()` 后注册 `CrashHandler::install(pathResolver.resolveLogDir())`。
  - **验证**：构建验证 Windows/POSIX 链接通过，单测验证 `CrashHandler` 路径缓冲初始化与幂等安装。

- [x] **T5-03 [B] 修复日志格式丢行号、环境变量优先级倒置与轮询入队 `INFO` 洪泛**
  - **位置**：`infra/logging/Logger.{h,cpp}`（`L144-159`, `L211-213`）、`app/main.cpp`（`L48-51`）、`core/modbus/dispatch/ModbusWorker.cpp`（`L109`）
  - **实证**：`Logger.cpp:212` 格式串缺 `[%s:%#]` 导致所有 `SPDLOG_*` 宏采集的文件名/行号被丢弃；`main.cpp:49-51` 用 INI `savedLogLevel` 无条件覆盖 `MODBUS_TOOLS_LOG_LEVEL` 环境变量；`ModbusWorker.cpp:109` 每帧入队打 `SPDLOG_INFO`，100ms 轮询每小时刷 36,000 条日志架空 `WorkerLogDemotion`；`Logger.h:12` 注释与 `CMakeLists.txt:34` 实际宏值矛盾。
  - **改造方案**：
    1. `Logger.cpp:212` 改为 `"%Y-%m-%d %H:%M:%S.%eZ [%t] [%^%l%$] [%s:%#] %v"`；同步修正 `Logger.h:9-15` 注释与 `CMakeLists.txt` 保持一致。
    2. 在 `Logger` 中提供 `HasEnvLogLevelOverride()`，`main.cpp` 仅在未设置 `MODBUS_TOOLS_LOG_LEVEL` 环境变量时应用 INI 中的 `savedLogLevel`。
    3. 将 `ModbusWorker.cpp:109` 的 `SPDLOG_INFO("ModbusWorker: enqueue request...")` 降级为 `SPDLOG_DEBUG`。
  - **验证**：`LoggerInitTest` 断言输出日志包含 `[filename:line]` 且环境变量优先于 INI；`WorkerLogDemotionTest` 验证稳态轮询 `INFO` 级别零输出。

- [x] **T5-04 [B] 修复异步 `TraceContext` 恒为 0 与旧日志前缀/串口地址误打**
  - **位置**：`core/modbus/session/RequestExecutor.cpp`（`L24-43`, `L233`, `L383`, `L391`, `L442`, `L511`, `L605-623`, `L704-748`）、`core/modbus/session/ConnectionManager.cpp`（`L47-52`, `L228-237`）
  - **实证**：`ModbusWorker::handleSubmit()` 栈上的 `trace::Scope` 在异步请求发起后即析构清零，`RequestExecutor` 虽在 `L233` 保存了 `activeTraceId_ = traceId` 却从未读取，导致所有异步超时/收包/重试回调中 `currentTraceRequired()` 恒为 0 并误报 `warnIfTraceLost()`；此外 `RequestExecutor.cpp:391,442` 与 `ConnectionManager.cpp:228,233` 仍印着旧前缀 `"ModbusClient:"`，`ConnectionManager.cpp:49` 在串口半开超时时误打 `ipAddress:port`。
  - **改造方案**：
    1. 在 `RequestExecutor` 的所有异步入口（`onDataReceived`、`onTimeout`、`onRetryTimerTimeout`、`onChannelError`、`onWriteDrained`、`abort`）入口处构造 `trace::Scope scope(activeTraceId_)`，使当前执行器及下层 `StateMachineBase`/`Channel` 均能读到真实 `activeTraceId_`；在 `completeTransaction()` / `resetState()` 清零 `activeTraceId_`。
    2. 修正 `RequestExecutor.cpp` 与 `ConnectionManager.cpp` 的旧 `"ModbusClient:"` 前缀为真实类名；`ConnectionManager::onRequestTimeout` 按 `channel_->kind()` 区分打印 `port=` 或 `target=ip:port`。
  - **验证**：扩展 `TraceContextTest` / `ModbusClientBoundaryTest` 验证异步超时日志中 `trace_id` 非零且不再误触发 `trace context lost`。

---

### 批次 2：日志三轨职能分离（移除 `LogBridge`，净化 UI/spdlog 边界）

> **本批范围**：`ui/logging/LogBridge.{h,cpp}`、`ui/CMakeLists.txt`、`ui/application/modbus/{TrafficLogController,PollingController,ModbusSessionPresenter,WorkerReleaseCoordinator}.*`、`ui/views/modbus/ModbusPage.cpp`。
> **验收标准**：`spdlog` 不再出现任何 `tr()` 中文/英文 UI 展示文案；一次 Connect/Disconnect 在 `spdlog` 中从 8~15 条收敛至 2~3 条清晰英文结构化记录；`ModbusPage.cpp` 零 `<spdlog/spdlog.h>` 依赖。

- [x] **T5-05 [S] 移除 `ui/logging/LogBridge.{h,cpp}`，切断 UI 遥测向 `spdlog` 回灌**
  - **位置**：`ui/logging/LogBridge.{h,cpp}`、`ui/CMakeLists.txt`（`L111-112`）、`ui/application/modbus/TrafficLogController.cpp`（`L6`, `L35-38`）、`ui/application/modbus/PollingController.cpp`（`L232-235`, `L254-256`）
  - **实证**：`TrafficLogController::publishEvent` 通过 `LogBridge::relay` 将带翻译的 UI 展示文本重复写入 `spdlog`，导致双倍记账、中英文混杂与热路径开销；而底层 `ConnectionManager`/`ModbusClient`/`Channel` 已记录所有连接与错误诊断信息。
  - **改造方案**：
    1. 删除 `ui/logging/LogBridge.h` 与 `ui/logging/LogBridge.cpp`，从 `ui/CMakeLists.txt` 移除；清理 `Logger.h:18` 过期注释。
    2. `TrafficLogController::publishEvent()` 仅 `emit trafficEvent(event)`，专注面向操作员的 UI 遥测流；清理 `PollingController.cpp` 中关于 `LogBridge` 的过期注释。
    3. 为弥补原 `LogBridge` 覆盖的“用户主动发送原始报文”审计点，在 `ModbusPagePresenter::handleRawExecute` 用 1 条 `SPDLOG_INFO("ModbusPagePresenter: sendRaw len={}", ...)` 记录。
  - **验证**：`TrafficLogControllerTest` 全绿，全仓 `git grep "LogBridge"` 0 残留。

- [x] **T5-06 [S] 消除 View 层 `spdlog` 回声并确立双锚点 + 故障 Backtrace 倾倒**
  - **位置**：`ui/views/modbus/ModbusPage.cpp`（`L19`, `L368`, `L383`, `L402`）、`ui/application/modbus/WorkerReleaseCoordinator.cpp`（`L136`, `L150`, `L284`, `L532`）、`ui/application/modbus/ModbusSessionPresenter.cpp`（`L166`, `L368`, `L580`）
  - **实证**：点击 Connect/Disconnect 在 `ModbusPage`、`ModbusSessionPresenter`、`WorkerReleaseCoordinator`、`ModbusWorker`、`ModbusClient` 五层反复打重复 `INFO`（高达 15 条）。
  - **改造方案**：
    1. 移除 `ModbusPage.cpp` 中的 `<spdlog/spdlog.h>` 及 3 处 `SPDLOG_INFO`（意图记录已由 `ModbusSessionPresenter` 唯一承担）。
    2. 将 `WorkerReleaseCoordinator` 正常释放过程的 4 条 `SPDLOG_INFO` 降级为 `SPDLOG_DEBUG`，仅超时/卡死/异常保留 `WARN`/`ERROR`/`CRITICAL`。
    3. 在 `ModbusSessionPresenter` 严重异常点（`onReleaseTimedOut`、`handlePollingFatalDisconnect`、非预期断线）调用 `logging::DumpBacktrace()` 倾倒事故前 64 条 `DEBUG` 轨迹。
  - **验证**：`qt_safety_check.py` 与 `test_qt_core`、`test_ui_widgets` 全绿。

- [x] **T5-07 [D] 同步日志测试与契约文档**
  - **位置**：`tests/unit/modbus/application/TrafficLogControllerTest.cpp`、`tests/unit/logging/LoggerInitTest.cpp`、`tests/unit/modbus/dispatch/WorkerLogDemotionTest.cpp`
  - **改造方案**：补齐无 `LogBridge` 下的 `TrafficLogController` 单测、新增 `DumpBacktrace` 与环境变量优先级单测。
  - **验证**：`ctest -C Release -R "Logger|TraceContext|WorkerLogDemotion|TrafficLog"` 100% 通过。

---

### 批次 3：P0 轮询与错误分类真实 Bug 止血（零跨页侵入）

> **本批范围**：`core/polling/PollingStrategy.{h,cpp}`、`ui/application/modbus/{PollingController,ModbusPagePresenter,ModbusSessionPresenter}.{h,cpp}`、`ui/widgets/modbus/ModbusWorkbenchWidget.{h,cpp}`。
> **验收标准**：被动断线后轮询能真实触发自愈与重连回显，超时耗尽后能触发 `pollingFatalDisconnect`；自定义轮询间隔生效；写轮询行为自洽；中文界面下超时/忙碌错误分类准确；取消/拒绝轮询不会卡死按钮或触发断言。

- [x] **T5-08 [B] 修复被动断线后轮询自愈失效与重连后 UI 永久停留在 Disconnected**
  - **位置**：`ui/application/modbus/PollingController.cpp`（`L78-90`, `L108-114`）、`ui/application/modbus/ModbusSessionPresenter.cpp`（`L545-549`）
  - **实证**：`handleTransientDisconnect()` 置 `sessionConnected = false` 后，生产定时器回调 `onPollTimerTick()` 第 111 行 `if (!context_.sessionConnected) return;` 直接拦截全部轮询，导致 `ensureConnected()` 永远不被调用、`pollingFatalDisconnect` 永远不触发（单测漏检是因为单测调用了 0 生产调用的 `handlePollRequest()`）；且即便底层自愈成功，`handleChannelStateTransition(Open)` 也未发射 `sessionConnected()`。
  - **改造方案**：
    1. 将 `onPollTimerTick()` 的守卫修正为允许非 `Idle` 状态在瞬态断线窗口内继续派发请求驱动 `ensureConnected()`（同时删除仅单测调用的重复入口 `handlePollRequest()`，让单测直接测试真实生产路径 `startPolling()` + `onPollTimerTick()`）。
    2. 在 `ModbusSessionPresenter::handleChannelStateTransition(Open)` 与 `handleRequestFinished()` 检测到由 `Disconnected` 恢复为 `Connected` 时，发射 `sessionConnected()`，令 `ModbusPagePresenter` 与 `PollingController` 同步切回已连接状态。
  - **验证**：在 `PollingControllerTest` 与 `ModbusSessionPresenterTest` 新增被动断线 → 定时自愈重连 / 超时触发 `pollingFatalDisconnect` 双路径测试。

- [x] **T5-09 [B] 修复轮询启动重置间隔为 1000ms、写轮询失效与迟到响应触发 `Idle -> Escalated` 断言**
  - **位置**：`ui/application/modbus/ModbusPagePresenter.cpp`（`L372-398`, `L494-497`）、`ui/application/modbus/PollingController.{h,cpp}`（`L92-140`, `L241-243`, `L348-359`）、`ui/application/modbus/ModbusExecuteSpec.h`
  - **实证**：① `togglePolling()` 构造 `PollSpec` 未复制 `intervalMs`，`startPolling()` 立即以默认 `1000ms` 覆盖用户已选间隔；② `PollSpec` 丢弃 `writeData` 且 `buildAndSubmit()` 硬编码调 `buildReadRequest()`，导致确认后的写轮询必败；③ `stopPoll()` 回到 `Idle` 后，在途轮询请求迟到失败直接触发 `Idle -> Escalated` 断言崩溃。
  - **改造方案**：
    1. `PollSpec` 显式携带 `intervalMs`（由 `ModbusWorkbenchWidget::currentSpec()` 填入或沿用 `PollingController` 当前间隔，不以默认值覆写）。
    2. **写轮询语义闭环**（执行前确认二选一，默认按方案 A）：
       - *方案 A（补全写轮询）*：`PollSpec` 携带 `writeData`，`PollingController::buildAndSubmit()` 统一通过 `requestService_->buildExecuteRequest(..., RequestKind::Poll)` 构建读/写 PDU；
       - *方案 B（YAGNI 禁写轮询）*：在 `ModbusWorkbenchWidget` 与 `ModbusPagePresenter` 明确对写功能码禁用轮询，移除 `confirmWritePoll`、`userConfirmedWritePoll_`、`writePollWarningBadge_`。
    3. 在 `PollingController::handleResponse()` 入口加守卫：若 `context_.state == PollState::Idle`（已停止轮询后的迟到响应），直接丢弃并 `return`，杜绝 `Idle -> Escalated` 非法迁移。
  - **验证**：新增单测验证自定义 `250ms` 间隔启动轮询不被重置、写轮询（或禁写策略）行为自洽、`stopPoll()` 后注入失败响应不崩溃且保持 `Idle`。

- [x] **T5-10 [B] 修复 `PollingStrategy::classifyError()` 英文子串匹配导致的 i18n 失效与重复窗口判定**
  - **位置**：`core/polling/PollingStrategy.{h,cpp}`、`ui/application/modbus/PollingController.{h,cpp}`（`L180-202`）、`ui/application/modbus/ModbusPagePresenter.cpp`（`L491-497`）
  - **实证**：`ModbusPagePresenter` 将强类型 `response.errorCode` 翻译为中文/英文 `QString` 后传给 `PollingController`，`PollingStrategy::classifyError` 再做 `"timeout"`/`"busy"` 英文子串匹配——中文界面 100% 失效，英文 `"Request already in progress"` 也匹配不到 `"busy"`；`3000ms` 升级窗口在两处重复计算；`ErrorSeverity::Fatal` 与 `PollingDecision::Recover` 均为死枚举。
  - **改造方案**：
    1. `PollingController::handleResponse()` 与 `PollingStrategy::classifyError()` 直接接收强类型 `::modbus::base::ModbusError`（`Timeout -> Timeout`，`Busy -> Busy`，其余 `Normal`）；展示文本 `localizedError` 仅用于构造 `TrafficEvent::summary`。
    2. 消除 `PollingController.cpp:190-192` 的重复 `zeroSuccessWindowExceeded` 与阈值二次钳制，统一收口到 `PollingStrategy::evaluateState()`；删除死枚举 `ErrorSeverity::Fatal`、`PollingDecision::Recover` 与未用信号 `PollingController::stateChanged`。
  - **验证**：`PollingStrategyTest` 与 `PollingControllerTest` 在 `zh_CN` 译文下验证超时/忙碌仍能正确按预期阈值升降级。

- [x] **T5-11 [B] 修复 `ModbusWorkbenchWidget::isPolling_` 乐观翻转在 Presenter 拒绝时卡死**
  - **位置**：`ui/widgets/modbus/ModbusWorkbenchWidget.cpp`（`L803-821`, `L88-103`）、`ui/application/modbus/ModbusPagePresenter.cpp`（`L270-282`, `L379-387`）
  - **实证**：`onPollCheckBoxToggled(true)` 先把 `isPolling_ = true` 改按钮为“停止轮询”，再发射 `pollToggleRequested`；若 Presenter 拒绝启动，`syncWidgetGuards` 因缓存的 `(state, lastPollingActive_=false)` 未变化而早退，控件永久停留在假轮询态。
  - **改造方案**：移除 `onPollCheckBoxToggled(true)` 中的乐观状态修改，仅发射 `pollToggleRequested(true, ...)`，由 `ModbusPagePresenter::syncWidgetGuards`（经 `setExecutionGuards`）单向驱动 `isPolling_`、`pollCheckBox_` 与按钮外观；拒绝时明确推送回滚或通过非缓存路径同步视图。
  - **验证**：`ModbusWorkbenchWidgetTest` + `ConnectionStateGuardingTest` 验证未连接或校验失败时勾选轮询不会使按钮卡死在“Stop Polling”。

---

### 批次 4：P0 核心连接/释放安全 Bug 修复与单线程模型收敛（零跨页侵入）

> **本批范围**：`core/modbus/session/{StateMachineBase.h,ConnectionManager.cpp,ModbusClient.cpp}`、`ui/application/modbus/{ModbusSessionPresenter,ModbusPagePresenter,WorkerReleaseCoordinator}.{h,cpp}`。
> **验收标准**：半开连接 3 次超时踢出后 UI 必切回 `Disconnected` 并弹出提示；析构守卫与超时泄漏逻辑真实有效；彻底消除假双线程架构，复用 `ThreadGuard`。

- [x] **T5-12 [B] 修复 TCP 半开踢出 UI 永久卡死 `Connected` 与 `handleConnectFinished(true)` 竞态**
  - **位置**：`ui/application/modbus/ModbusSessionPresenter.cpp`（`L600-640`）、`ui/application/modbus/ModbusPagePresenter.cpp`（`L75-84`）
  - **实证**：① 第 3 次超时触发 `ConnectionManager::onRequestTimeout()` 将 Core FSM 置为 `Failed` 后，`requestFinished` 先于 TCP 2s linger `Closed` 到达 GUI 线程，`handleRequestFinished()` 调用 `syncStateFromCore()` 静默将 `connectionState_` 改为 `Disconnected` 却不发任何信号，随后 `ChannelState::Closed` 到达时看到 `stateBeforeSync == Disconnected` 直接吞掉事件；② `handleConnectFinished(true)` 未校验 `connectionState_ == Connected` 就硬发 `sessionConnected` 与 `connectFinished(true)`。
  - **改造方案**：
    1. 移除 `handleRequestFinished()` 中遗留的 `syncStateFromCore()` 调用（该注释所称 `SessionHealth` 已无效），或将所有 `syncStateFromCore()` 状态变化统一路由到状态变迁处理器（配合批次 5 `T5-16` 彻底根治）。
    2. `handleConnectFinished(true)` 在 `syncStateFromCore()` 后校验 `connectionState_ == SessionConnectionState::Connected`，若已因并发断线落入 `Disconnected`，则按断线路径通知 UI 而非强行渲染 `Connected`。
    3. `ModbusPagePresenter::wireConnections()` 不再把 `connectFinished(true)` 和 `sessionConnected` 重复绑定到同一个 `syncWidgetGuards(Connected)`。
  - **验证**：新增模拟“第 3 次超时 → `requestFinished` 先于 `ChannelState::Closed` 到达”顺序的单测，断言 UI 必切回 `Disconnected` 且弹出断线告警。

- [x] **T5-13 [B] 修复 `~ModbusClient` 死守卫与 `StateMachineBase` 自迁移 CAS 漏洞**
  - **位置**：`core/modbus/session/ModbusClient.cpp`（`L105-130`）、`core/modbus/session/StateMachineBase.h`（`L32-64`）、`core/modbus/session/ConnectionManager.cpp`（`L53-59`, `L168-171`, `L209`）
  - **实证**：① `ModbusClient::~ModbusClient()` 在 `L106` 先调 `abort()` 把 `Sending`/`Waiting` 改成 `Aborted`，使 `L114` 的 UAF 守卫永远不可达；② `StateMachineBase::tryTransition` 对 `oldState == newState` 返回 `true`，导致 `ConnectionManager::onRequestTimeout` 用 `!transitionChecked(Failed)` 判并发胜负失效；③ `ConnectionManager::ensureConnected()` 在 `Disconnecting` 时返回 `true` 且 `L209` 忽略 `transitionChecked(Connected)` 失败返回值。
  - **改造方案**：
    1. `ModbusClient::~ModbusClient()` 将 `reqState == Sending || Waiting` 守卫移到 `abort()` **之前**执行（若线程已正确 join 则不会触发，若真有在途请求则经 `FatalFlushAndAbort` 立即暴露 UAF）。
    2. `StateMachineBase` 新增精确 CAS 接口 `tryTransitionFrom(expected, target, reason)`，修复 `ConnectionManager::onRequestTimeout()` 严格执行 `Connected -> Failed` CAS。
    3. `ConnectionManager::ensureConnected()` 在 `Disconnecting` 时返回 `false`（附错误 `"Disconnecting"`），并检查 `L209` `transitionChecked(Connected)` 返回值，失败时关闭通道并返回 `false`。
  - **验证**：在 `ModbusStateTest` 与 `ModbusClientBoundaryTest` 新增并发 `Connected -> Failed` CAS 与 `Disconnecting` 拒绝测试。

- [x] **T5-14 [B] 修复 `WorkerReleaseCoordinator` 的 `doomed.reset()` 误删与超时永不 finalize 缺陷**
  - **位置**：`ui/application/modbus/WorkerReleaseCoordinator.cpp`（`L266-275`, `L318-326`, `L431-443`, `L518-526`）
  - **实证**：`auto doomed = std::move(pending->channel); ... doomed.reset();` 会立即触发 `doomed` 的原删除器跨线程销毁 channel 并留下悬垂指针；且线程卡死进入 `TimedOut` 后 `tryComplete()` 仍要求 `!isRunning()`，导致 `pending_` 永不释放、永久阻塞后续连接。
  - **改造方案**：
    1. 真正实现安全刻意泄漏：将无法关闭的 `shared_ptr<IChannel>` 移入防析构泄漏容器（或 `new std::shared_ptr<io::IChannel>(std::move(pending->channel))`）阻止其引用计数归零，绝不构造悬垂别名后 `reset()`。
    2. 超时路径（`onTimeout()`）在确认线程无法按时退出后，解绑信号、将卡死对象隔离泄漏、清空 `pending_` 并发射 `releaseTimedOut`，恢复 Presenter 可用性。
  - **验证**：`WorkerReleaseCoordinatorTest` 新增“未关闭通道安全泄漏不触发跨线程析构”与“超时后 `hasPending() == false`”单测。

- [x] **T5-15 [S] 收敛假双线程为真实单线程模型并复用 `ThreadGuard`**
  - **位置**：`ui/application/modbus/ModbusSessionPresenter.{h,cpp}`（`L139-140`, `L190`, `L221-222`, `L414-415`）、`ui/application/modbus/WorkerReleaseCoordinator.{h,cpp}`（`L29-30`, `L68-72`, `L98-105`, `L167-183`）
  - **实证**：`ModbusFactory::createStack()` 只创建 1 个 `stack.thread`，`ModbusSessionPresenter` 却复制为 `channelThread_` 和 `modbusWorkerThread_` 两个成员，`WorkerReleaseCoordinator`（536 行）据此对同一个 `QThread` 双重连接 `finished`、双重调用 `requestInterruption()`，并手写重复的 `stopThread`/`releaseChannelSafely`。
  - **改造方案**：
    1. `ModbusSessionPresenter` 与 `StackHandle` 统一为单一 `ioThread_` / `StackHandle::thread`，删除 `channelThread_`、`modbusWorkerThread_`、`sharesChannelThread()` 与重复连接逻辑。
    2. 精简 `WorkerReleaseCoordinator` 的底层线程退出与通道释放步骤，直接组合 `core::common::ThreadGuard`，删除无用测试专口外的死阶段 `JoiningThreads` 冗余路径。
  - **验证**：`WorkerReleaseCoordinatorTest` 与 `ModbusSessionPresenterTest` 全绿，且 `WorkerReleaseCoordinator.cpp` 代码量显著瘦身。

---

### 批次 5：P1 状态机单一真相收口与死状态裁剪（零跨页侵入）

> **本批范围**：`core/modbus/session/{StateMachineBase.h,ConnectionStateMachine.h,RequestStateMachine.h,SessionTypes.h,ModbusClient.{h,cpp},RequestExecutor.{h,cpp}}`、`ui/application/modbus/{ModbusTypes.h,ModbusSessionPresenter.{h,cpp},ModbusPagePresenter.{h,cpp}}`、`ui/views/modbus/ModbusPage.{h,cpp}`。
> **验收标准**：`ModbusSessionPresenter` 不再存在绕过单一真相的影子状态分叉；死状态 `SessionHealth`、`TransportConnected`、`forceReset` 彻底清除；协议切换与延迟重连状态严格同步。

- [x] **T5-16 [S] 建立 Core 状态变迁单向驱动，消除 6 处直接覆写与 `disconnectIsExplicit_` 死分支**
  - **位置**：`ui/application/modbus/ModbusSessionPresenter.{h,cpp}`（`L51-206`, `L335-353`, `L494-598`）、`ui/application/modbus/PollingController.cpp`（`L361-366`）
  - **实证**：`ModbusSessionPresenter::connectionState_` 在 6 处被直接赋值，依赖 `stateBeforeSync` 竞态快照做边沿检测；`requestRelease()` 不发 `sessionDisconnected` 导致 `PollingController::context_.sessionConnected` 残留 `true`，且使 `L583-588` 的 `if (disconnectIsExplicit_)` 成为死代码。
  - **改造方案**：
    1. 收拢 `ModbusSessionPresenter` 的状态迁移为唯一私有入口 `setUiConnectionState(SessionConnectionState next, DisconnectKind kind, const QString& reason)`，统一负责边沿检测、日志与信号发射（`sessionConnected` / `sessionDisconnected` / `sessionTransientDisconnect` / `unexpectedDisconnectAlertRequested`）。
    2. `requestRelease()` 显式走 `DisconnectKind::Explicit` 路径，确保 `PollingController::reset()` 同步重置 `context_.sessionConnected = false`；移除死分支 `disconnectIsExplicit_` 与无用 `onConnectionStateChanged()`。
  - **验证**：`ModbusSessionPresenterTest` 与 `ConnectionStateGuardingTest` 覆盖显式断开、被动断线、连接失败、延迟重连全部边沿。

- [x] **T5-17 [S] 裁剪死状态 `SessionHealth` / `TransportConnected` / `forceReset` 与礼仪性 `RequestStateMachine` 噪声**
  - **位置**：`core/modbus/session/{SessionTypes.h,ModbusClient.{h,cpp},StateMachineBase.h,RequestStateMachine.h,RequestExecutor.{h,cpp}}`、`ui/application/modbus/{ModbusTypes.h,ModbusSessionPresenter.{h,cpp},ModbusPagePresenter.cpp}`
  - **实证**：`SessionHealth` 100% 未被读取；`SessionConnectionState::TransportConnected` 在 `ModbusPage` 中回退为 `Connecting` 且对应矛盾的“写锁定”Tooltip；`StateMachineBase::forceReset()` 全仓 0 调用；`RequestStateMachine` 0 生产读取，且 `Sending` 倒置与 `Completed -> Failed` 制造伪 `SPDLOG_ERROR`。
  - **改造方案**：
    1. 删除 `SessionHealth`、`StateMachineBase::forceReset()` 与 `SessionConnectionState::TransportConnected`（注意：**不改动**共享控件 `BaseConnectionWidget::DisplayState`，零侵入其他页面）；清理 `ModbusPagePresenter.cpp:288,297-300` 的死分支与矛盾 Tooltip。
    2. 收敛 `RequestExecutor`：修正 `Sending` 在 `channel_->write()` 之前进入、`executeAsync()` 入口先归位 `Idle` 再执行校验/建连以消除 `Completed -> Failed` 伪错；清理死成员 `connStateMachine_`、`responseReady_`、`pendingRequests_`、`kDupeTrackerCleanupThreshold`。
  - **验证**：`ModbusStateTest`、`ModbusSessionPresenterTest`、`ConnectionStateGuardingTest` 全绿，且无任何伪 `invalid transition` 错误日志。

- [x] **T5-18 [B] 修复 `ModbusPage::currentMode_` 抢跑失步与延迟重连不显示 `Connecting`**
  - **位置**：`ui/views/modbus/ModbusPage.cpp`（`L335-356`）、`ui/application/modbus/ModbusPagePresenter.cpp`（`L162-176`, `L225-226`）、`ui/application/modbus/ModbusSessionPresenter.cpp`（`L72-86`, `L379-386`）
  - **实证**：`ModbusPage::switchToProtocol()` 在异步释放完成前就修改了 `currentMode_` 与 `connectionStack_`，一旦超时回滚将与 Presenter 永久失步；`requestConnect()` 触发 `deferredAction_` 时，后续 `startConnect()` 切入 `Connecting` 未通知 `ModbusPagePresenter`，导致 UI 跳过 `Connecting`。
  - **改造方案**：
    1. `ModbusPage::switchToProtocol()` 仅锁定下拉框并向 Presenter 发起 `switchMode(mode)`，等待 Presenter 释放完成并回调 `setProtocolType()` 后再同步 `currentMode_` 与切换 `connectionStack_`（超时回滚时恢复下拉框选中项）。
    2. `ModbusSessionPresenter` 进入 `Connecting` 时统一经状态变更信号通知 `ModbusPagePresenter::syncWidgetGuards(Connecting)`，覆盖延迟重连路径。
  - **验证**：`ModbusPagePresenterTest` 与 `ConnectionStateGuardingTest` 覆盖模式切换超时回滚与延迟重连 `Connecting` 渲染。

---

### 批次 6：P2 Modbus 内部 DRY 收敛与 YAGNI 瘦身（零跨页侵入）

> **本批范围**：`core/Config.h`、`core/common/SettingsKeys.h`、`infra/config/SettingsService.cpp`、`core/modbus/dispatch/ModbusWorker.{h,cpp}`、`core/modbus/session/{ModbusClient,RequestExecutor}.{h,cpp}`、`ui/common/TrafficEvent.h`、`ui/application/modbus/*`、`ui/widgets/modbus/*`。
> **验收标准**：Modbus 专属死代码与 48 个僵尸配置键清零；`SessionMode` 消除；`ModbusTelemetryWidget` CSV 导出具备 CWE-1236 公式注入防护且未修改 `AnalyzerExporter`。

- [x] **T5-19 [S] 消除 `SessionMode` 冗余枚举并合并异常码/功能码名称复用**
  - **位置**：`ui/application/modbus/{ModbusTypes.h,ModbusPagePresenter.{h,cpp},ModbusSessionPresenter.{h,cpp}}`、`ui/views/modbus/ModbusPage.{h,cpp}`、`ui/shell/MainWindowPageBuilder.cpp`
  - **实证**：`SessionMode` 与 `modbus::base::ModbusMode` 1:1 重复，催生了 70 行胶水表 `ModbusModeDescriptor`（其中 `settingsPrefix`、`usesUnitIdLabel` 0 调用）；`ModbusPagePresenter::formatExceptionCode` 重复了异常码映射。
  - **改造方案**：
    1. 删除 `SessionMode`，Modbus 应用层与 `ModbusPage` 直接使用 `::modbus::base::ModbusMode`；精简 `ModbusModeDescriptor` 删除未用字段 `settingsPrefix`、`usesUnitIdLabel`（并将 `ModbusPage::applyProtocolSettingsGroup` 与精简后的描述符对齐）。
    2. 精简 `ModbusPagePresenter::formatExceptionCode` 避免与 `function_traits::exceptionCodeName()` 维护多份英文表（保留 `QCoreApplication::translate` 查表映射）。
  - **验证**：全仓 `git grep "SessionMode"` 0 残留，编译与全量测试通过。

- [x] **T5-20 [S] 清理 Modbus 生态全部 0 生产调用死代码与 48 个僵尸配置键**
  - **位置**：
    - `ui/application/modbus/RequestSubmissionService.{h,cpp}`：删除 177 行 `buildWriteRequest()`、`buildExecuteRequest` 的 raw 死分支、`validateRawData()`（`handleRawExecute` 改走 `validateSpec` 补齐 260B ADU 上限）、`lookup()`、`RequestTrackingInfo::startTime`、未用参数 `mode`；
    - `ui/application/modbus/ModbusSessionPresenter.{h,cpp}`：删除 0 槽连接的 `rawFrameReceived` 信号、僵尸指针 `requestService_`、未用参数 `generation`；
    - `ui/application/modbus/ModbusPagePresenter.h`：删除死槽 `requestExecute`、`requestPollToggle`、`requestService()`；
    - `ui/widgets/modbus/ModbusTelemetryWidget.{h,cpp}`：删除构造完整 `ModbusTelemetryWidget dummy;` 的静态 `formatInspectorDetails(pr, len)`、死信号 `frameSelectedForAnalysis`/`exportFailed`、4 个 0 调用 getter；
    - `ui/widgets/modbus/TelemetryRowView.h` & `ui/common/TrafficEvent.h`：删除 `sessionGeneration`、`protocol`（只写不读）、`TrafficEvent::isPoll`（只写不读）；
    - `core/modbus/dispatch/ModbusWorker.{h,cpp}`：删除 0 生产调用的 `requestDisconnect()`/`disconnectFinished`；
    - `core/common/SettingsKeys.h`、`infra/config/SettingsService.cpp`、`core/Config.h`：清理 48 个旧版 `standard/*`/`traffic/*`/`control/*`/`dataMonitorCollapsed` 僵尸配置键及配套 8 个死常量（`kDefaultStandard*`/`kDefaultControl*`/`kFrameAnalyzerCsvExportChunkRows`/`kTrafficLogExportChunkRows`/`kDataMonitorFlushIntervalMs`）。
  - **验证**：同步裁剪仅测死代码的废弃测试用例，验证 `SettingsServiceTest` 与全部现存功能单测通过。

- [x] **T5-21 [S] Modbus 内部 DRY 复用补齐与 CSV CWE-1236 公式注入防护**
  - **位置**：`ui/application/modbus/PollingController.{h,cpp}`、`core/modbus/session/RequestExecutor.cpp`（`L89-113`）、`ui/widgets/modbus/ModbusWorkbenchWidget.cpp`（`L855-869`）、`ui/widgets/modbus/ModbusTelemetryWidget.cpp`（`L49-53`, `L871-893`）
  - **实证**：`PollingController` 手写两遍 5s 去重而非用 `common::LogDedupe`；`RequestExecutor` 手写 `isBroadcastWriteFunction`/`isReadFunction` 而非用 `function_traits`；`ModbusWorkbenchWidget::onAddMbapClicked` 手拼 7 字节 MBAP 而非用 `buildTcpAdu()`；`ModbusTelemetryWidget::csvEscapeField` 缺少 CWE-1236 防护。
  - **改造方案**：
    1. `PollingController` 改用 `common::LogDedupe`；`RequestExecutor` 改用 `function_traits::isReadOperation`/`isWriteOperation`；`ModbusWorkbenchWidget::onAddMbapClicked` 复用 `modbus::base::buildTcpAdu()`。
    2. `ModbusTelemetryWidget` 删除自写 `csvEscapeField`，直接调用 `::modbus::analyzer::exporter::escapeCsvValue()`（**零修改** `AnalyzerExporter`）补齐 CWE-1236 防护；消除 `ModbusPagePresenter::handleStandardExecute` 对 `validateSpec` 的双重调用。
  - **验证**：新增 `ModbusTelemetryWidget` CSV 导出防公式注入（`=cmd`、`+1`、`-1`、`@SUM`）单测。

---

### 批次 7：P3 ModbusPage MVP 解耦、旧命名清扫与全量门禁闭环（零跨页侵入）

> **本批范围**：`ui/views/modbus/ModbusPage.{h,cpp}`、`ui/widgets/modbus/{ModbusWorkbenchWidget,ModbusTelemetryWidget,TelemetryRowView,TelemetryTableModel}.{h,cpp}`、`ui/application/modbus/{ModbusPagePresenter,ModbusExecuteSpec,IModbusPageView}.{h,cpp}`、`ui/i18n/*.ts`、`tests/**`。
> **验收标准**：`ModbusPage` 符合 Passive View 契约；遥测过滤与界面文案彻底解耦；全链路无误导性旧命名与僵尸注释；全量构建、`ctest` 与 `qt_safety_check.py` 全绿。

- [x] **T5-22 [S] `ModbusPage` MVP 解耦（配置组装与设置持久化收口至 Presenter / `SettingsController`）**
  - **位置**：`ui/views/modbus/ModbusPage.{h,cpp}`（`L29-39`, `L109-116`, `L235-241`, `L367-399`）、`ui/application/modbus/ModbusPagePresenter.{h,cpp}`
  - **实证**：`ModbusPage` 直接构造 `::modbus::base::ModbusConfig` / `ModbusConnectionSpec` 并绕过 `SettingsController` 直接读写 `ISettingsService` 的 `kModbusAddressBase`，导致兄弟页面收不到地址基准变更通知；`setLinked()` 重复同步 `telemetryWidget_`。
  - **改造方案**：
    1. `ModbusPage` 仅将原始 `tcpConnectRequested(ip, port)` / `serialConnectRequested(serialConfig)` 转发给 `ModbusPagePresenter`，由 Presenter 结合 `timingParams_` 与当前 `slaveId` 组装 `ModbusConfig` 与 `ModbusConnectionSpec`。
    2. `kModbusAddressBase` 的读取与写入统一经 `SettingsController`（与 `MainWindow` 共享变更通知），消除 `ModbusPage` 直接操作 `ISettingsService` 键值。
    3. 清理 `ModbusPage::setLinked()` 中冗余的 `telemetryWidget_->setLinkageState(linked)`（已由 Presenter 经 `IModbusPageView::setLinkageState` 推送）。
  - **验证**：`ConnectionWidgetsTest`、`ModbusPagePresenterTest`、`I18nWidgetTest` 全绿。

- [x] **T5-23 [S] 遥测结构化过滤与全链路旧命名 / 僵尸注释清扫**
  - **位置**：`ui/widgets/modbus/ModbusTelemetryWidget.{h,cpp}`（`L118`, `L824-831`, `L991`）、`ui/widgets/modbus/TelemetryTableModel.{h,cpp}`、`ui/widgets/modbus/ModbusWorkbenchWidget.{h,cpp}`、`ui/views/modbus/ModbusPage.h`、`ui/application/modbus/{ModbusPagePresenter,ModbusSessionPresenter,ModbusExecuteSpec}.{h,cpp}`
  - **改造方案**：
    1. `TelemetryTableModel` 新增自定义 `StatusCategoryRole`（`Info` / `Warning` / `Error` / `Success`），`ModbusTelemetryWidget::onFilterChanged()` 按角色过滤而非用英文正则 `"Error|Warning|Exception|Malformed"` 匹配显示文本。
    2. 统一清扫误导命名与僵尸注释：
       - `ModbusWorkbenchWidget`：`writeGroup_` → `requestDataGroup_`，`writeStackedLayout_` → `requestDataStackedLayout_`，`radix_` → `addressRadix_`，修正类注释 `"Master-detail workbench controller"`；
       - `ModbusPage`：`connectionWidget_` → `activeConnectionWidget_`；
       - `ModbusPagePresenter`：`handleLogMessage(const QString&, bool isWarning)` 对齐实际 `logWarning` 行为；
       - `ModbusExecuteSpec`：`rawPduOrAdu` → `rawAdu`（明确传输原始帧语义）；
       - `ModbusSessionPresenter`：清理 `UI FSM` / `transitionTo` / `control-widget` 僵尸注释，按 TCP vs Serial 区分连接成功日志文案（串口不再打 `"Transport connected, waiting for device response..."`）；
       - `ModbusTelemetryWidget`：采样率 Tooltip 改为动态读取 `config::Ui::kPollFrameSampleRate` 格式化，消除硬编码 `"1 in 10"`；补全遗漏 `workbench`/`telemetry` 前缀的 `objectName`；清理全批涉及文件中的未用 `#include`。
  - **验证**：`ModbusTelemetryWidgetTest`、`ModbusWorkbenchWidgetTest`、`ConnectionWidgetsTest` 全绿。

- [x] **T5-24 [D] 全量单测补强、`.ts` 翻译同步与提交前门禁闭环**
  - **位置**：`ui/i18n/Modbus-Tools_{zh_CN,zh_TW}.ts`、`tests/unit/logging/**`、`tests/unit/modbus/**`、`tests/unit/ui/**`
  - **改造方案**：同步更新中繁 `.ts` 中变动的文案条目；检查所有修改文件的类行数（非 UI $\le 800$，UI $\le 1200$）与函数行数（$\le 40$）；运行全套门禁命令确认零违规、零失败。
  - **验证**：全量构建 + `ctest` + 静态安全扫描全部通过。

---

## 四、统一门禁与回归命令

每个批次落地后执行以下全套验证：

```powershell
python .agents/scripts/tests/test_qt_safety_check.py
python .agents/scripts/qt_safety_check.py core infra ui app
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure -E "Loopback|Channel|TcpServerHandle|FirstConnect"
```

---

## 五、刻意挂起至后续页面重构批次的跨页项（不纳入 Task 5）

为避免在 Network / Serial / FrameAnalyzer / Utilities 重构前造成跨页侵入与二次返工，审计报告中发现的以下 5 项共享组件收敛**明确延期**：

1. **留待 Network / Serial 重构批次**：
   - 将 `core/modbus/telemetry/TrafficMetrics.h` 迁出 `core/modbus/` 并与 `ByteMonitorWidget` / `ModbusPagePresenter` 计数策略统筹；
   - 收敛 `ChannelController` 线程等待与 `ThreadGuard` 复用；
   - 清理共享控件 `BaseConnectionWidget` 的死字段 `isConnected_` 与死枚举 `DisplayState::TransportConnected`；
   - 统一 `FileExporter`（消除死 `append=true` 路径）并与 `ByteMonitorWidget` 对齐。
2. **留待 FrameAnalyzer / Utilities 重构批次**：
   - 合并 `ModbusDataHelper`、`ModbusFrameBuilderWidget` 与 `core/modbus/base` 的 4 套写载荷/进制解析器与 5 层重复 IEC 校验；
   - 合并 `AnalyzerExporter` 与 `FileExporter` 为统一原子安全导出服务；
   - 清理 `ValueFormatter` 旧 `QVariant` 重载、统一 4 份顺序不一致的 `RegisterDataType`/`RegisterOrder` 下拉列表、以及统一 `ProtocolType` 与 `ModbusMode` 枚举。
