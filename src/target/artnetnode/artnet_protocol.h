#pragma once

namespace ArtNet
{

    enum OpCode
    {
        OPCODE_POLL = 0x2000,
        OPCODE_POLL_REPLY = 0x2100,
        OPCODE_DIAG_DATA = 0x2300,
        OPCODE_COMMAND = 0x2400,
        OPCODE_DMX = 0x5000,
        OPCODE_NZS = 0x5100,
        OPCODE_ADDRESS = 0x6000,
        OPCODE_INPUT = 0x7000,

        //TODO: To be continued

        OPCODE_IPPROG = 0xf800,
        OPCODE_IPPROG_REPLY = 0xf900,

        //TODO: To be continued

        OPCODE_TIMECODE = 0x9700,
        OPCODE_TIMESYNC = 0x9800,
    };

    enum ReportCode
    {
        REPORT_DEBUG = 0x0000,
        REPORT_POWER_OK = 0x0001,
        REPORT_POWER_FAIL = 0x0002,
        REPORT_SOCKET_WR1 = 0x0003,
        REPORT_PARSE_FAIL = 0x0004,
        REPORT_UDP_FAIL = 0x0005,
        REPORT_SHORT_NAME_OK = 0x0006,
        REPORT_LONG_NAME_OK = 0x0007,
        REPORT_DMX_ERROR = 0x0008,
        REPORT_DMX_UDP_FULL = 0x0009,
        REPORT_DMX_RX_FULL = 0x000a,
        REPORT_SWITCH_ERROR = 0x000b,
        REPORT_CONFIG_ERROR = 0x000c,
        REPORT_DMX_SHORT = 0x000d,
        REPORT_FIRMWARE_FAIL = 0x000e,
        REPORT_USER_FAIL = 0x000f,
    };

    enum StyleCode
    {
        STYLE_NODE = 0x00,
        STYLE_CONTROLLER = 0x01,
        STYLE_MEDIA = 0x02,
        STYLE_ROUTE = 0x03,
        STYLE_BACKUP = 0x04,
        STYLE_CONFIG = 0x05,
        STYLE_VISUAL = 0x06,
    };

    enum PriorityCode
    {
        PRIORITY_LOW = 0x10,
        PRIORITY_MED = 0x40,
        PRIORITY_HIGH = 0x80,
        PRIORITY_CRITICAL = 0xe0,
        PRIORITY_VOLATILE = 0xf0,
    };

    class __attribute__((packed)) MessageHeader
    {
    public:
        MessageHeader(OpCode opCode) : id(0x74654e2d747241), opCode(opCode) {}

        uint64_t id;
        OpCode opCode : 16;

        inline bool isValid()
        {
          return this->id == 0x74654e2d747241;
        }
    };

    class __attribute__((packed)) MessageArtPoll : MessageHeader
    {
    public:
        MessageArtPoll() : MessageHeader(OPCODE_POLL) {}

        uint8_t protVer[2]; //Hi,Lo
        uint8_t talkToMe;
        PriorityCode priority : 8;
    };

    class __attribute__((packed)) MessageArtPollReply : MessageHeader
    {
    public:
        MessageArtPollReply() : MessageHeader(OPCODE_POLL_REPLY), port(0x1936) {}

        uint8_t ipAddress[4];
        uint16_t port;
        uint8_t versInfo[2]; //Hi,Lo
        uint8_t netSwitch;
        uint8_t subSwitch;
        uint8_t oem[2]; //Hi,Lo
        uint8_t ubeaVersion;
        uint8_t status1;
        uint16_t estaMan;
        char shortName[18];
        char longName[64];
        uint8_t nodeReport[64];
        uint8_t numPorts[2]; //Hi,Lo
        uint8_t portTypes[4];
        uint8_t goodInput[4];
        uint8_t goodOutputs[4];
        uint8_t swIn[4];
        uint8_t swOut[4];
        uint8_t swVideo;
        uint8_t swMacro;
        uint8_t swRemote;
        uint8_t spare[3]; //unused
        StyleCode style : 8;
        uint8_t mac[6]; //Hi,...,Lo
        uint8_t bindIp[4];
        uint8_t bindIndex;
        uint8_t status2;
        uint8_t filler[26]; //unused
    };

    class __attribute__((packed)) MessageArtDmx : MessageHeader
    {
    public:
        MessageArtDmx() : MessageHeader(OPCODE_DMX) {}

        uint8_t protVer[2]; //Hi,Lo
        uint8_t sequenze;
        uint8_t physical;
        uint16_t universe;
        uint8_t length[2]; //Hi,Lo
        uint8_t data[0];
    };

    class __attribute__((packed)) MessageArtIpProg : MessageHeader
    {
    public:
        MessageArtIpProg() : MessageHeader(OPCODE_IPPROG) {}

        uint8_t protVer[2]; //Hi,Lo
        uint8_t filler0[2]; //unused
        uint8_t command;
        uint8_t filler1; //unused
        uint8_t progIp[4]; //Hi,...,Lo
        uint8_t progSubnet[4]; //Hi,...,Lo
        uint8_t progPort[2]; //Hi,Lo
        uint8_t spare[8];
    };

    class __attribute__((packed)) MessageArtIpProgReply : MessageHeader
    {
    public:
        MessageArtIpProgReply() : MessageHeader(OPCODE_IPPROG_REPLY) {}

        uint8_t protVer[2]; //Hi,Lo
        uint8_t filler0[4]; //unused
        uint8_t progIp[4]; //Hi,...,Lo
        uint8_t progSubnet[4]; //Hi,...,Lo
        uint8_t progPort[2]; //Hi,Lo
        uint8_t status;
        uint8_t spare[7]; //unused
    };

    class __attribute__((packed)) MessageArtDiagData : MessageHeader
    {
    public:
        MessageArtDiagData() : MessageHeader(OPCODE_DIAG_DATA) {}

        uint8_t protVer[2]; //Hi,Lo
        uint8_t filler0; //unused
        PriorityCode priority : 8;
        uint8_t filler1[2]; //unused
        uint8_t length[2]; //Hi,Lo
        uint8_t data[0];
    };

    class __attribute__((packed)) MessageArtTimecode : MessageHeader
    {
    public:
        MessageArtTimecode() : MessageHeader(OPCODE_TIMECODE) {}

        uint8_t protVer[2]; //Hi,Lo
        uint8_t filler0[2]; //unused
        uint8_t frames;
        uint8_t seconds;
        uint8_t minutes;
        uint8_t hours;
        uint8_t type;
    };

    class __attribute__((packed)) MessageArtInput : MessageHeader
    {
    public:
        MessageArtInput() : MessageHeader(OPCODE_INPUT) {}

        uint8_t protVer[2]; //Hi,Lo
        uint8_t filler0[2]; //unused
        uint8_t numPorts[2]; //Hi,Lo
        uint8_t input[4];
    };

    class __attribute__((packed)) MessageArtAddress : MessageHeader
    {
    public:
        MessageArtAddress() : MessageHeader(OPCODE_ADDRESS) {}

        uint8_t protVer[2]; //Hi,Lo
        uint8_t netSwitch;
        uint8_t filler0; //unused
        char shortName[18];
        char longName[64];
        uint8_t swIn[4];
        uint8_t swOut[4];
        uint8_t subSwitch;
        uint8_t swVideo; // 'Reserved'
        uint8_t command;
    };
}
