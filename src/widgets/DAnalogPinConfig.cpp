#include "DAnalogPinConfig.hpp"
#include "ui_DAnalogPinConfig.h"

DAnalogPinConfig::DAnalogPinConfig(const QString &hwName, const int blockNum, QWidget *parent) : QWidget(parent),
    ui(new Ui::DAnalogPinConfig) {
    ui->setupUi(this);

    m_info.enabled = false;
    m_info.hwName = hwName;

    ui->cbEnabled->setChecked(m_info.enabled);
    ui->cbEnabled->setText(QString("%1 (%2)").arg(m_info.hwName).arg(blockNum));
    ui->editLogColumn->setText(QString("%1").arg(m_info.hwName));
    ui->editLogColumn->setPlaceholderText(QString("%1").arg(m_info.hwName));
    cbEnabled_checkStateChanged(ui->cbEnabled->checkState());

    connect(ui->cbEnabled, &QCheckBox::checkStateChanged, this, &DAnalogPinConfig::cbEnabled_checkStateChanged);
    connect(ui->editLogColumn, &QLineEdit::editingFinished, this, &DAnalogPinConfig::editLogColumn_editingFinished);
    connect(ui->cbDifferential, &QCheckBox::checkStateChanged, this,
            &DAnalogPinConfig::cbDifferenfial_checkStateChanged);
}

DAnalogPinConfig::~DAnalogPinConfig() {
    delete ui;
}

void DAnalogPinConfig::cbEnabled_checkStateChanged(const Qt::CheckState state) {
    const bool enabled = state == Qt::Checked;
    m_info.enabled = enabled;
    ui->editLogColumn->setEnabled(enabled);
    ui->cbDifferential->setEnabled(enabled);

    if (m_info.differential && m_differentialOther) {
        if (enabled) {
            m_differentialOther->setDisabled(enabled);
        } else {
            m_differentialOther->setDisabled(enabled);
        }
    }

    emit cb_checked(enabled, m_info.hwName);
}

void DAnalogPinConfig::cbDifferenfial_checkStateChanged(Qt::CheckState state) {
    const bool enabled = state == Qt::Checked;
    m_info.differential = enabled;

    if (m_differentialOther) {
        m_differentialOther->setDisabled(enabled);
    }
}

void DAnalogPinConfig::editLogColumn_editingFinished() {
    m_info.dataName = ui->editLogColumn->text();
}

DAnalogPinConfig::Info DAnalogPinConfig::info() const {
    return m_info;
}

void DAnalogPinConfig::setInfo(const Info &info) {
    m_info = info;
    ui->cbEnabled->setCheckState(m_info.enabled ? Qt::Checked : Qt::Unchecked);
    ui->editLogColumn->setText(m_info.dataName);
    ui->editLogColumn->setEnabled(m_info.enabled);
    ui->cbDifferential->setCheckState(info.differential ? Qt::Checked : Qt::Unchecked);
}

void DAnalogPinConfig::setDifferentialPin(DAnalogPinConfig *other) {
    m_differentialOther = other;
}

void DAnalogPinConfig::setDifferentialVisible(const bool visible) {
    ui->cbDifferential->setVisible(visible);
    m_info.canDifferential = false;
}