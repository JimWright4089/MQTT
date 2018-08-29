#include "MQTTInterface.h"
#include <memory.h>
#include <string.h>
#include <queue>
#include <memory>
#include <thread> 

using namespace std;

struct
{
  char* clientid;
  int nodelimiter;
  char delimiter;
  int qos;
  char* username;
  char* password;
  char* host;
  char* port;
  int showtopics;
  int keepalive;
} opts =
{
  "stdout-subscriber-async", 1, '\n', 2, NULL, NULL, "localhost", "1883", 0, 10
};

MQTTAsync gClient;
MQTTAsync_connectOptions gConnectOptions =
  MQTTAsync_connectOptions_initializer;
queue<int16_t> gErrorQueue;
queue<MessageType*> gMessageQueue;


extern "C"
{
  __declspec(dllexport) uint32 OpenServer(char* clientName, char* ip, uint16 port, uint16 keepAlive)
  {
    uint32 returnValue = SC_SUCCESS;

    int rc = 0;
    char url[MAX_STRING_LENGTH];
    sprintf_s(url, "%s:%d", ip, port);

    rc = MQTTAsync_create(&gClient, url, opts.clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    MQTTAsync_setCallbacks(gClient, gClient,
      onConnectionLost,
      onMessageArrived, NULL);

    gConnectOptions.keepAliveInterval = opts.keepalive;
    gConnectOptions.cleansession = 1;
    gConnectOptions.username = opts.username;
    gConnectOptions.password = opts.password;
    gConnectOptions.onSuccess = onConnect;
    gConnectOptions.onFailure = onConnectFailure;
    gConnectOptions.context = gClient;
    if ((rc = MQTTAsync_connect(gClient, &gConnectOptions)) != MQTTASYNC_SUCCESS)
    {
      gErrorQueue.push(ER_COULD_NOT_CONNECT);
      returnValue = ER_COULD_NOT_CONNECT;
    }
    else
    {
      gErrorQueue.push(SC_SUCCESS);
    }

    return returnValue;
  }

  __declspec(dllexport) uint32 Publish(char* topic,
    uint8* message,
    uint16 messageLength,
    uint8 qos,
    uint8 retain)
  {
    MQTTAsync_responseOptions pub_opts = MQTTAsync_responseOptions_initializer;
    int rc = 0;

    pub_opts.onSuccess = onPublish;
    pub_opts.onFailure = onPublishFailure;
    pub_opts.context = gClient;
    do
    {
      rc = MQTTAsync_send(gClient, topic, messageLength,
        message, qos, retain, &pub_opts);  // To Do Add Timeout
    } while (rc != MQTTASYNC_SUCCESS);

    return SC_SUCCESS;
  }

}

void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
  gErrorQueue.push(ER_LOST_CONNECTION);
}

void onConnectionLost(void *context, char *cause)
{
  MQTTAsync client = (MQTTAsync)context;
  int rc;

  if ((rc = MQTTAsync_connect(client, &gConnectOptions)) != MQTTASYNC_SUCCESS)
  {
    gErrorQueue.push(ER_COUNT_NOT_RECONNECT);
  }
}

void onConnect(void* context, MQTTAsync_successData* response)
{
  gErrorQueue.push(SC_CONNECT_SUCCESS);
}

int onMessageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
  MessageType* newMessage = (MessageType*)malloc(sizeof(MessageType));

  newMessage->retain = message->retained;
  newMessage->qos = message->qos;
  newMessage->messageLength = message->payloadlen;
  newMessage->message = (uint8*)malloc(newMessage->messageLength);
  memcpy(newMessage->message, message->payload, newMessage->messageLength);
  newMessage->topicLength = (uint16)topicLen;
  newMessage->topic = (char*)malloc(topicLen);
  memcpy(newMessage->topic, topicName, newMessage->topicLength);

  gMessageQueue.push(newMessage);

  return 1;
}


void onPublishFailure(void* context, MQTTAsync_failureData* response)
{
  gErrorQueue.push(ER_PUBLISH_FAILED);
}

void onPublish(void* context, MQTTAsync_successData* response)
{
  gErrorQueue.push(SC_PUBLISH_SUCCESS);
}


#ifdef INWORK
#include <Windows.h>
#include "MQTTInterface.h"

extern "C"
{

  DLLExport uint32 OpenServer(char* clientName, char* ip, uint16 port, uint16 keepAlive)
  {
    MessageBox(0, "How are u?", "Hi", MB_ICONINFORMATION);
    return 0;
  }

  DLLExport uint32 Publish(char* topic,
    uint8* message,
    uint16 messageLength,
    uint8 qos,
    uint8 retain)
  {
    return 0;
  }

}
#endif