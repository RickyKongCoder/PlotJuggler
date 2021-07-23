#include "logpanel.h"
#include <QWidget>

LogPanel::LogPanel(PJ::PlotDataMapRef &datamap, QWidget *parent)
    : ui(new Ui::LogPanel), QWidget(parent), datamap(datamap)
{
    ui->setupUi(this);
}

void LogPanel::printTime() {}
void LogPanel::printTag() {}
void LogPanel::printThreadName() {}
void LogPanel::update(double tracker_time)
{
    this->tracker_time = tracker_time;
    qDebug() << "Log Panel Update" << tracker_time;
    this->updateLogMessage();
}
void LogPanel::updateLogMessage()
{
    qDebug() << "update Log Message with size " << datamap.loging_message.size();
    for (auto &logi : datamap.loging_message) {
        int index = logi.second.getIndexFromX(this->tracker_time);

        ui->plainTextEdit->insertPlainText(
            "[" + QString::number(logi.second.at(index).x) + "s" + "]" + "["
            + QString::fromStdString(logi.first) + "]" + ":"
            + logi.second.getYfromX(tracker_time).value().data() + "\n");
    }
    for (auto &logi : datamap.loging_message) {
        for (auto &str : logi.second) {
            qDebug() << "logi str y" << str.y.data();
            qDebug() << "logi str x" << str.x;
            qDebug() << "tracker time:" << tracker_time;
        }
    }

    qDebug() << "XXXXXXXXXXXXXXXXXXX";
}
