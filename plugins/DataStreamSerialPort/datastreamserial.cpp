/*DataStreamSerial PlotJuggler  Plugin license(Faircode)

Copyright(C) 2018 Philippe Gauthier - ISIR - UPMC
Permission is hereby granted to any person obtaining a copy of this software and associated documentation files(the
"Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and / or sell copies("Use") of the Software, and to permit persons to whom the
Software is furnished to do so. The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "datastreamserial.h"

#include <math.h>

#include <QDebug>
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QWebSocket>
#include <mutex>
#include <thread>

DataStreamSerial::DataStreamSerial() : _running(false) {
	serial.setBaudRate(115200);
	serial.setDataBits(QSerialPort::Data8);
	serial.setParity(QSerialPort::NoParity);
	serial.setStopBits(QSerialPort::OneStop);
}

DataStreamSerial::~DataStreamSerial() { shutdown(); }

bool DataStreamSerial::start(QStringList*) {
	if (!_running) {
		bool ok;
		QStringList items;
		foreach (const QSerialPortInfo& info, QSerialPortInfo::availablePorts()) {
			portlist.push_back(info);
			items << info.portName();
		}
		QInputDialog qDialog;
		qDialog.setOptions(QInputDialog::UseListViewForComboBoxItems);
		qDialog.setComboBoxItems(items);
		qDialog.setWindowTitle("Choose port");
		if (qDialog.exec()) {
			setInputSerial(qDialog.textValue());
		}

	} else {
		qDebug() << "Serial Port already running on port " << serial.portName();
		QMessageBox::information(nullptr, "Info", QString("Serial Port already running on port: ") + serial.portName());
	}
	return _running;
}

void DataStreamSerial::shutdown() {
	if (_running) {
		serialDisconnected();
		serial.close();
		_running = false;
	}
}

void DataStreamSerial::onNewConnection() {
	qDebug() << "DataStreamSerial: onNewConnection";
	connect(&serial, &QIODevice::readyRead, this, &DataStreamSerial::processMessage);
}
#define start_byte 'j'
void DataStreamSerial::processMessage() {
	std::lock_guard<std::mutex> lock(mutex());
	if (!serial.canReadLine()) return;
	// just for test
	QString message = serial.readLine();
	int startByteIndex = message.lastIndexOf(start_byte);
	int endByteIndex = message.lastIndexOf('\n');
	if (startByteIndex < endByteIndex && startByteIndex != -1 && endByteIndex != -1) {
		message = message.mid(startByteIndex + 1, endByteIndex - (startByteIndex + 1));

		// qDebug() << "DataStreamSerial: processMessage: " << message;

		QStringList lst = message.split(':');
		if (lst.size() == 3) {
			QString key = lst.at(0);
			double time = lst.at(1).toDouble();
			double value = lst.at(2).toDouble();

			auto& serial_numeric_plots = dataMap().numeric;

			const std::string data_name_str = key.toStdString();
			auto target_plotIt = serial_numeric_plots.find(data_name_str);

			if (target_plotIt == serial_numeric_plots.end()) {
				qDebug() << "name:" << QString::fromStdString(data_name_str);
				dataMap().addNumeric(data_name_str);
			} else {
				qDebug() << "T:" << time << " V:" << value;
				target_plotIt->second.pushBack({time, value});
			}
		}
	}
}

void DataStreamSerial::serialDisconnected() {
	qDebug() << "DataStreamSerial: serialDisconnected";
	disconnect(&serial, &QIODevice::readyRead, this, &DataStreamSerial::processMessage);
}

void DataStreamSerial::setInputSerial(const QString& clicked_text) {
	QSerialPortInfo targetPort;
	for (int i = 0; i < portlist.size(); ++i) {
		if (portlist[i].portName() == clicked_text) {
			targetPort = portlist[i];
			break;
		}
	}
	serial.setPort(targetPort);

	bool opened = serial.open(QIODevice::ReadOnly);
	if (!opened) {
		QMessageBox::information(nullptr, "Error", QString("Unable to connect to port " + serial.portName()));
		_running = false;
	} else {
		QMessageBox::information(nullptr, "Info", QString("Connected to port " + serial.portName()));
		onNewConnection();
		_running = true;
	}
}