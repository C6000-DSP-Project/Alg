// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      混合基快速傅里叶变换/逆变换
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
float Input[N];             // 实数信号

#pragma DATA_ALIGN(FFTIn, 8);
float FFTIn[2 * N + 4];     // 复数信号

#pragma DATA_ALIGN(FFTInOrg, 8);
float FFTInOrg[2 * N + 4];  // 复数信号副本 用于作比较

#pragma DATA_ALIGN(FFTOut, 8);
float FFTOut[2 * N + 4];    // 频域(FFT 结果)

#pragma DATA_ALIGN(FFTOut, 8);
float IFFTOut[2 * N + 4];   // 时域(IFFT 结果)

#pragma DATA_ALIGN(w, 8);
float w[2 * N];             // 旋转因子

#pragma DATA_ALIGN(mo, 8);
float mo[N + 2];            // 幅值

// 二进制位翻转
// 快速傅里叶蝶形变换后输出顺序与输入不一致
#pragma DATA_ALIGN(brev, 8);
unsigned char brev[64]=
{
    0x0, 0x20, 0x10, 0x30, 0x8, 0x28, 0x18, 0x38,
    0x4, 0x24, 0x14, 0x34, 0xc, 0x2c, 0x1c, 0x3c,
    0x2, 0x22, 0x12, 0x32, 0xa, 0x2a, 0x1a, 0x3a,
    0x6, 0x26, 0x16, 0x36, 0xe, 0x2e, 0x1e, 0x3e,
    0x1, 0x21, 0x11, 0x31, 0x9, 0x29, 0x19, 0x39,
    0x5, 0x25, 0x15, 0x35, 0xd, 0x2d, 0x1d, 0x3d,
    0x3, 0x23, 0x13, 0x33, 0xb, 0x2b, 0x1b, 0x3b,
    0x7, 0x27, 0x17, 0x37, 0xf, 0x2f, 0x1f, 0x3f
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      辅助函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 产生旋转因子
void tw_gen(float *w, int n)
{
    int i, j, k;
    double x_t, y_t, theta1, theta2, theta3;

    for(j = 1, k = 0; j <= n >> 2; j = j << 2)
    {
        for(i = 0; i < n >> 2; i += j)
        {
            theta1 = 2 * PI * i / n;
            x_t = cos(theta1);
            y_t = sin(theta1);
            w[k] = (float)x_t;
            w[k + 1] = (float)y_t;

            theta2 = 4 * PI * i / n;
            x_t = cos(theta2);
            y_t = sin(theta2);
            w[k + 2] = (float)x_t;
            w[k + 3] = (float)y_t;

            theta3 = 6 * PI * i / n;
            x_t = cos(theta3);
            y_t = sin(theta3);
            w[k + 4] = (float)x_t;
            w[k + 5] = (float)y_t;
            k += 6;
        }
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

    // 确定基 单次计算最大支持 128K 复数点或 256K 实数点
    unsigned char rad;
    switch(N)
    {
        case 16:
        case 64:
        case 256:
        case 1024:
        case 4096:
        case 16384:
        case 65536: rad = 4; break;

        case 8:
        case 32:
        case 128:
        case 512:
        case 2048:
        case 8192:
        case 32768:
        case 131072: rad = 2; break;

        default:
            printf ("不支持计算 %d 点\n", N);
            SW_BREAKPOINT;
            break;
    }

    // 保留一份 FFT 输入副本
    memcpy(FFTInOrg, FFTIn, 2 * N * sizeof(float));

    // 相同点数 旋转因子仅需要生成一次
    tw_gen(w, N);

    // 快速傅里叶变换
    DSPF_sp_fftSPxSP(N, FFTIn, w, FFTOut, brev, rad, 0, N);

    // 幅值计算
    for(i = 0; i < N; i++)
    {
        mo[i] = sqrtsp(FFTOut[2 * i] * FFTOut[2 * i] + FFTOut[2 * i + 1] * FFTOut[2 * i + 1]);

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
    DSPF_sp_ifftSPxSP(N, FFTOut, w, IFFTOut, brev, rad, 0, N);

    printf("\n比较逆变换之后信号和原始型号: ");
    unsigned char count = 0;
    for(i = 0; i < N; i++)
    {
        if(abs(IFFTOut[i] - FFTInOrg[i]) > F_TOL)
        {
            count++;
        }
    }

    count ? printf("不相同\n") : printf("相同\n");

    // 断点
    SW_BREAKPOINT;
}
