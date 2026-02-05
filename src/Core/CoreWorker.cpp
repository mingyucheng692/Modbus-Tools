#include "CoreWorker.h"
#include <QThread>

CoreWorker::CoreWorker(QObject* parent) : QObject(parent) {}

CoreWorker::~CoreWorker() {
    spdlog::info("CoreWorker Destroyed in thread: {}", (uint64_t)QThread::currentThreadId());
}

void CoreWorker::init() {
    spdlog::info("CoreWorker Initialized in thread: {}", (uint64_t)QThread::currentThreadId());
    emit workerReady();
}

void CoreWorker::cleanup() {
    spdlog::info("CoreWorker Cleanup");
}

void CoreWorker::testWorker() {
    spdlog::info("CoreWorker received test command in thread: {}", (uint64_t)QThread::currentThreadId());
    emit testResponse("Hello from Worker Thread");
}
