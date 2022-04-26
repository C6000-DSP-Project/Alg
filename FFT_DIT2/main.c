// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      基 2 时间抽取快速傅里叶变换/逆变换
//
//      2022年04月26日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    数字信号处理函数库
 *
 *    纯算法示例可以在软件仿真环境下测试(CCSv5 以上版本均不再支持软件仿真)
 *
 *    - 希望缄默(bin wang)
 *    - bin@corekernel.net
 *
 *    官网 corekernel.net/.org/.cn
 *    社区 fpga.net.cn
 *
 */
#include <stdio.h>

#include <math.h>

#include "mathlib.h"

#include "dsplib.h"


// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      宏定义
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 软件断点
#define SW_BREAKPOINT     asm(" SWBP 0 ");

// 采样点数
#define N   1024
// 采样频率
#define Fs  1024.0

// π 及 浮点数极小值
#define PI                3.14159
#define F_TOL             (1e-06)

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      全局变量
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 快速傅里叶变换测试
#pragma DATA_ALIGN(Input, 8);
float Input[N];         // 实数信号

#pragma DATA_ALIGN(FFTIn, 8);
float FFTIn[2 * N];     // 复数信号

#pragma DATA_ALIGN(FFTIn, 8);
float FFTInOrg[2 * N];  // 复数信号副本 用于作比较

#pragma DATA_ALIGN(FFTOut, 8);
float FFTOut[2 * N];    // 频域(FFT 结果)

#pragma DATA_ALIGN(w, 8);
float w[2 * N];         // 旋转因子

#pragma DATA_ALIGN(mo, 8);
float mo[N];            // 幅值

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      辅助函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 比特位反转
// 快速傅里叶蝶形变换后输出顺序与输入不一致
void bit_rev(float *x, int n)
{
    int i, j, k;
    float rtemp, itemp;
    j = 0;

    for(i = 1; i < (n - 1); i++)
    {
        k = n >> 1;

        while(k <= j)
        {
            j -= k;
            k >>= 1;
        }

        j += k;

        if(i < j)
        {
            rtemp = x[j * 2];
            x[j * 2] = x[i * 2];
            x[i * 2] = rtemp;
            itemp = x[j * 2 + 1];
            x[j * 2 + 1] = x[i * 2 + 1];
            x[i * 2 + 1] = itemp;
        }
    }
}

// 产生旋转因子
void gen_w_r2(float *w, int n)
{
    int i;

    float e = PI * 2.0 / n;

    for(i = 0; i < (n >> 1); i++)
    {
        w[2 * i] = cossp(i * e);
        w[2 * i + 1] = sinsp(i * e);
    }
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      主函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
int main()
{
    // 生成信号
    unsigned int i;
    for(i = 0; i < N; i++)
    {
        Input[i] = 5 * sinsp(2 * PI * 150 * (i / Fs)) + 10 * sinsp(2 * PI * 350 * (i / Fs));
    }

    // 实数转复数
    for(i = 0; i < N; i++)
    {
        FFTIn[2 * i] = Input[i];  // 实部
        FFTIn[2 * i + 1] = 0;     // 虚部
    }

    // 保留一份 FFT 输入副本
    memcpy(FFTInOrg, FFTIn, 2 * N * sizeof(float));

    // 相同点数 旋转因子仅需要生成一次
    gen_w_r2(w, N);
    bit_rev(w, N >> 1);

    // 快速傅里叶变换(输出在输入变量中)
    DSPF_sp_cfftr2_dit(FFTIn, w, N);

    // 保留一份 FFT 输出副本
    memcpy(FFTOut, FFTIn, 2 * N * sizeof(float));

    bit_rev(FFTIn, N);

    // 幅值计算
    for(i = 0; i < N; i++)
    {
        mo[i] = sqrtsp(FFTIn[2 * i] * FFTIn[2 * i] + FFTIn[2 * i + 1] * FFTIn[2 * i + 1]);

        if(i == 0)
        {
            mo[i] = mo[i] / N;  // 直流分量
        }
        else
        {
            mo[i] = mo[i] * 2 / N;
        }
    }

    // 快速傅里叶逆变换
    DSPF_sp_icfftr2_dif(FFTOut, w, N);

    for(i = 0; i < N; i++)
    {
        FFTOut[i] = FFTOut[i] / N;
    }

    printf("\n比较逆变换之后信号和原始型号: ");
    unsigned char count = 0;
    for(i = 0; i < N; i++)
    {
        if(abs(FFTOut[i] - FFTInOrg[i]) > F_TOL)
        {
            count++;
        }
    }

    count ? printf("不相同\n") : printf("相同\n");

    // 断点
    SW_BREAKPOINT;
}
