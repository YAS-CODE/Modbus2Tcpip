################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/LodamModbusProject.cpp \
../src/ModbusController.cpp \
../src/ServerController.cpp \
../src/Utility.cpp 

OBJS += \
./src/LodamModbusProject.o \
./src/ModbusController.o \
./src/ServerController.o \
./src/Utility.o 

CPP_DEPS += \
./src/LodamModbusProject.d \
./src/ModbusController.d \
./src/ServerController.d \
./src/Utility.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/modbus -I/usr/include/curlpp -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


