#pragma once

#include "utils.hpp"


class LjManager;

QT_BEGIN_NAMESPACE

namespace Ui {
    class WOutputPinRow;
}

QT_END_NAMESPACE

class WOutputPinRow : public QWidget {
    Q_OBJECT

public:
    enum class ESupportedFunctions {
        None = 0,
        Pwm = 1 << 0,
        Pulse = 1 << 1,
    };

    explicit WOutputPinRow(QString hwName, int blockNum, ESupportedFunctions funcs, LjManager *ljm,
                           QWidget *parent = nullptr);

    ~WOutputPinRow() override;

    [[nodiscard]] const QString &getName() const;

private slots:
    void cbPinFunction_activated(int index);

    void cbDigitalOutput_checkStateChanged(Qt::CheckState state) const;

    void cbPwmEnable_checkStateChanged(Qt::CheckState state);

    void cbPwmClock_activated(int index);

    void sbPwmDuty_valueChanged(int value);

    void cbPulseClock_activated(int index);

    void sbPulseCount_valueChanged(int value);

    void btnPulseStart_clicked();

private:
    Ui::WOutputPinRow *ui;

    QString hwName;
    int dio_num;
    int blockNum;
    ESupportedFunctions supportedFunctions;
    LjManager *ljm;

    ESupportedFunctions currentFunction;

    struct {
        int selected_clock = 1;
        int duty_cycle = 0;
    } pwm_settings;

    struct {
        int selected_clock = 1;
        int pulse_ratio = 0;
        int pulse_count = 1;
    } pulse_settings;

    void updatePwmSettings();

    void updatePulseSettings();
};
