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

MQTTClient gClient;
queue<int16_t> gErrorQueue;
queue<MessageType*> gMessageQueue;

extern "C"
{
  __declspec(dllexport) uint32 OpenServer(char* clientName, char* ip, uint16 port, uint16 keepAlive)
  {
    uint32 returnValue = SC_SUCCESS;
    char url[MAX_STRING_LENGTH];
    sprintf_s(url, "%s:%d", ip, port);

    MQTTClient_connectOptions conn_opts = 
        MQTTClient_connectOptions_initializer;
    int rc;

    MQTTClient_create(&gClient, url, clientName,
      MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    if ((rc = MQTTClient_connect(gClient, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
      gErrorQueue.push(ER_COULD_NOT_CONNECT);
      returnValue = ER_COULD_NOT_CONNECT;
    }

    MQTTClient_setCallbacks(gClient, NULL, NULL, onMessage, NULL);

    return returnValue;
  }

  __declspec(dllexport) uint32 Publish(char* topic,
    uint8* message,
    uint16 messageLength,
    uint8 qos,
    uint8 retain)
  {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    pubmsg.payload = message;
    pubmsg.payloadlen = messageLength;
    pubmsg.qos = qos;
    pubmsg.retained = 0;
    MQTTClient_publishMessage(gClient, topic, &pubmsg, &token);
    rc = MQTTClient_waitForCompletion(gClient, token, 10000L);

    return SC_SUCCESS;
  }


  __declspec(dllexport) uint32 Subscribe(char* topic, int8 qos)
  {
    uint32 returnValue = SC_SUCCESS;
    MQTTClient_subscribe(gClient, topic, qos);
    return returnValue;
  }

  __declspec(dllexport) uint32 UnSubscribe(char* topic)
  {
    uint32 returnValue = SC_SUCCESS;
    MQTTClient_unsubscribe(gClient, topic);
    return returnValue;
  }

  __declspec(dllexport) uint32 CloseServer(void)
  {
    uint32 returnValue = SC_SUCCESS;

    MQTTClient_disconnect(gClient, 10000);
    MQTTClient_destroy(&gClient);
    return returnValue;
  }

  __declspec(dllexport) uint32 ReceiveMessage(
    uint8* message,
    uint16* messageLength,
    uint8* topic,
    uint16* topicLength,
    uint8* qos,
    uint8* retain)
  {
    if (gMessageQueue.empty())
    {
      return WN_NO_MORE_ROWS;
    }

    MessageType* localMessage;

    localMessage = (MessageType*)(gMessageQueue.front());

    *qos = localMessage->qos;
    *retain = localMessage->retain;

    // If the topic size is bigger than  the buffer, cut it down
    // If it's smaller then pass back the smaller size.
    uint16 theSize = *topicLength;
    if (theSize > localMessage->topicLength)
    {
      theSize = localMessage->topicLength;
      *topicLength = theSize;
    }

    for (uint16 i = 0; i < theSize; i++)
    {
      topic[i] = localMessage->topic[i];
    }

    // Do the same for the payload.
    theSize = *messageLength;
    if (theSize > localMessage->messageLength)
    {
      theSize = localMessage->messageLength;
      *messageLength = theSize;
    }

    for (uint16 i = 0; i < theSize; i++)
    {
      message[i] = localMessage->message[i];
    }

    free(localMessage->message);
    free(localMessage->topic);
    free(localMessage);

    gMessageQueue.pop();

    return SC_SUCCESS;

  }
}

int onMessage(void *context, char *topicName, int topicLen, MQTTClient_message *message)
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
