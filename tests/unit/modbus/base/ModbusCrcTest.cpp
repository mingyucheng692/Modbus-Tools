#include <gtest/gtest.h>
#include "modbus/base/ModbusCrc.h"
#include <QByteArray>
#include <vector>
#include <tuple>

using namespace modbus::base;

// Data-driven parameter structure for industrial standard testing
struct CrcTestCase {
    std::string hexData;
    uint16_t expectedCrc;
    std::string description;
};

class ModbusCrcParamTest : public ::testing::TestWithParam<CrcTestCase> {};

TEST_P(ModbusCrcParamTest, ValidatesVector) {
    const auto& param = GetParam();
    QByteArray data = QByteArray::fromHex(param.hexData.c_str());
    EXPECT_EQ(calculateModbusRtuCrc(data), param.expectedCrc) 
        << "Failed for: " << param.description;
}

// Industrial standard test vectors for Modbus RTU CRC-16
INSTANTIATE_TEST_SUITE_P(
    CommonVectors,
    ModbusCrcParamTest,
    ::testing::Values(
        CrcTestCase{"010300000001", 0x0A84, "Read Holding Registers Request"},
        CrcTestCase{"010304007B01C8", 0x2C8A, "Read Holding Registers Response"},
        CrcTestCase{"01", 0x807E, "Single Byte"},
        CrcTestCase{"", 0xFFFF, "Empty Data (Initial Value)"}
    )
);

TEST(ModbusCrcTest, LargeDataStability) {
    QByteArray large(256, 0x55);
    uint16_t crc1 = calculateModbusRtuCrc(large);
    uint16_t crc2 = calculateModbusRtuCrc(large);
    EXPECT_EQ(crc1, crc2);
}
