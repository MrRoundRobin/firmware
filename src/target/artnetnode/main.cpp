#include "global.h"
#include "app/main.h"
#include "lwip/netif.h"
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
    ArtNet::MessageArtFirmwareMaster firmwareMaster;
    ArtNet::MessageArtFirmwareReply firmwareReply;
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

void apply_ip_config()
{
    if (nodeCfg->useDhcp)
    {
        dhcp_start(&netIf.lwipIf);
    }
    else
    {
        if (netIf.lwipIf.dhcp->state == DHCP_BOUND)
            dhcp_release(&netIf.lwipIf);
        dhcp_stop(&netIf.lwipIf);

        netif_set_ipaddr(&netIf.lwipIf, &nodeCfg->ipAddress);
        netif_set_netmask(&netIf.lwipIf, &nodeCfg->subnet);
    }
}

template<typename T> T* alloc_response(struct pbuf** buf, int size, pbuf_type pool)
{
    size += sizeof(T);
    *buf = pbuf_alloc(PBUF_TRANSPORT, size, pool);
    if (!*buf) return NULL;
    memset((*buf)->payload, 0, size);
    return new((*buf)->payload) T();
}

void artnet_send_poll_reply(ip_addr* destination)
{
    struct pbuf* buf;
    ArtNet::MessageArtPollReply* reply = alloc_response<typeof(*reply)>(&buf, 0, PBUF_RAM);
    if (!reply) return;

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

    udp_sendto_if(artnet_udp, buf, destination, 0x1936, &netIf.lwipIf);
    pbuf_free(buf);
}

void artnet_handle_poll(ArtNet::MessageArtPoll* req, struct ip_addr* addr, u16_t port)
{
    if (nodeCfg->diagPriority != req->priority)
    {
        nodeCfg->diagPriority = req->priority;
        configChanged = true;
    }

    if (memcmp(&nodeCfg->howToTalk, &req->talkToMe, sizeof(nodeCfg->howToTalk)))
    {
        nodeCfg->howToTalk = req->talkToMe;

        configChanged = true;
    }

    //TODO: handle multiple controllers
    artnet_send_poll_reply(IP_ADDR_BROADCAST);
}

void artnet_handle_ipprog(ArtNet::MessageArtIpProg* req, struct ip_addr* addr, u16_t port)
{
    if (req->command.enableDhcp || req->command.resetNetConfig)
    {
        if(!nodeCfg->useDhcp)
        {
            nodeCfg->useDhcp = req->command.enableDhcp;
            configChanged = true;
        }
    }
    else
    {
        if (nodeCfg->useDhcp)
        {
            nodeCfg->useDhcp = false;
            configChanged = true;
        }

        if ((req->command.programAny || req->command.programIp)
                && !ip_addr_cmp(&req->progIp, &nodeCfg->ipAddress))
        {
            nodeCfg->ipAddress = req->progIp;
            configChanged = true;
        }

        if ((req->command.programAny || req->command.programSubnet)
                && !ip_addr_cmp(&req->progSubnet, &nodeCfg->subnet))
        {
            nodeCfg->subnet = req->progSubnet;
            configChanged = true;
        }
    }

    struct pbuf* buf;
    ArtNet::MessageArtIpProgReply* reply = alloc_response<typeof(*reply)>(&buf, 0, PBUF_RAM);
    if (!reply) return;

    reply->progIp = nodeCfg->ipAddress;
    reply->progSubnet = nodeCfg->subnet;
    SETU16BE(reply->progPort, 0x1936);
    reply->status.dhcpEnabled = nodeCfg->useDhcp;

    udp_sendto_if(artnet_udp, buf, addr, 0x1936, &netIf.lwipIf);
    pbuf_free(buf);

    if (configChanged)
        apply_ip_config();
}

void artnet_handle_dmx(ArtNet::MessageArtDmx* req, struct ip_addr* addr, u16_t port)
{
    //TODO: implement
}

#define DEFINE_HANDLER(opcode, field, handler) case opcode: if (len == sizeof(typeof(field))) handler(&field, addr, port); break;
#define DEFINE_HANDLER_VARSIZE(opcode, field, size, handler) case opcode: if (len == sizeof(typeof(field)) + size) handler(&field, addr, port); break;

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
            DEFINE_HANDLER(ArtNet::OPCODE_POLL, receivedPacket.poll, artnet_handle_poll)
            DEFINE_HANDLER(ArtNet::OPCODE_IPPROG, receivedPacket.ipProg, artnet_handle_ipprog)
            DEFINE_HANDLER_VARSIZE(ArtNet::OPCODE_DMX, receivedPacket.dmx, GETU16BE(receivedPacket.dmx.length), artnet_handle_dmx)
            default: break;
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

    netIf.lwipIf.hostname = nodeCfg->shortName;
    apply_ip_config();

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
