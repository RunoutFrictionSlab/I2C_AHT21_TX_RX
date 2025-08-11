################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"/home/jtwillobee/ti/ccs2020/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/bin/cl430" -vmspx --data_model=restricted -Ooff --use_hw_mpy=F5 --include_path="/home/jtwillobee/ti/ccs2020/ccs/ccs_base/msp430/include" --include_path="/home/jtwillobee/repos/I2C_AHT21_TX__RX" --include_path="/home/jtwillobee/repos/I2C_AHT21_TX__RX/driverlib/MSP430F5xx_6xx" --include_path="/home/jtwillobee/ti/ccs2020/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/include" --advice:power="none" --define=__MSP430F5529__ --define=DEPRECATED -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '


