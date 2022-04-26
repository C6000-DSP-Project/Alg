// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      PRU 引导加载驱动
//
//      2022年03月30日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    - 希望缄默(bin wang)
 *    - bin@corekernel.net
 *
 *    官网 corekernel.net/.org/.cn
 *    社区 fpga.net.cn
 *
 */
#include "hw_types.h"
#include "hw_syscfg0_C6748.h"

#include "soc_C6748.h"
#include "psc.h"

#include "PRULoader.h"

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      PRU 使能/禁用
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
void PRUEnable(unsigned char PRUNum)
{
    // 唤醒 PRU 核心
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_PRU, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);

    // 复位 PRU 核心
    HWREG(PRU_REGS(PRUNum) + PRU_CONTROL) = 0;
}

void PRUDisable(unsigned char PRUNum)
{
    // 禁用 PRU0 CYCLECNT 计数 | PRU 停止取指
    HWREG(PRU_REGS(PRUNum) + PRU_CONTROL) &= ~(1 << 3) | (1 << 1);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      PRU 程序加载
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
unsigned int PRULoad(unsigned char PRUNum, unsigned int* PRUCode, unsigned int CodeSizeInWords, unsigned int* PRUData, unsigned int DataSizeInWords)
{
    // 使能 PRU 核心
    PRUEnable(PRUNum);

    // 复制指令
    unsigned int i;

    unsigned int *PRUIRAM = (unsigned int *)PRU_IRAM(PRUNum);
    for(i = 0; i < CodeSizeInWords; i++)
    {
        PRUIRAM[i] = PRUCode[i];
    }

    // 复制数据
    unsigned int *PRUDRAM = (unsigned int *)PRU_DRAM(PRUNum);
    for(i = 0; i < DataSizeInWords; i++)
    {
        PRUDRAM[i] = PRUData[i];
    }

    return TRUE;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      PRU 程序运行
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
unsigned int PRURun(unsigned char PRUNum)
{
    HWREG(PRU_REGS(PRUNum) + PRU_CONTROL) = (1 << 3) | (1 << 1);

    return TRUE;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      PRU 等待程序运行结束
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
unsigned int PRUWaitForHalt(unsigned char PRUNum, int timeout)
{
    // 暂停 PRU 程序运行 允许主机访问 PRU 调试寄存器和指令内存
    int cnt = timeout;

    while(HWREG(PRU_REGS(PRUNum) + PRU_CONTROL) & (1 << 15))
    {
        if(cnt > 0)
        {
            cnt--;
        }

        if(cnt == 0)
        {
            return FALSE;
        }
    }

	return TRUE;
}
