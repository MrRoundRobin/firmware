#include "global.h"
#include "device/nrf/se8r01/se8r01.h"
#include "sys/time.h"
#include "sys/util.h"


void NRF::SE8R01::configure(Configuration* config)
{
    NRF::SE8R01::Config configOff = { 0 };
    configOff.b.maskDataReceived = true;
    configOff.b.maskDataSent = true;
    configOff.b.maskMaxRetrans = true;
    writeReg(Reg_Config, &configOff, sizeof(configOff));
    flushTx();
    Status status = flushRx();
    writeReg(Reg_Status, &status, sizeof(status));

    writeReg(Reg_FeatureCtl, &config->featureCtl, sizeof(config->featureCtl));
    writeReg(Reg_AutoAckCtl, &config->autoAckCtl, sizeof(config->autoAckCtl));
    writeReg(Reg_RxPipeEnable, &config->rxPipeEnable, sizeof(config->rxPipeEnable));
    writeReg(Reg_AddressCtl, &config->addressCtl, sizeof(config->addressCtl));
    writeReg(Reg_RetransCtl, &config->retransCtl, sizeof(config->retransCtl));
    writeReg(Reg_RfChannel, &config->rfChannel, sizeof(config->rfChannel));
    writeReg(Reg_RfSetup, &config->rfSetup, sizeof(config->rfSetup));
    writeReg(Reg_DynLengthCtl, &config->dynLengthCtl, sizeof(config->dynLengthCtl));
    writeReg(Reg_Tuning, &config->tuning, sizeof(config->tuning));
    writeReg(Reg_GuardCtl, &config->guardCtl, sizeof(config->guardCtl));
    writeReg(Reg_Config, &config->config, sizeof(config->config));
}

NRF::SPI::Status NRF::SE8R01::handleIRQ()
{
    Status status = getStatus();
    uint8_t length = 0;
    uint8_t data[32];
    TxObserve txObserve;
    if (status.b.dataReceived)
    {
        getRxPacketSize(&length);
        readPacket(data, length);
    }
    if (packetTransmitted && (status.b.maxRetrans || status.b.dataSent))
        readReg(Reg_TxObserve, &txObserve, sizeof(txObserve));
    if (status.b.maxRetrans || status.b.dataSent || status.b.dataReceived)
        status = writeReg(Reg_Status, &status, sizeof(status));
    if (packetReceived && status.b.dataReceived)
        packetReceived(status.b.rxPipe, data, length);
    if (packetTransmitted && (status.b.maxRetrans || status.b.dataSent))
        packetTransmitted(!status.b.maxRetrans, txObserve.b.retransCount);
    return status;
}
