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
    ArtNet::MessageArtPoll poll;
    ArtNet::MessageArtIpProgReply pollReply;
    ArtNet::MessageArtDiagData diagData;
    ArtNet::MessageArtCommand command;
    ArtNet::MessageArtDmx dmx;
    ArtNet::MessageArtNzs nzs;
    ArtNet::MessageArtAddress address;
    ArtNet::MessageArtInput input;
    ArtNet::MessageArtTodRequest todRequest;
    ArtNet::MessageArtTodData todData;
    ArtNet::MessageArtTodControl todControl;
    ArtNet::MessageArtRdm rdm;
    ArtNet::MessageArtRdmSub rdmSub;
    ArtNet::MessageArtIpProg ipProg;
    ArtNet::MessageArtIpProgReply ipProgReply;
    ArtNet::MessageArtTimecode timecode;
    ArtNet::MessageArtTrigger trigger;
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

            if (nodeCfg->diagPriority != receivedPacket.poll.priority)
            {
                nodeCfg->diagPriority = receivedPacket.poll.priority;
                configChanged = true;
            }

            if (memcmp(&nodeCfg->howToTalk, &receivedPacket.poll.talkToMe, sizeof( nodeCfg->howToTalk)))
            {
                nodeCfg->howToTalk = receivedPacket.poll.talkToMe;
                configChanged = true;
            }

            struct pbuf* buf = pbuf_alloc(PBUF_TRANSPORT, sizeof(ArtNet::MessageArtPollReply), PBUF_RAM);
            if (!buf) break;
            memset(buf->payload, 0, sizeof(ArtNet::MessageArtPollReply));
            ArtNet::MessageArtPollReply* reply = new(buf->payload) ArtNet::MessageArtPollReply();

            setu16be(reply->oem, nodeCfg->oemCode);
            reply->estaMan = nodeCfg->estaCode;
            SETU16BE(reply->versInfo,  0x01);

            reply->status1.rdmCapable = true;

            SETU16BE(reply->numPorts, MAX(DMX_IN_CHANNELS, DMX_OUT_CHANNELS));

            reply->portTypes[0].input = true;
            reply->portTypes[0].type = ArtNet::MessageArtPollReply::PortTypes::PortTypeDmx512;

            memcpy(reply->ipAddress, &netIf.lwipIf.ip_addr, sizeof(reply->ipAddress));
            memcpy(reply->mac, netIf.lwipIf.hwaddr, sizeof(reply->mac));

            reply->status2.dhcpCapable = true;
            reply->status2.supportsLargePortAddress = true;
            reply->status2.dhcpIp = true;

            reply->style = ArtNet::STYLE_NODE;
            memcpy(reply->shortName, nodeCfg->shortName, sizeof(nodeCfg->shortName));
            memcpy(reply->longName, nodeCfg->longName, sizeof(nodeCfg->longName));

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
