#include "global.h"
#include "sys/util.h"
#include "target/artnetnode/configstore.h"


void upgradeNodeConfig(int oldVersion, int oldSize)
{
    switch (oldVersion)
    {
    case 0:
       nodeCfg->diagPriority = ArtNet::PRIORITY_MED;
       nodeCfg->oemCode = 0x00ff;
       nodeCfg->estaCode = 0x7fff;
       nodeCfg->howToTalk.broadcast = true;

       SET_STRING(nodeCfg->longName, "FLEXperiment");
       SET_STRING(nodeCfg->shortName, "FLEXperiment");

       /* no break */

    case 1:
        nodeCfg->useDhcp = true;
    }
}
