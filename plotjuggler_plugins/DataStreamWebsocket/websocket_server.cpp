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
#include "bytesConversion.h"
#include "ui_websocket_server.h"
#include <chrono>
#include <mutex>
#include <string>
#include <QDebug>
#include <QDialog>
#include <QFile>
#include <QIntValidator>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>
#include <QWebSocket>
#define Init_byte '/'
#define end_Init_byte '@'
#define start_transfer_byte '*'
#define ready_byte '+'
#define init_start_byte '&'
#define time_length 4

//void bytesToFloat(uint8_t *bytes, int size, float &f)
//{
//    if (is_big_endian()) {
//        memcpy(&f, &bytes, 4);
//    } else {
//        uint32_t bytes4 = 0;
//        memcpy(&bytes4, bytes, size);
//        qDebug() << hex;
//        qDebug() << "bytes4 real:" << hex << bytes4;
//        uint32_t num = ((bytes4 & 0xFF000000) >> 24) + ((bytes4 & 0x00FF0000) >> 8)
//                       + ((bytes4 & 0x0000FF00) << 8) + ((bytes4 & 0x000000FF) << 24);
//        qDebug() << "bytes4 3 " << ((bytes4 & 0xFF000000) >> 24);
//        qDebug() << "bytes4 2 " << ((bytes4 & 0x00FF0000) >> 8);
//        qDebug() << "bytes4 1 " << ((bytes4 & 0x0000FF00) << 8);
//        qDebug() << "bytes4 0 " << ((bytes4 & 0x000000FF) << 24);
//        memcpy(&f, &num, size);
//    }
//}
//template<typename T>
//T qBytes4Convert(void *bytes, T &f)
//{
//    //it works for float so it should workd for other
//    if (is_big_endian()) {
//        memcpy(&f, &bytes, 4);
//    } else {
//        uint32_t bytes4 = 0;
//        memcpy(&bytes4, bytes, 4);
//        qDebug() << hex;
//        //qDebug()<<"bytes4 real:"<<hex<<bytes4<<endl;
//        uint32_t num = ((bytes4 & 0xFF000000) >> 24) + ((bytes4 & 0x00FF0000) >> 8)
//                       + ((bytes4 & 0x0000FF00) << 8) + ((bytes4 & 0x000000FF) << 24);
//        qDebug() << "bytes4 3 " << ((bytes4 & 0xFF000000) >> 24);
//        qDebug() << "bytes4 2 " << ((bytes4 & 0x00FF0000) >> 8);
//        qDebug() << "bytes4 1 " << ((bytes4 & 0x0000FF00) << 8);
//        qDebug() << "bytes4 0 " << ((bytes4 & 0x000000FF) << 24);
//        memcpy(&f, &num, 4);
//    }
//    return f;
//}
//template<typename T>
//T qBytes2Convert(void *bytes, T &f)
//{
//    //it works for float so it should workd for other
//    if (is_big_endian()) {
//        memcpy(&f, &bytes, 2);
//    } else {
//        uint32_t bytes4 = 0;
//        memcpy(&bytes4, bytes, 2);
//        qDebug() << hex;
//        //qDebug()<<"bytes4 real:"<<hex<<bytes4<<endl;
//        uint32_t num = ((bytes4 & 0xFF000000) >> 24) + ((bytes4 & 0x00FF0000) >> 8)
//                       + ((bytes4 & 0x0000FF00) << 8) + ((bytes4 & 0x000000FF) << 24);
//        qDebug() << "bytes2 1 " << ((bytes4 & 0x00FF) >> 8) << endl;
//        qDebug() << "bytes2 0 " << ((bytes4 & 0xFF00) << 8) << endl;
//        memcpy(&f, &num, 2);
//    }
//    return f;
//}

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
    QByteArray message = QString("*").toLocal8Bit();
    pSocket->get()->sendBinaryMessage(message);
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
            pSocket->pstate = READY;
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
            pSocket->pstate = PROC_INIT;
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

