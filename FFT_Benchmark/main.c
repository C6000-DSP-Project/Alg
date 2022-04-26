// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      混合基快速傅里叶变换/逆变换多点数性能测试
//
//      2022年04月26日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    数字信号处理函数库
 *
 *    纯算法示例可以在软件仿真环境下测试(CCSv5 以上版本均不再支持软件仿真)
 *    软件仿真运行程序不计算内存访问延迟 仅验证 DSP CPU 核心计算性能
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

#include "dspcache.h"

#include <c6x.h>

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      宏定义
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 软件断点
#define SW_BREAKPOINT     asm(" SWBP 0 ");

// 采样点数
#define N   131072
// 采样频率
#define Fs  1024.0

// π 及 浮点数极小值
#define PI                3.14159
#define F_TOL             (1e-06)

// 调试输出
//  #define SIMULATION
#define ConsoleWrite       printf

// 快速傅里叶变换函数选择
#define FUN_DIT2           0
#define FUN_SPxSP          1

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
//      性能测试
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
typedef struct
{
    unsigned int CPUClock;    // CPU 主频

    unsigned long long cycleoverhead;
    unsigned long long cyclestart;

    unsigned long long cycle;
    double time;
} Benchmark_Result;

void Benchmark_Begin(Benchmark_Result *t)
{
    t->cyclestart = _itoll(TSCH, TSCL);
}

void Benchmark_End(Benchmark_Result *t)
{
    unsigned long long elapsed = _itoll(TSCH, TSCL) - t->cyclestart - t->cycleoverhead;

    t->cycle = elapsed;
    t->time = elapsed / 1.0 / (t->CPUClock / 1000 / 1000);  // 微秒
}

void Benchmark_Init(Benchmark_Result *t)
{
    // 写任意值启动
    TSCL = 0;

    // 计算操作时间
    Benchmark_Begin(t);
    Benchmark_End(t);

    t->cycleoverhead = t->cycle;
}

Benchmark_Result t;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      FFT 计算
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 比特位反转(DSPF_sp_cfftr2_dit / DSPF_sp_icfftr2_dif)
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

// 产生旋转因子(DSPF_sp_cfftr2_dit / DSPF_sp_icfftr2_dif)
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

// 产生旋转因子(DSPF_sp_fftSPxSP / DSPF_sp_ifftSPxSP)
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

// 快速傅里叶变换
void FFT(unsigned char fun, float fs, unsigned int n, unsigned char rad)
{
    // 产生旋转因子
    if(fun == FUN_DIT2)
    {
        gen_w_r2(w, n);
        bit_rev(w, n >> 1);
    }
    else
    {
        tw_gen(w, n);
    }

    // FFT 计算
    if(fun == FUN_DIT2)
    {
        Benchmark_Begin(&t);
        DSPF_sp_cfftr2_dit(FFTIn, w, n);
        Benchmark_End(&t);

        ConsoleWrite("DSPF_sp_cfftr2_dit FFT Cycle %lld Time %.2lfus\n", t.cycle, t.time);
    }
    else
    {
        Benchmark_Begin(&t);
        DSPF_sp_fftSPxSP(n, FFTIn, w, FFTOut, brev, rad, 0, n);
        Benchmark_End(&t);

        ConsoleWrite("DSPF_sp_fftSPxSP FFT Cycle %lld Time %.2lfus\n", t.cycle, t.time);
    }

    if(fun == FUN_DIT2)
    {
        bit_rev(FFTIn, n);
        memcpy(FFTOut, FFTIn, 4 * n);
    }

    // 计算振幅
    unsigned int i;
    for(i = 0; i < n; i++)
    {
        mo[i] = sqrtsp(FFTOut[2 * i] * FFTOut[2 * i] + FFTOut[2 * i +1 ] * FFTOut[2 * i + 1]);

        if(i == 0)
        {
            mo[i] = mo[i] / N;  // 直流分量
        }
        else
        {
            mo[i] = mo[i] * 2 / N;
        }
    }
}

// 快速傅里叶逆变换
void IFFT(unsigned char fun, unsigned int n, unsigned char rad)
{
    // 产生旋转因子
    if(fun == FUN_DIT2)
    {
        gen_w_r2(w, n);
        bit_rev(w, n >> 1);
    }
    else
    {
        tw_gen(w, n);
    }

    // IFFT 计算
    if(fun == FUN_DIT2)
    {
        Benchmark_Begin(&t);
        DSPF_sp_icfftr2_dif(FFTOut, w, n);
        Benchmark_End(&t);

        ConsoleWrite("DSPF_sp_icfftr2_dif IFFT Cycle %lld Time %.2lfus\n", t.cycle, t.time);
    }
    else
    {
        Benchmark_Begin(&t);
        DSPF_sp_ifftSPxSP(n, FFTOut, w, IFFTOut, brev, rad, 0, n);
        Benchmark_End(&t);

        ConsoleWrite("DSPF_sp_ifftSPxSP IFFT Cycle %lld Time %.2lfus\n", t.cycle, t.time);
    }

    if(fun == FUN_DIT2)
    {
        unsigned int i;

        memcpy(IFFTOut, FFTOut, 4 * n);

        for(i = 0; i < n; i++)
        {
            IFFTOut[i] = FFTOut[i] / n;
        }
    }
}

