#include "global.h"
#include "app/main.h"
#include "target/artnetnode/configstore.h"
#include "target/artnetnode/network.h"
#include "target/artnetnode/dmx.h"
#include "target/artnetnode/misc.h"
#include "sys/util.h"

#include "artnet_protocol.h"

struct udp_pcb* artnet_udp;

bool configChanged;

union ReceivedPacket
{
    ReceivedPacket() {};
    uint32_t d32[0];
    uint8_t d8[0];
    ArtNet::MessageHeader header;
    ArtNet::MessageArtAddress address;
    ArtNet::MessageArtDmx dmx;
    ArtNet::MessageArtInput input;
    ArtNet::MessageArtIpProg ipProg;
    ArtNet::MessageArtPoll poll;
} receivedPacket;

void artnet_rx(void* dummy, struct udp_pcb* udp, struct pbuf* p, struct ip_addr* addr, u16_t port)
{
    do
    {
        int len = pbuf_copy_partial(p, &receivedPacket, sizeof(receivedPacket), 0);
        if (len < sizeof(ArtNet::MessageHeader)) break;
        ArtNet::MessageHeader* header = &receivedPacket.header;
        if (!header->isValid()) break;
        switch (receivedPacket.header.opCode)
        {
        case ArtNet::OPCODE_POLL:
        {
            if (len != sizeof(ArtNet::MessageArtPoll)) break;  // ArtDMX braucht da < statt !=

            nodeCfg->diagPriority = receivedPacket.poll.priority;
            nodeCfg->howToTalk    = receivedPacket.poll.talkToMe;

            configChanged = true;

            struct pbuf* buf = pbuf_alloc(PBUF_TRANSPORT, sizeof(ArtNet::MessageArtPollReply), PBUF_RAM);
            if (!buf) break;
            ArtNet::MessageArtPollReply* reply = (ArtNet::MessageArtPollReply*)buf->payload;
            *reply = ArtNet::MessageArtPollReply();

            reply->oem[1] = 0xff;
            reply->versInfo[1] = 0x01;
            reply->status1 = reply->status1 && 0b000010; // RDM
            reply->numPorts[1] = 0x04;

            reply->portTypes[0] = 0xc5; // Art-Net I/O
            reply->portTypes[1] = 0xc0; // DMX512 I/O
            reply->portTypes[2] = 0xc0; // DMX512 I/O
            reply->portTypes[3] = 0xc0; // DMX512 I/O
            //reply->swRemote = 0xff;

            reply->ipAddress[0] = ip4_addr1(&netIf.lwipIf.ip_addr);
            reply->ipAddress[1] = ip4_addr2(&netIf.lwipIf.ip_addr);
            reply->ipAddress[2] = ip4_addr3(&netIf.lwipIf.ip_addr);
            reply->ipAddress[3] = ip4_addr4(&netIf.lwipIf.ip_addr);
            memcpy(reply->mac, netIf.lwipIf.hwaddr, sizeof(reply->mac));
            reply->status2 = 0b00001110; // Art-Net 3, DHCP capable, DHCP used


            reply->style = ArtNet::STYLE_NODE;
            reply->shortName[0] = 'F';
            reply->shortName[1] = 'L';
            reply->shortName[2] = 'E';
            reply->shortName[3] = 'X';
            reply->shortName[4] = 'p';
            reply->shortName[5] = 'e';
            reply->shortName[6] = 'r';
            reply->shortName[7] = 'i';
            reply->shortName[8] = 'm';
            reply->shortName[9] = 'e';
            reply->shortName[10] = 'n';
            reply->shortName[11] = 't';
            reply->shortName[12] = '\0';

            udp_sendto_if(artnet_udp, buf, IP_ADDR_BROADCAST, 0x1936, &netIf.lwipIf);
            pbuf_free(buf);

            // was auch immer du damit machen willst
            break;
        }
        default:
            break;
        }
    }
    while (false);
    pbuf_free(p);
}

int main()
{
    earlyInit();
    initConfig();
    initDMX();
    initNetwork();
    lateInit();

    dhcp_start(&netIf.lwipIf);

    artnet_udp = udp_new();
    udp_bind(artnet_udp, IP_ADDR_ANY, 0x1936);
    udp_recv(artnet_udp, &artnet_rx, NULL);

    while (true)
    {
        doBackgroundWork();

        if (configChanged)
        {
            saveConfig();
            configChanged = false;
        }
    }
}

void dmxInUpdated(int channel)
{
    struct pbuf* buf = pbuf_alloc(PBUF_TRANSPORT, 18, PBUF_RAM);
    if (!buf) return;
    struct pbuf* payload = pbuf_alloc(PBUF_RAW, 512, PBUF_POOL);
    if (!payload)
    {
        pbuf_free(buf);
        return;
    }
    pbuf_take(payload, dmxInChannel[channel]->getInDataPtr(), 512);
    uint8_t* header = (uint8_t*)buf->payload;
    memcpy(header, "Art-Net\0\0\x50\0\0\0", 14);
    header[13] = channel;
    header[14] = 8 + channel;
    header[15] = 0;
    header[16] = 2;
    header[17] = 0;
    pbuf_cat(buf, payload);
    udp_sendto_if(artnet_udp, buf, IP_ADDR_BROADCAST, 0x1936, &netIf.lwipIf);
    pbuf_free(buf);
}