bool WebsocketServer::initcycl_proc(QByteArray message, WebSocket *pSocket)
{
    uint8_t strlen = 0;
    ObjectInfo *info;
    ENUMTYPE *eType;
    QString enum_name = "";
    bool enum_stored = 0;
    uint8_t enum_index = 0;
    qDebug() << "INIT SHIT" << endl;
    for (QByteArray::const_iterator iter = message.begin(); iter != message.end(); iter++) {
        if (*iter == end_Init_byte) {
            pSocket->proc_state = END_INIT;
            pSocket->prev_state = END_INIT;
            QByteArray message = QString("/").toLocal8Bit(); //send bitch
            pSocket->get()->sendBinaryMessage(message);
            //     return message;
        }
        switch (pSocket->proc_state) {
        case ID:
            info = new ObjectInfo();
            info->setId(*iter);
            qDebug() << "Id:" << uint8_t(*iter);
            pSocket->proc_state = STRLEN; //
            pSocket->prev_state = ID;
            break;
        case STRLEN:
            strlen = *iter;
            qDebug() << "Getlength:" << strlen;
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
            info->setTypeMem((ObjectType)(*iter));
            info->setSize((ObjectType)(*iter));
            qDebug() << "The Size of shit is : " << info->getSize() << endl;

            pSocket->get_objsRef().insert(
                pair<uint8_t, ObjectInfo *>(info->getId(), info)); //INSERT THE VARIABLE HERE

            if ((ObjectType)(uint8_t) *iter == ENUMTYPE_) {
                qDebug() << "found enum type" << endl;
                pSocket->prev_state = TYPE;
                pSocket->proc_state = STRLEN;
            }
            break;
        case ENUMNAME: //To do is test if enum name works
            pSocket->prev_state = ENUMNAME;
            enum_name = message.mid(iter - message.begin(), strlen);
            qDebug() << "Received ENUM Name" << enum_name;
            info->set_enumName(enum_name);
            enum_stored = (pSocket->get_enumRef().find(info->getName())
                           != pSocket->get_enumRef().end());
            iter = std::next(iter, strlen - 1);

            if (enum_stored) {
                pSocket->proc_state = ID;
                break;

            } else {
                eType = new ENUMTYPE{enum_name};
                pSocket->proc_state = STRLEN;
                pSocket->get_enumRef().insert(std::pair<QString, ENUMTYPE *>(eType->name, eType));
                break;
            }
            break;
        case ENUMELEM:
            qDebug() << "Received ENUMELEM" << message.mid(iter - message.begin(), strlen);
            pSocket->get_enumRef()[enum_name]->enum_Map.insert(
                pair<uint8_t, QString>(enum_index, message.mid(iter - message.begin(), strlen)));
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
            enum_index++;
            //will refractor later BITCH 8=======================================D!!!!!
            break;
        default:
            break;
        }
    }

    //debug show all objs
    for (map<uint8_t, ObjectInfo *>::const_iterator iter = pSocket->get_objsRef().begin();
         iter != pSocket->get_objsRef().end();
         iter++) {
        qDebug() << "ObjectInfos Name:" << iter->second->getName() << "|";
        qDebug() << "ObjectInfos Type:" << iter->second->getType() << "|";
        qDebug() << "ObjectInfos id:" << iter->second->getId() << "|";
    }
    //debug show all enum elements
    for (map<QString, ENUMTYPE *>::const_iterator enumref = pSocket->get_enumRef().begin();
         enumref != pSocket->get_enumRef().end();
         enumref++) {
        qDebug() << "Enum Name:" << enumref->second->name << "|";
        for (map<uint8_t, QString>::const_iterator iter = enumref->second->enum_Map.begin();
             iter != enumref->second->enum_Map.end();
             iter++) {
            qDebug() << "Enum Element id:" << iter->first << "|";
            qDebug() << "Enum Element Name:" << iter->second << "|";
        }
    }
    pSocket->proc_state = TIME;
    return true;
}

