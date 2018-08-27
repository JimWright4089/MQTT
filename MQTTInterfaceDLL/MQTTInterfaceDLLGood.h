//----------------------------------------------------------------------------
//
//  $Workfile: StopWatch.hpp$
//
//  $Revision: X$
//
//  Project:    multiple
//
//                            Copyright (c) 2017
//                                Jim Wright
//                            All Rights Reserved
//
//  Modification History:
//  $Log:
//  $
//
//----------------------------------------------------------------------------
#pragma once

#include "MQTTAsync.h"
#include "MQTTClientPersistence.h"

using namespace std;

const int MAX_STRING_LENGTH = 256;
const int MAX_MESSAGE_LENGTH = 10000;

extern "C"
{
  typedef struct Message
  {
    uint16_t messageID;
    char* topic;
    uint16_t topicLength;
    uint8_t* message;
    uint16_t messageLength;
    uint8_t qos;
    uint8_t retain;
  } MessageType;

  DLLExport uint32_t OpenServer(char* clientName, char* ip, uint16_t port, uint16_t keepAlive);

#ifdef INWORK
  DLLExport uint32_t CloseServer();
  DLLExport uint32_t ReceiveLoop();

  DLLExport uint32_t Publish(char* topic,
    uint8_t* message,
    uint16_t messageLength,
    uint8_t qos,
    uint8_t retain);
  DLLExport uint32_t Subscribe(char* topic, int8_t qos);
  DLLExport uint32_t UnSubscribe(char* topic);
  DLLExport uint32_t ReceiveMessage(uint16_t* messageID,
    uint8_t* message,
    uint16_t* messageLength,
    uint8_t* topic,
    uint16_t* topicLength,
    uint8_t* qos,
    uint8_t* retain);
}

//----------------------------------------------------------------------------
//  Functions
//----------------------------------------------------------------------------
void connectCallback(struct mosquitto *mosq, void *obj, int rc);
void disconnectCallback(struct mosquitto *mosq, void *obj, int result);
void messageCallback(struct mosquitto *mosq, void *obj,
  const struct mosquitto_message *message);
void ReceiveThead();

int onMessageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *message);
void onDisconnect(void* context, MQTTAsync_successData* response);
void onSubscribe(void* context, MQTTAsync_successData* response);
void onSubscribeFailure(void* context, MQTTAsync_failureData* response);
void onConnectFailure(void* context, MQTTAsync_failureData* response);
void onConnect(void* context, MQTTAsync_successData* response);
void onPublishFailure(void* context, MQTTAsync_failureData* response);
void onPublish(void* context, MQTTAsync_successData* response);
#endif
