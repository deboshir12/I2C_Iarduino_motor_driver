#include "motor_i2c.h"

static void Motor_Int16ToLeBytes(int16_t value, uint8_t data[2])
{
  uint16_t raw = (uint16_t)value;

  data[0] = (uint8_t)(raw & 0xFFU);
  data[1] = (uint8_t)((raw >> 8) & 0xFFU);
}

static void Motor_U24ToLeBytes(uint32_t value, uint8_t data[3])
{
  data[0] = (uint8_t)(value & 0xFFU);
  data[1] = (uint8_t)((value >> 8) & 0xFFU);
  data[2] = (uint8_t)((value >> 16) & 0xFFU);
}

HAL_StatusTypeDef Motor_SetPwm(I2C_HandleTypeDef *hi2c, int16_t pwm_val)
{
  uint8_t data[2];

  Motor_Int16ToLeBytes(pwm_val, data);

  return HAL_I2C_Mem_Write(hi2c,
                           (MOTOR_I2C_ADDRESS << 1),
                           MOTOR_SET_PWM_REG,
                           I2C_MEMADD_SIZE_8BIT,
                           data,
                           sizeof(data),
                           MOTOR_I2C_TIMEOUT);
}

HAL_StatusTypeDef Motor_SetRpm(I2C_HandleTypeDef *hi2c, int16_t rpm_val)
{
  uint8_t data[2];

  Motor_Int16ToLeBytes(rpm_val, data);

  return HAL_I2C_Mem_Write(hi2c,
                           (MOTOR_I2C_ADDRESS << 1),
                           MOTOR_SET_RPM_REG,
                           I2C_MEMADD_SIZE_8BIT,
                           data,
                           sizeof(data),
                           MOTOR_I2C_TIMEOUT);
}

HAL_StatusTypeDef Motor_Config(I2C_HandleTypeDef *hi2c)
{
  uint8_t num_of_magnets = MOTOR_DEFAULT_MAGNETS;
  uint32_t reducer_ratio = MOTOR_DEFAULT_REDUCER_RATIO;
  uint8_t reducer_data[3];
  HAL_StatusTypeDef status;

  Motor_U24ToLeBytes(reducer_ratio, reducer_data);

  status = HAL_I2C_Mem_Write(hi2c,
                             (MOTOR_I2C_ADDRESS << 1),
                             MOTOR_NUM_MAGNETS_REG,
                             I2C_MEMADD_SIZE_8BIT,
                             &num_of_magnets,
                             1U,
                             MOTOR_I2C_TIMEOUT);

  if (status != HAL_OK)
  {
    return status;
  }

  return HAL_I2C_Mem_Write(hi2c,
                           (MOTOR_I2C_ADDRESS << 1),
                           MOTOR_REDUCER_REG,
                           I2C_MEMADD_SIZE_8BIT,
                           reducer_data,
                           sizeof(reducer_data),
                           MOTOR_I2C_TIMEOUT);
}
