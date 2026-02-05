#pragma once
#include <QObject>
#include <spdlog/spdlog.h>

class CoreWorker : public QObject {
    Q_OBJECT
public:
    explicit CoreWorker(QObject* parent = nullptr);
    ~CoreWorker();

public slots:
    void init();
    void cleanup();
    void testWorker();

signals:
    void workerReady();
    void testResponse(const QString& msg);
};
