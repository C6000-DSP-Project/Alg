// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �º˿Ƽ�(����)���޹�˾
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ��ϻ����ٸ���Ҷ�任/��任��������ܲ���
//
//      2022��04��26��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
/*
 *    �����źŴ�������
 *
 *    ���㷨ʾ��������������滷���²���(CCSv5 ���ϰ汾������֧���������)
 *    ����������г��򲻼����ڴ�����ӳ� ����֤ DSP CPU ���ļ�������
 *
 *    - ϣ����Ĭ(bin wang)
 *    - bin@corekernel.net
 *
 *    ���� corekernel.net/.org/.cn
 *    ���� fpga.net.cn
 *
 */
#include <stdio.h>

#include <math.h>

#include "mathlib.h"

#include "dsplib.h"

#include "dspcache.h"

#include <c6x.h>

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      �궨��
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ����ϵ�
#define SW_BREAKPOINT     asm(" SWBP 0 ");

// ��������
#define N   131072
// ����Ƶ��
#define Fs  1024.0

// �� �� ��������Сֵ
#define PI                3.14159
#define F_TOL             (1e-06)

// �������
//  #define SIMULATION
#define ConsoleWrite       printf

// ���ٸ���Ҷ�任����ѡ��
#define FUN_DIT2           0
#define FUN_SPxSP          1

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ȫ�ֱ���
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ���ٸ���Ҷ�任����
#pragma DATA_ALIGN(Input, 8);
float Input[N];             // ʵ���ź�

#pragma DATA_ALIGN(FFTIn, 8);
float FFTIn[2 * N + 4];     // �����ź�

#pragma DATA_ALIGN(FFTOut, 8);
float FFTOut[2 * N + 4];    // Ƶ��(FFT ���)

#pragma DATA_ALIGN(FFTOut, 8);
float IFFTOut[2 * N + 4];   // ʱ��(IFFT ���)

#pragma DATA_ALIGN(w, 8);
float w[2 * N];             // ��ת����

#pragma DATA_ALIGN(mo, 8);
float mo[N + 2];            // ��ֵ

// ������λ��ת
// ���ٸ���Ҷ���α任�����˳�������벻һ��
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

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ���ܲ���
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
typedef struct
{
    unsigned int CPUClock;    // CPU ��Ƶ

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
    t->time = elapsed / 1.0 / (t->CPUClock / 1000 / 1000);  // ΢��
}

void Benchmark_Init(Benchmark_Result *t)
{
    // д����ֵ����
    TSCL = 0;

    // �������ʱ��
    Benchmark_Begin(t);
    Benchmark_End(t);

    t->cycleoverhead = t->cycle;
}

Benchmark_Result t;

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      FFT ����
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
// ����λ��ת(DSPF_sp_cfftr2_dit / DSPF_sp_icfftr2_dif)
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

// ������ת����(DSPF_sp_cfftr2_dit / DSPF_sp_icfftr2_dif)
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

// ������ת����(DSPF_sp_fftSPxSP / DSPF_sp_ifftSPxSP)
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

// ���ٸ���Ҷ�任
void FFT(unsigned char fun, float fs, unsigned int n, unsigned char rad)
{
    // ������ת����
    if(fun == FUN_DIT2)
    {
        gen_w_r2(w, n);
        bit_rev(w, n >> 1);
    }
    else
    {
        tw_gen(w, n);
    }

    // FFT ����
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

    // �������
    unsigned int i;
    for(i = 0; i < n; i++)
    {
        mo[i] = sqrtsp(FFTOut[2 * i] * FFTOut[2 * i] + FFTOut[2 * i +1 ] * FFTOut[2 * i + 1]);

        if(i == 0)
        {
            mo[i] = mo[i] / N;  // ֱ������
        }
        else
        {
            mo[i] = mo[i] * 2 / N;
        }
    }
}

// ���ٸ���Ҷ��任
void IFFT(unsigned char fun, unsigned int n, unsigned char rad)
{
    // ������ת����
    if(fun == FUN_DIT2)
    {
        gen_w_r2(w, n);
        bit_rev(w, n >> 1);
    }
    else
    {
        tw_gen(w, n);
    }

    // IFFT ����
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
    // �����ź�
    unsigned int i;
    for(i = 0; i < n; i++)
    {
        Input[i] = 5 * sinsp(2 * PI * 150 * (i / fs)) + 10 * sinsp(2 * PI * 350 * (i / fs));
    }

    // ʵ��ת����
    for(i = 0; i < n; i++)
    {
        FFTIn[2 * i] = Input[i];  // ʵ��
        FFTIn[2 * i + 1] = 0;     // �鲿
    }

    // FFT/IFFT ����
    ConsoleWrite("������������������������������������������������������������������������������������������������������������������������\n");
    ConsoleWrite("%d Point FFT Test\r\n", n);

    // ���ٸ���Ҷ�任����
    if(n >= 32 && n <= 32768)
    {
        FFT(FUN_DIT2, fs, n, NULL);
        FFT(FUN_SPxSP, fs, n, rad);
    }
    else
    {
        FFT(FUN_SPxSP, fs, n, rad);
    }

    // ���ٸ���Ҷ��任����
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

// ������������������������������������������������������������������������������������������������������������������������������������������������������
//
//      ������
//
// ������������������������������������������������������������������������������������������������������������������������������������������������������
int main()
{
    // ���ܲ��Գ�ʼ��
    t.CPUClock = 456000000;  // 456MHz
    Benchmark_Init(&t);

    // FFT ����
#ifndef SIMULATION
    // ����
    CacheDisableMAR((unsigned int)0xC0000000, (unsigned int)0x08000000);
#endif

    // ��ӡ˵��
    ConsoleWrite("\r\nInput y=2+3cos(2pi*50t-(30/180)pi)+1.5cos(2pi*75t+*(90/180)pi)\n");
    ConsoleWrite("�������������������������������������������� Cache Disabled ��������������������������������������������\n");

    // ���ٸ���Ҷ�任 / ���ٸ���Ҷ��任 ����
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

    ConsoleWrite("���������������������������������������������� Cache Enabled ��������������������������������������������\n");

    // ʹ�ܻ��� L1 �� L2
    CacheEnableMAR((unsigned int)0xC0000000, (unsigned int)0x08000000);
    CacheEnable(L1DCFG_L1DMODE_32K | L1PCFG_L1PMODE_32K | L2CFG_L2MODE_256K);


    // ���ٸ���Ҷ�任 / ���ٸ���Ҷ��任 ����
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

    // �ϵ�
    SW_BREAKPOINT;
}
