#pragma once

#include <QWidget>


QT_BEGIN_NAMESPACE

namespace Ui {
    class DAnalogPinConfig;
}

QT_END_NAMESPACE

class DAnalogPinConfig : public QWidget {
    Q_OBJECT

public:
    struct Info {
        bool enabled;
        QString hwName;
        QString dataName;
        bool differential = false;
        bool canDifferential = true;
    };

    explicit DAnalogPinConfig(const QString &hwName, int blockNum, QWidget *parent = nullptr);

    ~DAnalogPinConfig() override;

    [[nodiscard]] Info info() const;

    void setInfo(const Info &info);

    void setDifferentialPin(DAnalogPinConfig *other);

    void setDifferentialVisible(bool visible);

    signals:
    

    void cb_checked(bool checked, QString hwName);

private:
    Ui::DAnalogPinConfig *ui;

    Info m_info;

    DAnalogPinConfig *m_differentialOther = nullptr;

private
    slots:
    

    void cbEnabled_checkStateChanged(Qt::CheckState state);

    void cbDifferenfial_checkStateChanged(Qt::CheckState state);

    void editLogColumn_editingFinished();
};