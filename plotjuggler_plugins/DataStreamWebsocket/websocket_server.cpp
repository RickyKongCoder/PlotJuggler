/*Wensocket PlotJuggler Plugin license(Faircode, Davide Faconti)

Copyright(C) 2018 Philippe Gauthier - ISIR - UPMC
Copyright(C) 2020 Davide Faconti
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
#include "websocket_server.h"
#include "ObjectInfo.h"
#include <chrono>
#include <mutex>
#include <QDebug>
#include <QDialog>
#include <QFile>
#include <QIntValidator>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>
#include <QWebSocket>

#include "ui_websocket_server.h"
#define Init_byte '/'
#define ready_byte '+'
#define init_start_byte '&'

class WebsocketDialog : public QDialog
{
public:
  WebsocketDialog():
    QDialog(nullptr),
    ui(new Ui::WebSocketDialog)
  {
    ui->setupUi(this);
    ui->lineEditPort->setValidator( new QIntValidator() );
    setWindowTitle("WebSocket Server");
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  }
  ~WebsocketDialog()
  {
    while( ui->layoutOptions->count() > 0)
    {
      auto item = ui->layoutOptions->takeAt(0);
      item->widget()->setParent(nullptr);
    }
    delete ui;
  }
  Ui::WebSocketDialog* ui;
};

WebsocketServer::WebsocketServer() :
  _running(false),
  _server("plotJuggler", QWebSocketServer::NonSecureMode)
{
    connect(&_server, &QWebSocketServer::newConnection, this, &WebsocketServer::onNewConnection);
}

WebsocketServer::~WebsocketServer()
{
  shutdown();
}

bool WebsocketServer::start(QStringList*)
{
  if (_running)
  {
    return _running;
  }

  if( !availableParsers() )
  {
    QMessageBox::warning(nullptr,tr("Websocket Server"), tr("No available MessageParsers"),  QMessageBox::Ok);
    _running = false;
    return false;
  }

  bool ok = false;

  WebsocketDialog* dialog = new WebsocketDialog();

  for( const auto& it: *availableParsers())
  {
    dialog->ui->comboBoxProtocol->addItem( it.first );

    if(auto widget = it.second->optionsWidget() )
    {
      widget->setVisible(false);
      dialog->ui->layoutOptions->addWidget( widget );
    }
  }

  // load previous values
  QSettings settings;
  QString protocol = settings.value("WebsocketServer::protocol", "JSON").toString();
  int port = settings.value("WebsocketServer::port", 9871).toInt();

  dialog->ui->lineEditPort->setText( QString::number(port) );

  std::shared_ptr<MessageParserCreator> parser_creator;

  connect(dialog->ui->comboBoxProtocol, qOverload<int>( &QComboBox::currentIndexChanged), this,
          [&](int index)
  {
    if( parser_creator ){
      QWidget*  prev_widget = parser_creator->optionsWidget();
      prev_widget->setVisible(false);
    }
    QString protocol = dialog->ui->comboBoxProtocol->itemText(index);
    parser_creator = availableParsers()->at( protocol );

    if(auto widget = parser_creator->optionsWidget() ){
      widget->setVisible(true);
    }
  });

  dialog->ui->comboBoxProtocol->setCurrentText(protocol);

  int res = dialog->exec();
  if( res == QDialog::Rejected )
  {
    _running = false;
    return false;
  }

  port = dialog->ui->lineEditPort->text().toUShort(&ok);
  protocol = dialog->ui->comboBoxProtocol->currentText();
  dialog->deleteLater();

  _parser = parser_creator->createInstance({}, dataMap());

  // save back to service
  settings.setValue("WebsocketServer::protocol", protocol);
  settings.setValue("WebsocketServer::port", port);

  if (_server.listen(QHostAddress::Any, port)) {
      qDebug() << "Websocket listening on port" << port;
      _running = true;
  } else {
      QMessageBox::warning(nullptr,
                           tr("Websocket Server"),
                           tr("Couldn't open websocket on port %1").arg(port),
                           QMessageBox::Ok);
      _running = false;
  }
  qDebug() << "Start Server with ip" << _server.serverAddress().AnyIPv4 << endl;
  qDebug() << "Start Server with port" << _server.serverPort() << endl;
  return _running;
}

void WebsocketServer::shutdown()
{
  if (_running)
  {
    socketDisconnected();
    _server.close();
    _running = false;
  }
}

void WebsocketServer::onNewConnection()
{
    WebSocket *pSocket = new WebSocket(_server.nextPendingConnection(), this);
    //connect(pSocket, &QWebSocket::textMessageReceived, this, &WebsocketServer::processMessage);
    connect(pSocket, &WebSocket::binaryReceived, this, &WebsocketServer::processBinaryMessage);
    connect(pSocket->get(), &QWebSocket::disconnected, this, &WebsocketServer::socketDisconnected);
    qDebug() << "New connection with ip" << _server.serverAddress() << endl;
    qDebug() << "New connection with ip" << _server.serverPort() << endl;
    qDebug() << "I sent Text" << Init_byte << endl;
    pSocket->get()->sendTextMessage("FUCK ME");
    _clients << pSocket;
}

void WebsocketServer::processMessage(QString message)
{
    std::lock_guard<std::mutex> lock(mutex());

    using namespace std::chrono;
    auto ts = high_resolution_clock::now().time_since_epoch();
    double timestamp = 1e-6 * double(duration_cast<microseconds>(ts).count());
    QByteArray bmsg = message.toLocal8Bit();
    MessageRef msg(reinterpret_cast<uint8_t *>(bmsg.data()), bmsg.size());
    qDebug() << "message in string: " << message << endl;

    try {
        _parser->parseMessage(msg, timestamp);
    } catch (std::exception &err) {
        QMessageBox::warning(
            nullptr,
            tr("Websocket Server"),
            tr("Problem parsing the message. Websocket Server will be stopped.\n%1").arg(err.what()),
            QMessageBox::Ok);
        shutdown();
        emit closed();
        return;
    }
    emit dataReceived();
    return;
}

QByteArray WebsocketServer::waitready_proc(QByteArray message, WebSocket *pSocket)
{
    int index = 0;
    for (QByteArray::const_iterator iter = message.begin(); iter != message.end(); iter++) {
        if (*iter == Init_byte) {
            qDebug() << "find init signal" << endl;
            pstate = READY;
            message.remove(index, 1);
            QString str = QString(ready_byte);
            pSocket->get()->sendTextMessage(str);
            qDebug() << "this is message after waitready_proc" << index << " " << message;
            return message;
        }
        index++;
    }
    return message;
}
QByteArray WebsocketServer::waitinit_cyclproc(QByteArray message, WebSocket *pSocket)
{
    int index = 0;
    for (QByteArray::const_iterator iter = message.begin(); iter != message.end(); iter++) {
        if (*iter == init_start_byte) {
            qDebug() << "find waitinit signal" << endl;
            pstate = PROC_INIT;
            message.remove(index, 1);
            qDebug() << "this is message after waitinit_cyclproc" << index << " " << message;
            return message;
        }
        index++;
    }
    return message;
}
//void WebsocketServer::proc_init(QString message)
//{
//    for (int i = 0; i < message.size(); i++) {
//    }
//}

QByteArray WebsocketServer::initcycl_proc(QByteArray message, WebSocket *pSocket)
{
    uint8_t strlen = 0;
    ObjectInfo *info = new ObjectInfo();
    ENUMTYPE *eType;
    QString enum_name = "";
    for (QByteArray::const_iterator iter = message.begin(); iter != message.end(); iter++) {
        switch (pSocket->proc_state) {
        case ID:
            info->setId(*iter);
            qDebug() << "Id:" << uint8_t(*iter) << endl;
            pSocket->proc_state = STRLEN; //
            pSocket->prev_state = ID;
            break;
        case STRLEN:
            strlen = *iter;
            qDebug() << "Getlength:" << strlen << endl;
            if (pSocket->prev_state == ID) {
                pSocket->proc_state = VAR_NAME;
            } else if (pSocket->prev_state == VAR_NAME) {
                pSocket->proc_state = TYPE;
            } else if (pSocket->prev_state == TYPE) {
                pSocket->proc_state = ENUMNAME;
            } else if (pSocket->prev_state == ENUMNAME) {
                pSocket->proc_state = ENUMELEM;
            } else if (pSocket->prev_state == ENUMELEM) {
                pSocket->proc_state = ENUMELEM;
            }
            pSocket->prev_state = STRLEN;
            break;
        case VAR_NAME:
            qDebug() << "Received variable name" << message.mid(iter - message.begin(), strlen);
            info->setName(message.mid(iter - message.begin(), strlen));
            iter = std::next(iter, strlen - 1);
            pSocket->proc_state = TYPE;
            pSocket->prev_state = VAR_NAME;
            break;
        case TYPE:
            //I should get 1 bytes integer here
            qDebug() << "Received variable type" << (uint8_t(*iter));
            pSocket->proc_state = ID;
            if ((ObjectType)(uint8_t) *iter == ENUMTYPE_) {
                qDebug() << "found enum type" << endl;
                pSocket->prev_state = TYPE;
                pSocket->proc_state = STRLEN;
                info->setType(ENUMTYPE_);
            }
            break;
        case ENUMNAME: //To do is test if enum name works
            enum_name = message.mid(iter - message.begin(), strlen);
            qDebug() << "Received ENUM Name" << enum_name;
            info->set_enumName(enum_name);
            if (pSocket->get_enumRef().find(enum_name) == pSocket->get_enumRef().end()) {
                eType = new ENUMTYPE;
                pSocket->get_enumRef().insert(std::pair<QString, ENUMTYPE *>(enum_name, eType));
            }
            iter = std::next(iter, strlen - 1);

            pSocket->proc_state = STRLEN;
            pSocket->prev_state = ENUMNAME;
            break;
        case ENUMELEM:
            qDebug() << "Received ENUMELEM" << message.mid(iter - message.begin(), strlen);
            iter = std::next(iter, strlen - 1);
            if (*std::next(iter, 1) == '/') { //detect end signal
                pSocket->proc_state = ID;
                pSocket->prev_state = ENUMELEM;
                iter = std::next(iter, 1);
                qDebug() << "Detect End Signal" << endl;
                break;
            }
            pSocket->proc_state = STRLEN;
            pSocket->prev_state = ENUMELEM;

            //will refractor later BITCH 8=======================================D!!!!!
            break;
        default:
            break;
        }
    }
}
void WebsocketServer::processBinaryMessage(QByteArray message, WebSocket *qsocket)
{
    std::lock_guard<std::mutex> lock(mutex());

    //    if (!serial.canReadLine())
    //        return;
    // just for test
    // std::string data_name_str = "fuck";
    // auto target_plotIt = dataMap().numeric.find(data_name_str);
    qDebug() << "Message byte array" << message << endl;
    QByteArray pass;
    switch (pstate) {
    case INIT:
        pass = waitready_proc(message, qsocket);
        pass = waitinit_cyclproc(pass, qsocket);
        pass = initcycl_proc(pass, qsocket);
        pstate = PROC_VALUE;
        qDebug() << pass << "message for now" << endl;
        break;
    case PROC_VALUE:
        break;
    default:
        break;
    }

    emit dataReceived();
}
void WebsocketServer::socketDisconnected()
{
    WebSocket *pClient = qobject_cast<WebSocket *>(sender());
    if (pClient) {
        disconnect(pClient->get(),
                   &QWebSocket::textMessageReceived,
                   this,
                   &WebsocketServer::processMessage);
        disconnect(pClient->get(),
                   &QWebSocket::disconnected,
                   this,
                   &WebsocketServer::socketDisconnected);
        _clients.removeAll(pClient);
        pClient->get()->deleteLater();
    }
}