void FFTBenchmark(unsigned char index, float fs, unsigned int n, unsigned char rad)
{
    // 生成信号
    unsigned int i;
    for(i = 0; i < n; i++)
    {
        Input[i] = 5 * sinsp(2 * PI * 150 * (i / fs)) + 10 * sinsp(2 * PI * 350 * (i / fs));
    }

    // 实数转复数
    for(i = 0; i < n; i++)
    {
        FFTIn[2 * i] = Input[i];  // 实部
        FFTIn[2 * i + 1] = 0;     // 虚部
    }

    // FFT/IFFT 测试
    ConsoleWrite("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    ConsoleWrite("%d Point FFT Test\r\n", n);

    // 快速傅里叶变换测试
    if(n >= 32 && n <= 32768)
    {
        FFT(FUN_DIT2, fs, n, NULL);
        FFT(FUN_SPxSP, fs, n, rad);
    }
    else
    {
        FFT(FUN_SPxSP, fs, n, rad);
    }

    // 快速傅里叶逆变换测试
    if(n >= 32 && n <= 32768)
    {
        IFFT(FUN_DIT2, n, NULL);
        IFFT(FUN_SPxSP, n, rad);
    }
    else
    {
        IFFT(FUN_SPxSP, n, rad);
    }
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      主函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
int main()
{
    // 性能测试初始化
    t.CPUClock = 456000000;  // 456MHz
    Benchmark_Init(&t);

    // FFT 测试
#ifndef SIMULATION
    // 禁用
    CacheDisableMAR((unsigned int)0xC0000000, (unsigned int)0x08000000);
#endif

    // 打印说明
    ConsoleWrite("\r\nInput y=2+3cos(2pi*50t-(30/180)pi)+1.5cos(2pi*75t+*(90/180)pi)\n");
    ConsoleWrite("━━━━━━━━━━━━━━━━━━━━━━ Cache Disabled ━━━━━━━━━━━━━━━━━━━━━━\n");

    // 快速傅里叶变换 / 快速傅里叶逆变换 测试
    FFTBenchmark(1,       8.0,      8, 2);
    FFTBenchmark(2,      16.0,     16, 4);
    FFTBenchmark(3,      32.0,     32, 2);
    FFTBenchmark(4,      64.0,     64, 4);
    FFTBenchmark(5,     128.0,    128, 2);
    FFTBenchmark(6,     256.0,    256, 4);
    FFTBenchmark(7,     512.0,    512, 2);
    FFTBenchmark(8,    1024.0,   1024, 4);
    FFTBenchmark(9,    2048.0,   2048, 2);
    FFTBenchmark(10,   4096.0,   4096, 4);
    FFTBenchmark(11,   8192.0,   8192, 2);
    FFTBenchmark(12,  16384.0,  16384, 4);
    FFTBenchmark(13,  32768.0,  32768, 2);
    FFTBenchmark(14,  65536.0,  65536, 4);
    FFTBenchmark(15, 131072.0, 131072, 2);

#ifndef SIMULATION

    ConsoleWrite("━━━━━━━━━━━━━━━━━━━━━━━ Cache Enabled ━━━━━━━━━━━━━━━━━━━━━━\n");

    // 使能缓存 L1 及 L2
    CacheEnableMAR((unsigned int)0xC0000000, (unsigned int)0x08000000);
    CacheEnable(L1DCFG_L1DMODE_32K | L1PCFG_L1PMODE_32K | L2CFG_L2MODE_256K);


    // 快速傅里叶变换 / 快速傅里叶逆变换 测试
    FFTBenchmark(1,       8.0,      8, 2);
    FFTBenchmark(2,      16.0,     16, 4);
    FFTBenchmark(3,      32.0,     32, 2);
    FFTBenchmark(4,      64.0,     64, 4);
    FFTBenchmark(5,     128.0,    128, 2);
    FFTBenchmark(6,     256.0,    256, 4);
    FFTBenchmark(7,     512.0,    512, 2);
    FFTBenchmark(8,    1024.0,   1024, 4);
    FFTBenchmark(9,    2048.0,   2048, 2);
    FFTBenchmark(10,   4096.0,   4096, 4);
    FFTBenchmark(11,   8192.0,   8192, 2);
    FFTBenchmark(12,  16384.0,  16384, 4);
    FFTBenchmark(13,  32768.0,  32768, 2);
    FFTBenchmark(14,  65536.0,  65536, 4);
    FFTBenchmark(15, 131072.0, 131072, 2);
#endif

    // 断点
    SW_BREAKPOINT;
}
