#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_

#include <memory>

class Buffer;
class TimeStamp;
class ClientConnection;

typedef std::shared_ptr<ClientConnection> ClientConnectionPtr;
typedef std::function<void(const ClientConnectionPtr&)> CloseCallback;
typedef std::function<void(const ClientConnectionPtr&, Buffer*, TimeStamp)> MessageCallback;
typedef std::function<void(const ClientConnectionPtr&)> ConnectionCallback;


#endif
