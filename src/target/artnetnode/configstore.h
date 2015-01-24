#pragma once

#include "global.h"

#include "artnet_protocol.h"


#define ARTNETNODECONFIG_VERSION 0

struct ArtNetNodeConfig
{
    uint8_t howToTalk;
    ArtNet::PriorityCode diagPriority;
    uint8_t oemCode[2];
    uint8_t estaCode[2];
    char shortName[18];
    char lonName[64];
    uint8_t portTypes[4];
    uint8_t inputStatus[4];
    uint8_t outputStatus[4];
};


extern struct ArtNetNodeConfig* const nodeCfg;
extern bool configChanged;

extern void initConfig();
extern void saveConfig();
extern void upgradeNodeConfig(int oldVersion, int oldSize);
