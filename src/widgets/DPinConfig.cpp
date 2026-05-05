#include "DPinConfig.hpp"
#include "ui_DPinConfig.h"

DPinConfig::DPinConfig(const QString &hwName, const int blockNum, QWidget *parent) : QWidget(parent),
    ui(new Ui::DPinConfig) {
    ui->setupUi(this);

    m_info.enabled = false;
    m_info.hwName = hwName;

    ui->cbEnabled->setChecked(m_info.enabled);
    ui->cbEnabled->setText(QString("%1 (%2)").arg(m_info.hwName).arg(blockNum));
    ui->editLogColumn->setText(QString("%1").arg(m_info.hwName));
    ui->editLogColumn->setPlaceholderText(QString("%1").arg(m_info.hwName));
    cbEnabled_checkStateChanged(ui->cbEnabled->checkState());

    connect(ui->cbEnabled, &QCheckBox::checkStateChanged, this, &DPinConfig::cbEnabled_checkStateChanged);
    connect(ui->editLogColumn, &QLineEdit::editingFinished, this, &DPinConfig::editLogColumn_editingFinished);
}

DPinConfig::~DPinConfig() {
    delete ui;
}

void DPinConfig::cbEnabled_checkStateChanged(const Qt::CheckState state) {
    const bool enabled = state == Qt::Checked;
    m_info.enabled = enabled;
    ui->editLogColumn->setEnabled(enabled);
    emit cb_checked(enabled, m_info.hwName);
}

void DPinConfig::editLogColumn_editingFinished() {
    m_info.dataName = ui->editLogColumn->text();
}

DPinConfig::Info DPinConfig::info() const {
    return m_info;
}

void DPinConfig::setInfo(const Info &info) {
    m_info = info;
    ui->cbEnabled->setCheckState(m_info.enabled ? Qt::Checked : Qt::Unchecked);
    ui->editLogColumn->setText(m_info.dataName);
    ui->editLogColumn->setEnabled(m_info.enabled);
}