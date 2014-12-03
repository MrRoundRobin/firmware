#include "global.h"
#include "target/artnetnode/network.h"
#include "sys/util.h"
#include "sys/serialnum.h"
#include "soc/stm32/f1/ethmac.h"
#include "device/lan8720/lan8720.h"


class MacAddressGenerator
{
public:
    uint8_t address[6];

    MacAddressGenerator()
    {
        uint32_t serial = serialnum_chip();
        address[0] = 0x12;
        address[1] = 0xef;
        address[2] = serial >> 24;
        address[3] = serial >> 16;
        address[4] = serial >> 8;
        address[5] = serial;
    }
};


static MacAddressGenerator addrGen = MacAddressGenerator();
static STM32::ETHMAC netMAC = STM32::ETHMAC(Ethernet::MII::TYPE_RMII);
static LAN8720 netPHY = LAN8720(&netMAC, 0);
static Ethernet::MII::Driver netDriver = Ethernet::MII::Driver(&netMAC, &netPHY);
Ethernet::Interface netIf = Ethernet::Interface(&netDriver, 1500, addrGen.address);


void initNetwork()
{
}
