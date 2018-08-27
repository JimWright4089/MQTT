#include "MQTTInterfaceDLL.h"
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

queue<MessageType*> gQueue;
struct mosquitto *gMosquitto;
bool gRun = true;

DLLExport uint32_t OpenServer(char* clientName, char* ip, uint16_t port, uint16_t keepAlive)
{
  char url[MAX_STRING_LENGTH];
  sprintf(url, "%s:%s", ip, port);

  rc = MQTTAsync_create(&gClient, url, opts.clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);

  MQTTAsync_setCallbacks(gClient, gClient, connectionLost, messageArrived, NULL);

  signal(SIGINT, cfinish);
  signal(SIGTERM, cfinish);

  conn_opts.keepAliveInterval = opts.keepalive;
  conn_opts.cleansession = 1;
  conn_opts.username = opts.username;
  conn_opts.password = opts.password;
  conn_opts.onSuccess = onConnect;
  conn_opts.onFailure = onConnectFailure;
  conn_opts.context = gClient;
  if ((rc = MQTTAsync_connect(gClient, &conn_opts)) != MQTTASYNC_SUCCESS)
  {
    printf("Failed to start connect, return code %d\n", rc);
    exit(EXIT_FAILURE);
  }
}

#ifdef Working

MQTT_API uint32_t CloseServer()
{
}


void onPublishFailure(void* context, MQTTAsync_failureData* response)
{
  printf("Publish failed, rc %d\n", response ? -1 : response->code);
  published = -1;
}


void onPublish(void* context, MQTTAsync_successData* response)
{
  published = 1;
}


int onMessageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
  if (opts.showtopics)
    printf("%s\t", topicName);
  if (opts.nodelimiter)
    printf("%.*s", message->payloadlen, (char*)message->payload);
  else
    printf("%.*s%c", message->payloadlen, (char*)message->payload, opts.delimiter);
  fflush(stdout);
  MQTTAsync_freeMessage(&message);
  MQTTAsync_free(topicName);
  return 1;
}


void onDisconnect(void* context, MQTTAsync_successData* response)
{
  disconnected = 1;
}


void onSubscribe(void* context, MQTTAsync_successData* response)
{
  subscribed = 1;
}


void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
  printf("Subscribe failed, rc %d\n", response->code);
  finished = 1;
}


void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
  printf("Connect failed, rc %d\n", response ? response->code : -99);
  finished = 1;
}


void onConnect(void* context, MQTTAsync_successData* response)
{
  MQTTAsync client = (MQTTAsync)context;
  MQTTAsync_responseOptions ropts = MQTTAsync_responseOptions_initializer;
  int rc;

  if (opts.showtopics)
    printf("Subscribing to topic %s with client %s at QoS %d\n", topic, opts.clientid, opts.qos);

  ropts.onSuccess = onSubscribe;
  ropts.onFailure = onSubscribeFailure;
  ropts.context = client;
  if ((rc = MQTTAsync_subscribe(client, topic, opts.qos, &ropts)) != MQTTASYNC_SUCCESS)
  {
    printf("Failed to start subscribe, return code %d\n", rc);
    finished = 1;
  }
}


MQTT_API uint32_t Publish(char* topic,
  uint8_t* message,
  uint16_t messageLength,
  uint8_t qos,
  uint8_t retain)
{
  return mosquitto_publish(gMosquitto, NULL, topic, messageLength, message, qos, (retain == 0) ? false : true);
}
MQTT_API uint32_t Subscribe(char* topic, int8_t qos)
{
  int mid;
  return mosquitto_subscribe(gMosquitto, &mid, topic, qos);
}
MQTT_API uint32_t UnSubscribe(char* topic)
{
  int mid;
  return mosquitto_unsubscribe(gMosquitto, &mid, topic);
}

MQTT_API uint32_t ReceiveMessage(uint16_t* messageID,
  uint8_t* message,
  uint16_t* messageLength,
  uint8_t* topic,
  uint16_t* topicLength,
  uint8_t* qos,
  uint8_t* retain)
{
  if (gQueue.empty())
  {
    return 1;
  }

  MessageType* localMessage;

  localMessage = (MessageType*)(gQueue.front());

  *messageID = localMessage->messageID;
  *qos = localMessage->qos;
  *retain = localMessage->retain;

  // If the topic size is bigger than  the buffer, cut it down
  // If it's smaller then pass back the smaller size.
  uint16_t theSize = *topicLength;
  if (theSize > localMessage->topicLength)
  {
    theSize = localMessage->topicLength;
    *topicLength = theSize;
  }

  for (uint16_t i = 0; i < theSize; i++)
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

  for (uint16_t i = 0; i < theSize; i++)
  {
    message[i] = localMessage->message[i];
  }

  free(localMessage->message);
  free(localMessage->topic);
  free(localMessage);

  gQueue.pop();

  return 0;
}

//----------------------------------------------------------------------------
//  Purpose:
//      Connect to the server call back
//
//  Notes:
//      None
//
//----------------------------------------------------------------------------
void connectCallback(struct mosquitto *mosq, void *obj, int rc)
{
  printf("connect rc: %d\n", rc);
}

//----------------------------------------------------------------------------
//  Purpose:
//      Something caused a dissconnect
//
//  Notes:
//      None
//
//----------------------------------------------------------------------------
void disconnectCallback(struct mosquitto *mosq, void *obj, int result)
{
  printf("disconnect rc: %d\n", result);
  gRun = false;
}

//----------------------------------------------------------------------------
//  Purpose:
//      Published has finished
//
//  Notes:
//      None
//
//----------------------------------------------------------------------------
void publishCallback(struct mosquitto *mosq, void *obj, int mid)
{
}

//----------------------------------------------------------------------------
//  Purpose:
//      Called when a message arrives
//
//  Notes:
//      None
//
//----------------------------------------------------------------------------
void messageCallback(struct mosquitto *mosq, void *obj,
  const struct mosquitto_message *message)
{

  MessageType* newMessage = (MessageType*)malloc(sizeof(MessageType));

  newMessage->retain = message->retain;
  newMessage->qos = message->qos;
  newMessage->messageID = message->mid;
  newMessage->messageLength = message->payloadlen;
  newMessage->message = (uint8_t*)malloc(newMessage->messageLength);
  memcpy(newMessage->message, message->payload, newMessage->messageLength);
  newMessage->topicLength = (uint16_t)strlen(message->topic);
  newMessage->topic = (char*)malloc(newMessage->topicLength);
  memcpy(newMessage->topic, message->topic, newMessage->topicLength);

  gQueue.push(newMessage);
}

MQTT_API uint32_t ReceiveLoop()
{
  return mosquitto_loop(gMosquitto, 10000, 1);
}

#endif