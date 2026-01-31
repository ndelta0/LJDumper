#pragma once

#include <functional>
#include <LabJackM.h>
#include <QApplication>

#include <QtConcurrent/QtConcurrentRun>

struct LjHandleInfo {
    int deviceType;
    int connectionType;
    int serialNumber;
    int ipAddress;
    int port;
    int maxBytesPerMB;
};

class LjManager : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(LjManager)

public:
    explicit LjManager(QObject *parent = nullptr);

    ~LjManager() override;

    template<typename T>
    void execute(const std::function<T()> &operation,
                 const std::function<void(T)> &callback = nullptr);

    int ljHandle() const;

#define DECLARE(name, ret_type, ...) \
    auto name(__VA_ARGS__ __VA_OPT__(,) const std::function<void(ret_type, LJM_ERROR_RETURN)> &callback = nullptr) -> void; \
    auto name##Sync(__VA_ARGS__) -> std::pair<ret_type, LJM_ERROR_RETURN>;
#define DECLARE_VOID(name, ...) \
    auto name(__VA_ARGS__ __VA_OPT__(,) const std::function<void(LJM_ERROR_RETURN)> &callback = nullptr) -> void; \
    auto name##Sync(__VA_ARGS__) -> LJM_ERROR_RETURN;

    DECLARE(openDevice, int, int deviceType, int connectionType, const QString &identifier)

    DECLARE_VOID(closeDevice)

    DECLARE(getHandleInfo, LjHandleInfo)

    DECLARE(readName, double, const QString &name)

    DECLARE(readNameString, QString, const QString &name)

    DECLARE_VOID(writeName, const QString &name, double value)

    DECLARE_VOID(writeNameString, const QString &name, const QString &value)

    DECLARE(streamStart, double, int scansPerRead, const std::vector<int> &addresses, double scanRate);

    DECLARE_VOID(streamStop);

    DECLARE_VOID(streamRead, double *data, int *devBacklog, int *ljmBacklog);

signals:
    void operationStarted();

    void operationFinished();

private:
    QThread m_workerThread;
    int m_ljHandle = -1;
};

// Template implementation
template<typename T>
void LjManager::execute(const std::function<T()> &operation,
                        const std::function<void(T)> &callback) {
    emit operationStarted();

    QThreadPool::globalInstance()->start([operation, callback, this] {
        T result = operation();

        // Use QMetaObject::invokeMethod to ensure the callback runs in the main thread
        if (callback) {
            QMetaObject::invokeMethod(QApplication::instance()->thread(), [callback, result] {
                callback(result);
            });
        }

        emit operationFinished();
    });
}
