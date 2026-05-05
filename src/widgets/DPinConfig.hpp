#pragma once

#include <QWidget>


QT_BEGIN_NAMESPACE

namespace Ui {
    class DPinConfig;
}

QT_END_NAMESPACE

class DPinConfig : public QWidget {
    Q_OBJECT

public:
    struct Info {
        bool enabled;
        QString hwName;
        QString dataName;
    };

    explicit DPinConfig(const QString &hwName, int blockNum, QWidget *parent = nullptr);

    ~DPinConfig() override;

    [[nodiscard]] Info info() const;

    void setInfo(const Info &info);

    signals:
    

    void cb_checked(bool checked, QString hwName);

private:
    Ui::DPinConfig *ui;

    Info m_info;

private
    slots:
    

    void cbEnabled_checkStateChanged(Qt::CheckState state);

    void editLogColumn_editingFinished();
};