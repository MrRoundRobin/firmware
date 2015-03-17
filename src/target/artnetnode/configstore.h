#pragma once

#include "global.h"

#include "artnet_protocol.h"
#include "lwip/ip_addr.h"


#define ARTNETNODECONFIG_VERSION 2

struct ArtNetNodeConfig
{
    //Version 1
    ArtNet::MessageArtPoll::TalkToMe howToTalk;
    ArtNet::PriorityCode diagPriority : 8;
    uint16_t oemCode;
    uint16_t estaCode;
    char shortName[18];
    char longName[64];

    //Version2
    uint8_t useDhcp;
    ip_addr ipAddress;
    ip_addr subnet;
    ip_addr gateway;
    ip_addr talkToMe;
};


extern struct ArtNetNodeConfig* const nodeCfg;
extern bool configChanged;

extern void initConfig();
extern void saveConfig();
extern void upgradeNodeConfig(int oldVersion, int oldSize);
