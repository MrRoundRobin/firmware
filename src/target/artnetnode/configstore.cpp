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

       memcpy(nodeCfg->longName, "FLEXperiment", 13);
       memcpy(nodeCfg->shortName, "FLEXperiment", 13);

       oldVersion = 1;
       /* no break */

    case 1:
        nodeCfg->useDhcp = true;

        oldVersion = 2;
    }
}
