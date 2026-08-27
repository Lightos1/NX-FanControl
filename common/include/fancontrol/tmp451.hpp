#ifndef __TMP451_H_
#define __TMP451_H_

#include <switch.h>

#define TMP451_PCB_TEMP_REG    0x00
#define TMP451_SOC_TEMP_REG    0x01
#define TMP451_SOC_TEMP_DEC_REG 0x10
#define TMP451_PCB_TEMP_DEC_REG 0x15

// Forward declarations
Result I2cReadRegHandler8(u8 reg, I2cDevice dev, u8 *out);

static inline Result Tmp451ReadReg(u8 reg, u8 *out)
{
	u8 data = 0;
	Result res = I2cReadRegHandler8(reg, I2cDevice_Tmp451, &data);

	if (R_FAILED(res))
	{
		return res;
	}

	*out = data;
	return res;
}

static inline Result Tmp451GetSocTemp(float* temperature) {
    u8 integer = 0;
    u8 decimals = 0;

    Result rc = Tmp451ReadReg(TMP451_SOC_TEMP_REG, &integer);
    if (R_FAILED(rc))
        return rc;
    rc = Tmp451ReadReg(TMP451_SOC_TEMP_DEC_REG, &decimals);
    if (R_FAILED(rc))
        return rc;
    
    decimals = ((u16)(decimals >> 4) * 625) / 100;
    *temperature = (float)(integer) + ((float)(decimals) / 100);
    return rc;
}

static inline Result Tmp451GetPcbTemp(float* temperature) {
    u8 integer = 0;
    u8 decimals = 0;

    Result rc = Tmp451ReadReg(TMP451_PCB_TEMP_REG, &integer);
    if (R_FAILED(rc))
        return rc;
    rc = Tmp451ReadReg(TMP451_PCB_TEMP_DEC_REG, &decimals);
    if (R_FAILED(rc))
        return rc;
    
    decimals = ((u16)(decimals >> 4) * 625) / 100;
    *temperature = (float)(integer) + ((float)(decimals) / 100);
    return rc;
}

#endif /* __TMP451_H_ */