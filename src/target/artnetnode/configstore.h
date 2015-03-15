#pragma once

#include "global.h"

#include "artnet_protocol.h"


#define ARTNETNODECONFIG_VERSION 1

struct ArtNetNodeConfig
{
    ArtNet::MessageArtPoll::TalkToMe howToTalk;
    ArtNet::PriorityCode diagPriority : 8;
    uint16_t oemCode;
    uint16_t estaCode;
    char shortName[18];
    char longName[64];
};


extern struct ArtNetNodeConfig* const nodeCfg;
extern bool configChanged;

extern void initConfig();
extern void saveConfig();
extern void upgradeNodeConfig(int oldVersion, int oldSize);
