// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      新核科技(广州)有限公司
//
//      Copyright (C) 2022 CoreKernel Technology Guangzhou Co., Ltd
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//
//      OMAP-L138 及 DSP C6748 内存空间分配定义
//
//      2014年07月05日
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
-heap  0x1000  // 堆
-stack 0x1000  // 栈

MEMORY
{
    DSPL2RAM    org = 0x00800000    len = 0x00040000  // 256KB L2RAM */

    SHRAM       org = 0x80000000    len = 0x00020000  // 128KB 共享 RAM */
    DDR2        org = 0xC0000000    len = 0x08000000  // 128MB DDR2 */
}

SECTIONS
{
    .text			>  DSPL2RAM			      		  // 可执行代码
    .stack  		>  DSPL2RAM		 			      // 软件系统栈

    .cio			>  DSPL2RAM		 			      // C 输入输出缓存
    .vectors		>  DSPL2RAM		 			      // 中断向量表
    .const			>  DSPL2RAM		 			      // 常量
    .data			>  DSPL2RAM		 			      // 已初始化全局及静态变量
    .switch			>  DSPL2RAM		 			      // 跳转表
    .sysmem			>  DSPL2RAM		 			      // 动态内存分配区域

    .pinit			>  DSPL2RAM					      // C++ 结构表
    .cinit			>  DSPL2RAM		 			      // 初始化表

    .init_array		>  DSPL2RAM
    .fardata		>  DSPL2RAM

	GROUP(NEARDP_DATA)
	{
	   .neardata
	   .rodata
	   .bss
	}               >  DSPL2RAM

    .far			>  DSPL2RAM
}
