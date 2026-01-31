#pragma once

#include "lj/ljManager.hpp"
#include <QWidget>


class Int64SpinBox;
QT_BEGIN_NAMESPACE

namespace Ui {
    class WClockConfig;
}

QT_END_NAMESPACE

class WClockConfig : public QWidget {
    Q_OBJECT

public:
    explicit WClockConfig(int clockNumber, long maxRollover, LjManager *ljm, QWidget *parent = nullptr);

    ~WClockConfig() override;

    double getTickRate() const;

    double getResolution() const;

    double getFrequency() const;

private
    slots:
    

    void cbAdvanced_checkStateChanged(Qt::CheckState state);

    void applyConfigBasic();

    void applyConfigAdvanced();

private:
    Ui::WClockConfig *ui;
    Int64SpinBox *sbAdvRollover;

    LjManager *ljm;

    int m_clockNumber;
    long m_maxRollover;

    int m_divisor;
    long m_rollover;
};