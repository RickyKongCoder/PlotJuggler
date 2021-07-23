#ifndef LOGPANEL_H
#define LOGPANEL_H

#include "PLotJuggler/plotdata.h"
#include "PlotJuggler/svg_util.h"
#include "ui_logpanel.h"
#include <QWidget>
namespace Ui {
class LogPanel;
};

class LogPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LogPanel(PJ::PlotDataMapRef &dataplot, QWidget *parent = nullptr);

private slots:

public slots:
    void updateLogMessage();
    void update(double tracker_time);

protected:
private:
    Ui::LogPanel *ui;
    double latest_time;
    double tracker_time;
    PJ::PlotDataMapRef &datamap;
    void printTime();
    void printTag();
    void printThreadName();
signals:
};

#endif // LOGPANEL_H
