// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      数学函数库
//
//      2022年04月26日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
/*
 *    数学函数库示例
 *      sp 单精度浮点运算
 *      dp 双精度浮点运算
 *      mathlib 浮点计算 比 FastRTS 库效率更高
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

#include <math.h>   // FastRTS
#include <mathf.h>  // FastRTS

#include "mathlib.h"

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      宏定义
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// 软件断点
#define SW_BREAKPOINT     asm(" SWBP 0 ");

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      主函数
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
int main()
{
    printf("\n");

    // 除法
    printf("除法 单精度 2/3 = %f / 3/2 = %f\r\n", divsp(2, 3), divsp(3, 2));
    printf("除法 双精度 2/3 = %.15lf / 3/2 = %.15lf\r\n\r\n", divdp(2, 3), divdp(3, 2));

    // 返回小于或者等于指定表达式的最大整数
    printf("返回小于或者等于指定表达式的最大整数 RTS 库 %f\r\n\r\n", floorf(1.5));

    // 返回大于或者等于指定表达式的最小整数
    printf("返回大于或者等于指定表达式的最小整数 RTS 库 %f\r\n\r\n", ceilf(1.5));

    // 绝对值
    printf("绝对值 RTS 库 1.5 %f / -1.5 %f\r\n\r\n", fabsf(1.5), fabsf(-1.5));

    // 余数
    printf("余数 RTS 库 4.5 %% 3 %f\r\n\r\n", fmodf(4.5, 3));

    // 幂
    printf("幂 单精度 2^3 = %f / 2^1.5 = %f\r\n", powsp(2, 3), powsp(2, 1.5));
    printf("幂 双精度 2^3 = %.15lf / 2^1.5 = %.15lf\r\n", powdp(2, 3), powdp(2, 1.5));
    printf("幂 RTS 库 2^3 = %f / 2^1.5 = %f\r\n\r\n", powf(2, 3), powf(2, 1.5));

    // 倒数
    printf("倒数 单精度 0.5 %f\r\n", recipsp(0.5));
    printf("倒数 双精度 0.5 %.15lf\r\n\r\n", recipdp(0.5));

    // 开方
    printf("开方 单精度 3^2 + 4^2 %f\r\n", sqrtsp(3 * 3 + 4 *4));
    printf("开方 双精度 3^2 + 4^2 %.15lf\r\n", sqrtdp(3 * 3 + 4 *4));
    printf("开方 RTS 库 3^2 + 4^2 %f\r\n\r\n", sqrtf(3 * 3 + 4 *4));

    // 开方 倒数
    printf("开方 倒数 单精度 3^2 + 4^2 %f\r\n", rsqrtsp(3 * 3 + 4 *4));
    printf("开方 倒数 双精度 3^2 + 4^2 %.15lf\r\n\r\n", rsqrtdp(3 * 3 + 4 *4));

    // 对数
    printf("对数 以2为底 单精度 4 %f\r\n", log2sp(4));
    printf("对数 以2为底 双精度 4 %.15lf\r\n", log2dp(4));
    printf("对数 以2为底 RTS 库 4 %f\r\n", log2f(4));

    printf("对数 以e为底 单精度 e^2 %f\r\n", logsp(2.71828 * 2.71828));
    printf("对数 以e为底 双精度 e^2 %.15lf\r\n", logdp(2.71828 * 2.71828));

    printf("对数 以10为底 单精度 100 %f\r\n", log10sp(100));
    printf("对数 以10为底 双精度 100 %.15lf\r\n", log10dp(100));
    printf("对数 以10为底 RTS 库 100 %f\r\n\r\n", log10f(100));

    // 指数
    printf("指数 以2为底 单精度 4 %f\r\n", exp2sp(4));
    printf("指数 以2为底 双精度 4 %.15lf\r\n", exp2dp(4));
    printf("指数 以2为底 RTS 库 4 %f\r\n", exp2f(4));

    printf("指数 以e为底 单精度 4 %f\r\n", expsp(2));
    printf("指数 以e为底 双精度 4 %.15lf\r\n", expdp(2));

    printf("指数 以10为底 单精度 4 %f\r\n", exp10sp(4));
    printf("指数 以10为底 双精度 4 %.15lf\r\n", exp10dp(4));
    printf("指数 以10为底 RTS 库 4 %f\r\n\r\n", exp10f(4));

    // 三角函数 参数弧度
    // π = 6arcsin(1/2)
    printf("正弦 单精度 sin(π/6) %f\r\n", sinsp(6 * asin(0.5) / 6));
    printf("正弦 双精度 sin(π/6) %.15lf\r\n", sindp(6 * asin(0.5) / 6));
    printf("正弦 RTS 库 sin(π/6) %f\r\n", sinf(6 * asin(0.5) / 6));

    printf("余弦 单精度 cos(π/3) %f\r\n", cossp(6 * asin(0.5) / 3));
    printf("余弦 双精度 cos(π/3) %.15lf\r\n", cosdp(6 * asin(0.5) / 3));
    printf("余弦 RTS 库 cos(π/3) %f\r\n", cosf(6 * asin(0.5) / 3));

    printf("正切 RTS 库 tan(π/4) %f\r\n\r\n", tanf(6 * asin(0.5) / 4));

    // 双曲函数 参数弧度
    // π = 6arcsin(1/2)
    printf("双曲函数 单精度 恒等式 cosh(x)^2 - sinh(x)^2 = 1 %f\r\n\r\n", pow(coshf(6 * asin(0.5) / 6), 2) - pow(sinhf(6 * asin(0.5) / 6), 2));

    // 断点
    SW_BREAKPOINT;
}
