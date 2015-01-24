#include "global.h"
#include "app/main.h"
#include "target/artnetnode/configstore.h"
#include "target/artnetnode/network.h"
#include "target/artnetnode/dmx.h"
#include "target/artnetnode/misc.h"
#include "sys/util.h"

#include "artnet_protocol.h"

struct udp_pcb* artnet_udp;

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
        uint32_t len = pbuf_copy_partial(p, &receivedPacket, sizeof(receivedPacket), 0);
        if (len < sizeof(ArtNet::MessageHeader)) break;
        ArtNet::MessageHeader* header = &receivedPacket.header;
        if (!header->isValid()) break;
        switch (receivedPacket.header.opCode)
        {
        case ArtNet::OPCODE_POLL:
        {
            if (len != sizeof(ArtNet::MessageArtPoll)) break;

            nodeCfg->diagPriority = receivedPacket.poll.priority;
            nodeCfg->howToTalk    = receivedPacket.poll.talkToMe;

            configChanged = true;

            struct pbuf* buf = pbuf_alloc(PBUF_TRANSPORT, sizeof(ArtNet::MessageArtPollReply), PBUF_RAM);
            if (!buf) break;
            memset(buf->payload, 0, sizeof(ArtNet::MessageArtPollReply));
            ArtNet::MessageArtPollReply* reply = new(buf->payload) ArtNet::MessageArtPollReply();

            reply->oem[1] = 0xff;
            reply->versInfo[1] = 0x01;
            reply->status1 = reply->status1 && 0b000010; // RDM
            reply->numPorts[1] = 0x04;

            reply->portTypes[0] = 0xc0; // DMX512 I/O
            reply->portTypes[1] = 0xc0; // DMX512 I/O
            reply->portTypes[2] = 0xc0; // DMX512 I/O
            reply->portTypes[3] = 0xc0; // DMX512 I/O
            //reply->swRemote = 0xff;

            memcpy(reply->ipAddress, &netIf.lwipIf.ip_addr, sizeof(reply->ipAddress));
            memcpy(reply->mac, netIf.lwipIf.hwaddr, sizeof(reply->mac));
            reply->status2 = 0b00001110; // Art-Net 3, DHCP capable, DHCP used

            reply->style = ArtNet::STYLE_NODE;
            memcpy(reply->shortName, "FLEXperiment", 13);

            udp_sendto_if(artnet_udp, buf, IP_ADDR_BROADCAST, 0x1936, &netIf.lwipIf);
            pbuf_free(buf);
            break;
        }

        case ArtNet::OPCODE_DMX:
        {
            if (len < sizeof(ArtNet::MessageArtDmx)) break;
            //TODO: Implement
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
