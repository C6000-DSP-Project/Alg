################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
PRULoader.obj: ../PRULoader.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ti-cgt-c6000_8.3.12/bin/cl6x" -mv6740 --abi=eabi --include_path="C:/ti/ti-cgt-c6000_8.3.12/include" --include_path="../../../Include/StarterWare/Drivers" --include_path="../../../Include/StarterWare/Drivers/hw" --include_path="../../../Include/Loader" --define=c6748 -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="PRULoader.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


