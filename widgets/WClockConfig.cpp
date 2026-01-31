#include "WClockConfig.hpp"

#include "Int64SpinBox.hpp"
#include "ui_WClockConfig.h"
#include "utils.hpp"


WClockConfig::WClockConfig(const int clockNumber, const long maxRollover, LjManager *ljm,
                           QWidget *parent) : QWidget(parent), ui(new Ui::WClockConfig), ljm(ljm),
                                              m_clockNumber(clockNumber), m_maxRollover(maxRollover) {
    ui->setupUi(this);

    ui->dispClockName->setText(QString("Zegar %1").arg(clockNumber));
    sbAdvRollover = new Int64SpinBox;
    sbAdvRollover->setRange(1, maxRollover);
    sbAdvRollover->setValue(maxRollover);
    ui->sbAdvPlaceholder->layout()->addWidget(sbAdvRollover);
    ui->sbAdvPlaceholder->setContentsMargins(0, 0, 0, 0);

    ui->cbAdvanced->setCheckState(Qt::Unchecked);
    ui->swConfig->setCurrentIndex(0);
    applyConfigBasic();

    connect(ui->cbAdvanced, &QCheckBox::checkStateChanged, this, &WClockConfig::cbAdvanced_checkStateChanged);
    connect(ui->cbBasicFrequency, &QComboBox::currentIndexChanged, this, &WClockConfig::applyConfigBasic);
    connect(ui->cbAdvDivisor, &QComboBox::currentIndexChanged, this, &WClockConfig::applyConfigAdvanced);
    connect(sbAdvRollover, qOverload<Int64SpinBox::ValueType>(&Int64SpinBox::valueChanged), this,
            &WClockConfig::applyConfigAdvanced);
}

WClockConfig::~WClockConfig() {
    delete ui;
}

void WClockConfig::cbAdvanced_checkStateChanged(const Qt::CheckState state) {
    if (state == Qt::Checked) {
        ui->swConfig->setCurrentIndex(1);
        applyConfigAdvanced();
    } else {
        ui->swConfig->setCurrentIndex(0);
        applyConfigBasic();
    }
}

constexpr double CORE_FREQ = 80'000'000.0; // 80 MHz for T7

void WClockConfig::applyConfigBasic() {
    if (ui->swConfig->currentIndex() != 0) return;

    int divisor = 0;
    int rollover = 0;

    switch (ui->cbBasicFrequency->currentIndex()) {
        case 0:
            // 50 kHz -> div=1, roll=1600
            divisor = 1;
            rollover = 1600;
            break;
        default:
        case 1:
            // 20 kHz -> div=2, roll=2000
            divisor = 2;
            rollover = 2000;
            break;
        case 2:
            // 10 kHz -> div=2, roll=4000
            divisor = 2;
            rollover = 4000;
            break;
        case 3:
            // 5 kHz -> div=4, roll=4000
            divisor = 4;
            rollover = 4000;
            break;
        case 4:
            // 2 kHz -> div=8, roll=5000
            divisor = 8;
            rollover = 5000;
            break;
    }

    m_divisor = divisor;
    m_rollover = rollover;

    const auto reg_prefix = QString("DIO_EF_CLOCK%1").arg(m_clockNumber);
    ljm->writeNameSync(reg_prefix + "_ENABLE", 0);
    ljm->writeNameSync(reg_prefix + "_DIVISOR", m_divisor);
    ljm->writeNameSync(reg_prefix + "_ROLL_VALUE", m_rollover);
    ljm->writeNameSync(reg_prefix + "_ENABLE", 1);

    ui->dispBasicResolution->setText(FormatTime(getResolution()));
}

void WClockConfig::applyConfigAdvanced() {
    if (ui->swConfig->currentIndex() != 1) return;

    m_divisor = std::pow(2, ui->cbAdvDivisor->currentIndex());
    m_rollover = sbAdvRollover->value();

    const auto reg_prefix = QString("DIO_EF_CLOCK%1").arg(m_clockNumber);
    ljm->writeNameSync(reg_prefix + "_ENABLE", 0);
    ljm->writeNameSync(reg_prefix + "_DIVISOR", m_divisor);
    ljm->writeNameSync(reg_prefix + "_ROLL_VALUE", m_rollover);
    ljm->writeNameSync(reg_prefix + "_ENABLE", 1);

    ui->dispAdvTickRate->setText(FormatFrequency(getTickRate()));
    ui->dispAdvResolution->setText(FormatTime(getResolution()));
    ui->dispAdvFrequency->setText(FormatFrequency(getFrequency()));
}

double WClockConfig::getTickRate() const {
    return CORE_FREQ / m_divisor;
}

double WClockConfig::getResolution() const {
    return 1.0 / getTickRate();
}

double WClockConfig::getFrequency() const {
    return getTickRate() / m_rollover;
}