bool WebsocketServer::procc_data(QByteArray message, WebSocket *pSocket)
{
    std::string var_name;
    ObjectType type;
    float time;
    uint8_t id = 0;
    ObjectInfo *obj_ptr;
    uint8_t size = 0;
    PlotGroup::Ptr pltgrp_ptr;
    for (QByteArray::const_iterator iter = message.begin(); iter != message.end(); iter++) {
        switch (pSocket->proc_state) {
        case TIME:
            qBytes4Convert(message.mid(iter - message.begin(), time_length).data(),
                           &time); //memcpy time
            iter = next(iter, time_length - 1);
            qDebug() << "Received Time" << time;
            pSocket->proc_state = ID_TRAN;
            break;
        case ID_TRAN: //id -> var_name
            id = *iter;
            qDebug() << "Received ID Tran" << id;
            obj_ptr = pSocket->get_objsRef()[*iter];
            var_name = obj_ptr->getName().toStdString();
            type = obj_ptr->getType();
            pSocket->proc_state = DATABYTE;
            break;
        case DATABYTE:
            size = obj_ptr->getSize();
            qDebug() << "Tran var Size " << size << endl;
            if (!obj_ptr->isobj()) {
                auto &serial_numeric_plots = dataMap().numeric;
                auto target_plotIt = serial_numeric_plots.find(var_name);

                if (target_plotIt == serial_numeric_plots.end()) {
                    qDebug() << "name:" << QString::fromStdString(var_name);
                    dataMap().addNumeric(var_name);
                }
                target_plotIt = serial_numeric_plots.find(var_name);
                char *array = message.mid(iter - message.begin(), size).data();
                qDebug() << "Data message" << message << endl;
                obj_ptr->setBytestoValue(message.mid(iter - message.begin(), size).data());
                target_plotIt->second.pushBack({(double) time, obj_ptr->getValueInDouble()});

            } else {
                //plot_complicated shits like OBJS,ENUMS,BLAH BLAH B
                auto &serial_numeric_plots = dataMap().numeric;
                auto target_plotIt = serial_numeric_plots.find(var_name);

                if (target_plotIt == serial_numeric_plots.end()) {
                    qDebug() << "name:" << QString::fromStdString(var_name);
                    //                    pltgrp_ptr = make_shared<PJ::PlotGroup>(PJ::PlotGroup{var_name});
                    //for now assum only xytheta use here
                    obj_ptr->addPlotNumeric(&dataMap());
                }
                target_plotIt = serial_numeric_plots.find(var_name);
                char *array = message.mid(iter - message.begin(), size).data();
                qDebug() << "Data message" << message << endl;
                obj_ptr->setBytestoValue(message.mid(iter - message.begin(), size).data());
                qDebug() << "x" << ((XYTheta *) obj_ptr->getValueRef())->x_pos << endl;
                qDebug() << "y" << ((XYTheta *) obj_ptr->getValueRef())->y_pos << endl;
                qDebug() << "theta" << ((XYTheta *) obj_ptr->getValueRef())->theta << endl;
                double xyt[] = {double(((XYTheta *) obj_ptr->getValueRef())->x_pos),
                                double(((XYTheta *) obj_ptr->getValueRef())->y_pos),
                                double(((XYTheta *) obj_ptr->getValueRef())->theta)};
                obj_ptr->updatePlotNumeric(&dataMap(), xyt, time);
                //                pltgrp_ptr->setAttribute("x", ((XYTheta *) obj_ptr->getValueRef())->x_pos);
                //                pltgrp_ptr->setAttribute("y", ((XYTheta *) obj_ptr->getValueRef())->y_pos);
                //                pltgrp_ptr->setAttribute("theta", ((XYTheta *) obj_ptr->getValueRef())->theta);
                // target_plotIt->second.get()->attributes();
                //                target_plotIt->second
            }
            pSocket->proc_state = ID_TRAN;
            iter = next(iter, size - 1);
            break;
        default:
            break;
        }
    }
    pSocket->proc_state = TIME;

    return true;
}

void WebsocketServer::processBinaryMessage(QByteArray message, WebSocket *qsocket)
{
    std::lock_guard<std::mutex> lock(mutex());

    //    QByteArray Test;
    //    Test.append(0x41);
    //    Test.append(0x25);
    //    Test.append(0x53);
    //    Test.append(0xF8);
    //    float f = 0;
    //    qBytes4Convert(Test.data(), &f);
    //    qDebug() << "floaing point number" << f << endl;
    qDebug() << "Message byte array" << message << endl;
    QByteArray pass;

    switch (qsocket->pstate) {
    case INIT:
        pass = waitready_proc(message, qsocket);
        pass = waitinit_cyclproc(pass, qsocket);
        qsocket->pstate = (initcycl_proc(pass, qsocket)) ? TRANSFER : INIT;
        qDebug() << pass << "message for now" << endl;
        break;
    case TRANSFER: //for now only consider big endian
        procc_data(message, qsocket);
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
