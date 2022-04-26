// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      矩阵计算
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

#include "math.h"
#include "mathlib.h"

#include "dsplib.h"

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      宏定义
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 软件断点
#define SW_BREAKPOINT     asm(" SWBP 0 ");

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      全局变量
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 矩阵运算测试
// 叉乘运算（外积）矩阵B的行数必须等于矩阵A的列数！
// 点乘运算（内积）矩阵A的维数必须等于矩阵C的维数！
#define RA  4           // 矩阵 A 行数
#define CA  4           // 矩阵 A 列数

#define RB  4           // 矩阵 B 行数
#define CB  2           // 矩阵 B 列数

#define RC  4           // 矩阵 C 行数
#define CC  4           // 矩阵 C 列数

// 矩阵 A
#pragma DATA_ALIGN(A, 8);
float A[RA * CA] =
{
    1,  2,  3,  4,
    5,  6,  7,  8,
    9,  1,  2,  3,
    4,  5,  6,  7
};

// 矩阵 B
#pragma DATA_ALIGN(B, 8);
float B[RB * CB] =
{
    1,  2,
    3,  4,
    5,  6,
    7,  8
};

// 矩阵 C
#pragma DATA_ALIGN(C, 8);
float C[RC * CC] =
{
    2,  1,  0, -3,
    5,  0, -1,  9,
    4,  0,  6, -2,
    7,  0,  1,  8
};

// 输出
// 转置输出
#pragma DATA_ALIGN(MatTransA, 8);
float MatTransA[RA * CA];

// 叉乘(外积)运算输出
#pragma DATA_ALIGN(MatEPOut, 8);
float MatEPOut[RA * CB];

// 点乘(内积)运算输出
#pragma DATA_ALIGN(MatIPOut, 8);
float MatIPOut[RA * CA];

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      矩阵乘法
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 矩阵点乘(内积)运算函数
void MAT_mul_IP(float *x1, const int r1, const int c1, float *x2, float *restrict y)
{
    unsigned int i;

    for(i = 0; i < (r1 * c1); i++)
    {
        *(y + i) = (*(x1 + i)) * (*(x2 + i));
    }
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      主函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
int main()
{
    // 输出矩阵
    unsigned char i, j;

    printf("\r\n矩阵 A %d * %d\n", RA, CA);
    for(i = 0; i < RA; i++)
    {
        for(j = 0; j < CA ; j++)
        {
            printf("%.2f ", A[i * CA + j]);
        }
        printf("\r\n");
    }
    printf("\r\n\r\n");

    printf("矩阵 B %d * %d\n", RB, CB);
    for(i = 0; i < RB ; i++)
    {
        for(j = 0; j < CB; j++)
        {
            printf("%.2f ", B[i * CB + j]);
        }
        printf("\r\n");
    }
    printf("\r\n\r\n");

    printf("矩阵 C %d * %d\n", RC, CC);
    for(i = 0; i < RC; i++)
    {
        for(j = 0; j < CC ; j++)
        {
            printf("%.2f ", C[i * CC + j]);
        }
        printf("\r\n");
    }
    printf("\r\n\r\n");

    // 矩阵转置
    DSPF_sp_mat_trans(A, RA, CA, MatTransA);

    printf("矩阵 A 转置矩阵为 \n");
    for(i = 0; i < CA; i++)
    {
        for(j = 0; j < RA; j++)
        {
            printf("%.2f ", MatTransA[i * RA + j]);
        }
        printf("\r\n");
    }
    printf("\r\n\r\n");

    // 矩阵乘法
    // 矩阵 A 和矩阵 B 的外积(叉乘)
    DSPF_sp_mat_mul(A, RA, CA, B, CB, MatEPOut);

    printf("矩阵 A * B (外积)\n");
    for(i = 0; i < RA; i++)
    {
        for(j = 0; j < CB ; j++)
        {
            printf("%.2f ", MatEPOut[i * CB + j]);
        }
        printf("\r\n");
    }
    printf("\r\n\r\n");

    // 求矩阵A和矩阵C的内积(点乘)
    MAT_mul_IP(A, RA, CA, C, MatIPOut);

    // 打印内积（点乘）结果
    printf("矩阵 A .* B(内积)\n");
    for(i = 0; i < RA; i++)
    {
        for(j = 0; j < CA; j++)
        {
            printf("%.2f ", MatIPOut[i * CA + j]);
        }
        printf("\n");
    }
    printf("\r\n\r\n");

    // 向量数量积(点乘)
    float x[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    float y[8] = {1, 0, 1, 0, 1, 0, 1, 0};
    float sum;

    printf("向量 x\n");
    for(i = 0; i < 8; i++)
    {
        printf("%.2f ", x[i]);
    }
    printf("\n");

    printf("向量 y\n");
    for(i = 0; i < 8; i++)
    {
        printf("%.2f ", y[i]);
    }
    printf("\n");

    sum = DSPF_sp_dotprod(x, y, 8);
    printf("向量 x * y %.2f\n", sum);

    // 断点
    SW_BREAKPOINT;
}
