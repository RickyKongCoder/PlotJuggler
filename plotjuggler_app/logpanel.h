#ifndef LOGPANEL_H
#define LOGPANEL_H

#include <QWidget>

#include "PlotJuggler/svg_util.h"
#include "ui_logpanel.h"
namespace Ui {
class LogPanel;
};

class LogPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LogPanel(QWidget *parent);

private slots:

public slots:

protected:
private:
    Ui::LogPanel *ui;
signals:
};

#endif // LOGPANEL_H
