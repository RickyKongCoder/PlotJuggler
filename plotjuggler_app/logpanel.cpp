#include "logpanel.h"
#include <QWidget>

LogPanel::LogPanel(QWidget *parent) : ui(new Ui::LogPanel), QWidget(parent)
{
    ui->setupUi(this);
}
