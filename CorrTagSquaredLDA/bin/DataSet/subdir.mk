################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../DataSet/CDictionaryReader.cpp \
../DataSet/CParameterMatrixReaderWriter.cpp \
../DataSet/CTagLDAParameterMatrixReaderWriter.cpp \
../DataSet/CTrainValidateSplitter.cpp 

OBJS += \
./DataSet/CDictionaryReader.o \
./DataSet/CParameterMatrixReaderWriter.o \
./DataSet/CTagLDAParameterMatrixReaderWriter.o \
./DataSet/CTrainValidateSplitter.o 

CPP_DEPS += \
./DataSet/CDictionaryReader.d \
./DataSet/CParameterMatrixReaderWriter.d \
./DataSet/CTagLDAParameterMatrixReaderWriter.d \
./DataSet/CTrainValidateSplitter.d 


# Each subdirectory must supply rules for building sources it contributes
DataSet/%.o: ../DataSet/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


