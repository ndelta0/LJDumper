#pragma once

#include <QWidget>


class WClockConfig;
class WOutputPinRow;
class LjManager;
QT_BEGIN_NAMESPACE

namespace Ui {
    class LjOutputControlWindow;
}

QT_END_NAMESPACE

class LjOutputControlWindow : public QWidget {
    Q_OBJECT

public:
    explicit LjOutputControlWindow(LjManager *ljm, QWidget *parent = nullptr);

    ~LjOutputControlWindow() override;

    void SetOutputDisabled(bool disable, const QString &hwName);

protected
    slots:
    

    void closeEvent(QCloseEvent *event) override;

private:
    Ui::LjOutputControlWindow *ui;
    LjManager *ljm;

    // WClockConfig *clockConfig0;
    WClockConfig *clockConfig1;
    WClockConfig *clockConfig2;

    QVector<WOutputPinRow *> pins;
};