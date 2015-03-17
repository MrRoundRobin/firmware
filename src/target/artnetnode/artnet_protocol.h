#pragma once

#include "lwip/ip_addr.h"

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

        OPCODE_TOD_REQUEST = 0x8000,
        OPCODE_TOD_DATA = 0x8100,
        OPCODE_TOD_CONTROL = 0x8200,

        OPCODE_RDM = 0x8300,
        OPCODE_RDM_SUB = 0x8400,

        OPCODE_VIDEO_SETUP = 0xa010,
        OPCODE_VIDEO_PALETTE = 0xa020,
        OPCODE_VIDEO_DATA = 0xa040,

        OPCODE_MAC_MASTER = 0xf000,
        OPCODE_MAC_SLAVE = 0xf100,

        OPCODE_FIRMWARE_MASTER = 0xf200,
        OPCODE_FIRMWARE_REPLY = 0xf300,

        OPCODE_FILE_TN_MASTER = 0xf400,
        OPCODE_FILE_FN_MASTER = 0xf500,
        OPCODE_FILE_FN_REPLY = 0xf600,

        OPCODE_IPPROG = 0xf800,
        OPCODE_IPPROG_REPLY = 0xf900,

        OPCODE_MEDIA = 0x9000,
        OPCODE_MEDIA_PATCH = 0x9100,
        OPCODE_MEDIA_CONTROL = 0x9200,
        OPCODE_MEDIA_CONTROL_REPLY = 0x9300,

        OPCODE_TIMECODE = 0x9700,
        OPCODE_TIMESYNC = 0x9800,

        OPCODE_TRIGGER = 0x9900,

        OPCODE_DIRECTORY = 0x9a00,
        OPCODE_DIRECTORY_REPLY = 0x9b00
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

    enum AddressCommand
    {
        AC_NONE = 0x00,

        AC_CANCEL_MERGE = 0x01,

        AC_LED_NORMAL = 0x02,
        AC_LED_MUTE = 0x03,
        AC_LED_LOCATE = 0x04,

        AC_RESET_RXFLAGS = 0x05,

        AC_MERGE_LTP0 = 0x10,
        AC_MERGE_LTP1 = 0x11,
        AC_MERGE_LTP2 = 0x12,
        AC_MERGE_LTP3 = 0x13,

        AC_MERGE_HTP0 = 0x50,
        AC_MERGE_HTP1 = 0x51,
        AC_MERGE_HTP2 = 0x52,
        AC_MERGE_HTP3 = 0x53,

        AC_CLEAR_OP0 = 0x90,
        AC_CLEAR_OP1 = 0x91,
        AC_CLEAR_OP2 = 0x92,
        AC_CLEAR_OP3 = 0x93,
    };

    enum RdmVersion {
               RDMVERSION_DRAFT_1 = 0,
               RDMVERSION_STANDARD_1 = 1
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
        // Controller:  r-b
        // Node:        r--
        // Media:       r--
    public:
        MessageArtPoll() : MessageHeader(OPCODE_POLL), protVer{0x00, 0x0e} {}

        uint8_t protVer[2]; //Hi,Lo
        struct TalkToMe {
            uint8_t : 1;
            uint8_t pushMessages : 1;
            uint8_t sendDiagnostic : 1;
            uint8_t broadcast : 1;
            uint8_t : 4;
        } talkToMe;
        PriorityCode priority : 8;
    };

    class __attribute__((packed)) MessageArtPollReply : MessageHeader
    {
        // Controller: r-b
        // Node:       --b
        // Media:      --b
    public:
        MessageArtPollReply() : MessageHeader(OPCODE_POLL_REPLY), port(0x1936) {}

        uint8_t ipAddress[4];
        uint16_t port;
        uint8_t versInfo[2]; //Hi,Lo
        uint8_t netSwitch;
        uint8_t subSwitch;
        uint8_t oem[2]; //Hi,Lo
        uint8_t ubeaVersion;
        struct Status1 {
            uint8_t ubeaPresent : 1;
            uint8_t rdmCapable : 1;
            uint8_t romBooted : 1;
            uint8_t : 1;
            enum PortAddressAuthority {
                PortAddressAuthorityUnknown = 0,
                PortAddressAuthorityFrontpanel = 1,
                PortAddressAuthorityNetwork = 2
            } portAddressAuthority : 2;
            enum IndecatorState {
                IndecatorStateUnknown = 0,
                IndecatorStateLocate = 1,
                IndecatorStateMute = 2,
                IndecatorStateNormal = 3
            } indecatorState : 2;
        } status1;
        uint16_t estaMan;
        char shortName[18];
        char longName[64];
        uint8_t nodeReport[64];
        uint8_t numPorts[2]; //Hi,Lo
        struct PortTypes {
            enum Type {
                PortTypeDmx512 = 0,
                PortTypeMidi = 1,
                PortTypeAvab = 2,
                PortTypeColortranCmx = 3,
                PortTypeAdb625 = 4,
                PortTypeArtNet = 5
            } type : 6;
            uint8_t input : 1;
            uint8_t output : 1;
        } portTypes[4];
        struct GoodInput {
            uint8_t : 2;
            uint8_t errorsReceived : 1;
            uint8_t disabled : 1;
            uint8_t textPackages : 1;
            uint8_t sip : 1; // TODO: what's this?
            uint8_t testPackages : 1;
            uint8_t dataReceived : 1;
        } goodInput[4];
        struct GoodOutput {
            uint8_t : 1;
            uint8_t ltpMergeMode : 1;
            uint8_t shortDetected : 1; // TODO: what's this?
            uint8_t merging : 1;
            uint8_t textPackages : 1;
            uint8_t sip : 1;
            uint8_t testPackages : 1;
            uint8_t dataTransmited : 1;
        } goodOutputs[4];
        uint8_t swIn[4];
        uint8_t swOut[4];
        uint8_t swVideo;
        uint8_t swMacroBitmap;
        uint8_t swRemoteBitmap;
        uint32_t : 24; //unused
        StyleCode style : 8;
        uint8_t mac[6]; //Hi,...,Lo
        uint8_t bindIp[4];
        uint8_t bindIndex;
        struct Status2 {
                uint8_t supportsBrowserConfiguration : 1;
                uint8_t dhcpIp : 1;
                uint8_t dhcpCapable : 1;
                uint8_t supportsLargePortAddress : 1;
                uint8_t : 4;
        } status2;
        uint8_t filler[26]; //unused
    };

    class __attribute__((packed)) MessageArtDmx : MessageHeader
    {
        // Controller: ru-
        // Node:       ru-
        // Media:      ru-
    public:
        MessageArtDmx() : MessageHeader(OPCODE_DMX), protVer{0x00, 0x0e} {}

        uint8_t protVer[2]; //Hi,Lo
        uint8_t sequenze;
        uint8_t physical;
        uint16_t universe;
        uint8_t length[2]; //Hi,Lo
        uint8_t data[0];
    };

    class __attribute__((packed)) MessageArtIpProg : MessageHeader
    {
        // Controller: -u-
        // Node:       r--
        // Media:      r--
    public:
        MessageArtIpProg() : MessageHeader(OPCODE_IPPROG), protVer{0x00, 0x0e} {}

        uint8_t protVer[2]; //Hi,Lo
        uint16_t : 16; //unused
        struct Command {
            uint8_t programPort : 1;
            uint8_t programSubnet : 1;
            uint8_t programIp : 1;
            uint8_t resetNetConfig : 1;
            uint8_t : 2;
            uint8_t enableDhcp : 1;
            uint8_t programAny : 1;
        } command;
        uint8_t : 8; //unused
        ip_addr progIp;
        ip_addr progSubnet;
        uint8_t progPort[2]; //Hi,Lo
        uint64_t : 64;

        //TODO: find place for gateway
    };

    class __attribute__((packed)) MessageArtIpProgReply : MessageHeader
    {
        // Controller: r--
        // Node:       -u-
        // Media:      -u-
    public:
        MessageArtIpProgReply() : MessageHeader(OPCODE_IPPROG_REPLY), protVer{0x00, 0x0e} {}

        uint8_t protVer[2]; //Hi,Lo
        uint32_t : 32; //unused
        ip_addr progIp;
        ip_addr progSubnet;
        uint8_t progPort[2]; //Hi,Lo
        struct Status {
            uint8_t : 6;
            uint8_t dhcpEnabled : 1;
            uint8_t : 1;
        } status;
        uint64_t : 56; //unused
    };

    class __attribute__((packed)) MessageArtDiagData : MessageHeader
    {
        // Controller: rub
        // Node:       -ub
        // Media:      -ub
    public:
        MessageArtDiagData() : MessageHeader(OPCODE_DIAG_DATA), protVer{0x00, 0x0e} {}

        uint8_t protVer[2]; //Hi,Lo
        uint8_t : 8; //unused
        PriorityCode priority : 8;
        uint16_t : 16; //unused
        uint8_t length[2]; //Hi,Lo
        uint8_t data[0];
    };

    class __attribute__((packed)) MessageArtTimecode : MessageHeader
    {
        // Controller: rub
        // Node:       rub
        // Media:      rub
    public:
        MessageArtTimecode() : MessageHeader(OPCODE_TIMECODE), protVer{0x00, 0x0e} {}

        uint8_t protVer[2]; //Hi,Lo
        uint16_t : 16; //unused
        uint8_t frames;
        uint8_t seconds;
        uint8_t minutes;
        uint8_t hours;
        enum Type {
            TypeFilm = 0,       //24fps
            TypeEbu = 1,        //25fps
            TypeDf = 2,            //29.97fps
            TypeSmpte = 3   // 30fps
        } type : 8;
    };

    class __attribute__((packed)) MessageArtInput : MessageHeader
    {
        // Controller: -u-
        // Node:       r--
        // Media:      r--
    public:
        MessageArtInput() : MessageHeader(OPCODE_INPUT), protVer{0x00, 0x0e} {}

        uint8_t protVer[2]; //Hi,Lo
        uint16_t : 16; //unused
        uint8_t numPorts[2]; //Hi,Lo
        struct Input {
            uint8_t disable : 1;
            uint8_t : 7;
        } input[4];
    };

    class __attribute__((packed)) MessageArtAddress : MessageHeader
    {
        // Controller: -u-
        // Node:       r--
        // Media:      r--
    public:
        MessageArtAddress() : MessageHeader(OPCODE_ADDRESS), protVer{0x00, 0x0e} {}

        uint8_t protVer[2]; //Hi,Lo
        uint8_t netSwitch;
        uint8_t : 8; //unused
        char shortName[18];
        char longName[64];
        uint8_t swIn[4];
        uint8_t swOut[4];
        uint8_t subSwitch;
        uint8_t swVideo; // 'Reserved'
        AddressCommand command : 8;
    };

    class __attribute__((packed)) MessageArtTrigger : MessageHeader
    {
        // Controller: rub
        // Node:       rub
        // Media:      rub
        public:
            MessageArtTrigger() : MessageHeader(OPCODE_TRIGGER), protVer{0x00, 0x0e} {}

            uint8_t protVer[2]; //Hi,Lo
            uint8_t oemCode[2]; //Hi,Lo;
            enum Key {
                KeyAscii = 0,
                KeyMacro = 1,
                KeySoft = 2,
                KeyShow = 3
            } key : 8;
            uint8_t subKey;
            uint8_t data[0]; //max: 512
    };

    class __attribute__((packed)) MessageArtCommand : MessageHeader
    {
        // Controller: rub
        // Node:       rub
        // Media:      rub
    public:
        MessageArtCommand() : MessageHeader(OPCODE_COMMAND), protVer{0x00, 0x0e} {}

        uint8_t protVer[2]; //Hi,Lo
        uint8_t estaMan[2]; // Hi,Lo
        uint8_t length[2]; //Hi,Lo
        uint8_t data[0]; //max: 512
    };

    class __attribute__((packed)) MessageArtNzs : MessageHeader
    {
        // Controller: ru-
        // Node:       ru-
        // Media:      ru-
    public:
        MessageArtNzs() : MessageHeader(OPCODE_NZS), protVer{0x00, 0x0e} {}

        uint8_t protVer[2]; //Hi,Lo
        uint8_t sequenze;
        uint8_t startCode;
        uint16_t universe;
        uint8_t length[2]; //Hi,Lo
        uint8_t data[0]; //max: 512
    };

    class __attribute__((packed)) MessageArtFirmwareMaster : MessageHeader
     {
        // Controller: -u-
        // Node:       r--
        // Media:      r--
     public:
        MessageArtFirmwareMaster() : MessageHeader(OPCODE_FIRMWARE_MASTER), protVer{0x00, 0x0e} {}

        uint8_t protVer[2]; //Hi,Lo
        uint16_t : 16;
        enum Type {
            TypeFirmFirst = 0x00,
            TypeFirmCont = 0x01,
            TypeFirmLast = 0x02,
            TypeUbeaFirst = 0x03,
            TypeUbeaCont = 0x04,
            TypeUbeaLast = 0x05
        } type : 8;
        uint8_t blockId;
        uint8_t length[4]; //Hi,...,Lo
        uint32_t : 20;
        uint16_t data[512];
     };

    class __attribute__((packed)) MessageArtFirmwareReply : MessageHeader
     {
        // Controller: r--
        // Node:       -u-
        // Media:      -u-
     public:
        MessageArtFirmwareReply() : MessageHeader(OPCODE_FIRMWARE_REPLY), protVer{0x00, 0x0e} {}

        uint8_t protVer[2]; //Hi,Lo
        uint16_t : 16;
        enum Type {
            TypeFirmBlockGood = 0x00,
            TypeFirmAllGood = 0x01,
            TypeFirmFail = 0xff
        } type : 8;
        uint64_t : 64;
        uint64_t : 64;
        uint64_t : 40;
     };

    class __attribute__((packed)) MessageArtTodRequest : MessageHeader
     {
        // Controller: --b
        // Node:       r-b
        // Media:      ---
     public:
        MessageArtTodRequest() : MessageHeader(OPCODE_TOD_REQUEST), protVer{0x00, 0x0e} {}

        uint8_t protVer[2]; //Hi,Lo
        uint64_t : 64;
        uint8_t : 8;
        uint8_t net;
        enum Command {
            CommandTodFull = 0
        } command : 8;
        uint8_t addressLength;
        uint8_t address[0]; //max: 32
     };

    class __attribute__((packed)) MessageArtTodData : MessageHeader
     {
        // Controller: r--
        // Node:       r-b
        // Media:      ---
     public:
        MessageArtTodData() : MessageHeader(OPCODE_TOD_DATA), protVer{0x00, 0x0e} {}

        uint8_t protVer[2]; //Hi,Lo
        RdmVersion rdmVer : 8;
        uint8_t port;
        uint64_t : 56;
        uint8_t net;
        enum CommandResponse {
            CommandResponseTodFull = 0x00,
            CommandResponseTodNak = 0xff
        } commandResponse : 8;
        uint8_t address;
        uint8_t uidTotal[2]; //Hi, Lo
        uint8_t blockCount;
        uint8_t uidCount;
        //uid 48 * x
     };

    class __attribute__((packed)) MessageArtTodControl : MessageHeader
     {
        // Controller: --b
        // Node:       r-b
        // Media:      ---
     public:
        MessageArtTodControl() : MessageHeader(OPCODE_TOD_CONTROL), protVer{0x00, 0x0e} {}

        uint8_t protVer[2]; //Hi,Lo
        uint8_t : 8;
        uint64_t : 64;
        uint8_t net;
        enum Command {
            CommandActNone = 0,
            CommandActFlush = 1
        } command : 8;
        uint8_t address;
     };

    class __attribute__((packed)) MessageArtRdm : MessageHeader
     {
        // Controller: -ub
        // Node:       rub
        // Media:      ---
     public:
        MessageArtRdm() : MessageHeader(OPCODE_RDM), protVer{0x00, 0x0e} {}

        uint8_t protVer[2]; //Hi,Lo
        RdmVersion rdmVer : 8;
        uint64_t : 64;
        uint8_t net;
        enum Command {
            CommandArtProcess = 0
        } command : 8;
        uint8_t address;
        uint8_t rdmPacket[0]; //max: 512
     };

    class __attribute__((packed)) MessageArtRdmSub : MessageHeader
     {
        // Controller: ru-
        // Node:       ru-
        // Media:      ---
     public:
        MessageArtRdmSub() : MessageHeader(OPCODE_RDM_SUB), protVer{0x00, 0x0e} {}

        uint8_t protVer[2]; //Hi,Lo
        RdmVersion rdmVer : 8;
        uint8_t : 8;
        uint8_t uid[6];
        uint8_t : 8;
        uint8_t commandClass; //Todo: see RDM specs
        uint16_t parameterId;
        uint16_t subDevice;
        uint16_t subCount;
        uint32_t : 32;
        uint16_t data[0]; //size: 0/subCount
     };
}
