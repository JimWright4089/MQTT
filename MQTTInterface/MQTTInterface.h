#include "MQTTAsync.h"
#include "MQTTClientPersistence.h"

typedef unsigned char uint8;
typedef char int8;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned long uint32;
typedef long int32;

using namespace std;

typedef struct Message
{
  char* topic;
  uint16 topicLength;
  uint8* message;
  uint16 messageLength;
  uint8 qos;
  uint8 retain;
} MessageType;

const int MAX_STRING_LENGTH = 256;
const int MAX_MESSAGE_LENGTH = 10000;

const int16 ER_LOST_CONNECTION = -1;
const int16 ER_COULD_NOT_CONNECT = -2;
const int16 ER_COUNT_NOT_RECONNECT = -3;
const int16 ER_COULD_NOT_SUBSCRIBE = -4;
const int16 ER_SUBSCRIBE_FAILED = -5;
const int16 ER_UNSUBSCRIBE_FAILED = -6;
const int16 ER_DISCONNECT_FAILED = -7;
const int16 ER_PUBLISH_FAILED = -8;
const int16 ER_COULD_NOT_UNSUBSCRIBE = -8;
const int16 WN_NO_MORE_ROWS = 1;
const int16 SC_SUCCESS = 1000;
const int16 SC_CONNECT_SUCCESS = 1001;
const int16 SC_SUBSCRIBE_SUCCESS = 1002;
const int16 SC_UNSUBSCRIBE_SUCCESS = 1003;
const int16 SC_PUBLISH_SUCCESS = 1004;

void onConnectFailure(void* context, MQTTAsync_failureData* response);
void onConnect(void* context, MQTTAsync_successData* response);
void onConnectionLost(void *context, char *cause);
int onMessageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message);
void onPublishFailure(void* context, MQTTAsync_failureData* response);
void onPublish(void* context, MQTTAsync_successData* response);
