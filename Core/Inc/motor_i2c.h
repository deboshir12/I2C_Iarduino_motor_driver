#ifndef MOTOR_I2C_H
#define MOTOR_I2C_H

#include "stm32g4xx_hal.h"
#include <stdint.h>

#define MOTOR_I2C_ADDRESS      0x09U
#define MOTOR_I2C_TIMEOUT      10U

#define MOTOR_MODEL_REG        0x04U
#define MOTOR_VERSION_REG      0x05U
#define MOTOR_ADDRESS_REG      0x06U
#define MOTOR_PWM_FREQ_REG     0x08U
#define MOTOR_NUM_MAGNETS_REG  0x11U
#define MOTOR_REDUCER_REG      0x12U
#define MOTOR_SET_PWM_REG      0x15U
#define MOTOR_SET_RPM_REG      0x17U
#define MOTOR_GET_RPM_REG      0x19U

#define MOTOR_DEFAULT_MAGNETS       7U
#define MOTOR_DEFAULT_REDUCER_RATIO 1450U

HAL_StatusTypeDef Motor_SetPwm(I2C_HandleTypeDef *hi2c, int16_t pwm_val);
HAL_StatusTypeDef Motor_SetRpm(I2C_HandleTypeDef *hi2c, int16_t rpm_val);
HAL_StatusTypeDef Motor_Config(I2C_HandleTypeDef *hi2c);

#endif /* MOTOR_I2C_H */
