/*DataStreamServer PlotJuggler  Plugin license(Faircode)

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
#pragma once

#include <QWebSocketServer>
#include <QWebSocket>
#include <QList>

#include "ObjectInfo.h"
#include "PlotJuggler/datastreamer_base.h"
#include "PlotJuggler/messageparser_base.h"
#include <thread>
#include <QObject>
#include <QtPlugin>
//#define INIT_BYTE '/'
//#define READ_BYTE '+'
//#define INIT_START_BYTE '&'
//#define INIT_
//#define Id
//#define var_strlen
//#define strlen of type type
//#define enumname
typedef enum {

    INIT,
    READY,
    PROC_INIT,
    TRANSFER

} ProcessState;
typedef enum {
    ID,
    STRLEN,
    VAR_NAME,
    TYPE,
    ENUMNAME,
    ENUMELEM,
    END_INIT,
    ID_TRAN,
    TIME,
    DATABYTE
} ProcessSubstate;

using namespace PJ;

class WebSocket : public QObject
{
    Q_OBJECT
    QWebSocket *websocket;
    std::map<uint8_t, ObjectInfo *> objs;
    std::map<QString, ENUMTYPE *> enums;

public:
    ProcessState pstate = INIT;
    ProcessSubstate proc_state = ID;
    ProcessSubstate prev_state = ID;
    explicit WebSocket(QWebSocket *web, QObject *parent = nullptr) : QObject(parent), websocket(web)
    {
        connect(websocket, &QWebSocket::binaryMessageReceived, this, &WebSocket::emitbinaryReceived);
    };
    ~WebSocket()
    {
        qDebug() << "BRAH DESTRUCT ME" << endl;
        if (websocket == nullptr)
            delete websocket;
    }
    void set(QWebSocket *socketptr) { websocket = socketptr; }
    QWebSocket *get() { return websocket; }
    std::map<uint8_t, ObjectInfo *> &get_objsRef() { return objs; };
    std::map<QString, ENUMTYPE *> &get_enumRef() { return enums; };

signals:
    void binaryReceived(QByteArray byte, WebSocket *socket);
public slots:
    void emitbinaryReceived(QByteArray byte)
    {
        emit binaryReceived(byte, this);
        qDebug() << "runing emit binary" << endl;
    };
};

class WebsocketServer : public PJ::DataStreamer
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "facontidavide.PlotJuggler3.DataStreamer")
  Q_INTERFACES(PJ::DataStreamer)

public:
  WebsocketServer();

  virtual bool start(QStringList*) override;

  virtual void shutdown() override;

  virtual bool isRunning() const override
  {
    return _running;
  }

  virtual ~WebsocketServer() override;

  virtual const char* name() const override
  {
    return "WebSocket Server";
  }

  virtual bool isDebugPlugin() override { return false; }

  private:
  bool _running;
  QList<WebSocket *> _clients;
  //std::map<QString, QWebSocket *> _clients;
  QWebSocketServer _server;
  PJ::MessageParserPtr _parser;
  // ProcessState pstate = INIT;
  QByteArray waitready_proc(QByteArray, WebSocket *);
  QByteArray waitinit_cyclproc(QByteArray, WebSocket *);
  bool initcycl_proc(QByteArray, WebSocket *);
  bool procc_data(QByteArray, WebSocket *);
  //  QByteArray plot_raw_data(, WebSocket *);
  private slots:
  void onNewConnection();
  void processMessage(QString message);
  void processBinaryMessage(QByteArray, WebSocket *);
  void socketDisconnected();
};


