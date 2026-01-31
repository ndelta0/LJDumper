#pragma once

#include <QFile>
#include <QMainWindow>

#include "widgets/DFlowLayout.hpp"
#include "widgets/DPinConfig.hpp"

class LjOutputControlWindow;
class LjManager;
QT_BEGIN_NAMESPACE

namespace Ui {
    class LjdMainWindow;
}

QT_END_NAMESPACE

struct CallbackInfo {
    uint64_t sampleNumber = 0;
    LjManager *ljm;
    int scansPerRead;
    int scanRate;
    double timeResolution;
    int timePrecision;
    int precision;
    std::vector<int> columns;
    QFile *outputFile;
    int error = 0;
};

class LjdMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit LjdMainWindow(QWidget *parent = nullptr);

    ~LjdMainWindow() override;

private slots:
    void btnConnection_clicked();

    void btnStartStop_clicked();

    void inputEnabled(bool enabled, const QString &hwName) const;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::LjdMainWindow *ui;

    LjManager *m_ljm;
    bool m_isConnected = false;
    bool m_isScanning = false;

    DFlowLayout *m_inputConfigLayout;

    QVector<DPinConfig *> m_pinConfigs;

    CallbackInfo m_callbackInfo;
    double *m_dataBuffer = nullptr;

    LjOutputControlWindow *m_outputControlWindow = nullptr;

    void streamReadCallback();
};
