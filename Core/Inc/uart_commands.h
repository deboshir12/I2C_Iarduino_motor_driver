#ifndef UART_COMMANDS_H
#define UART_COMMANDS_H

#include "stm32g4xx_hal.h"
#include <stdint.h>

#ifndef UART_COMMANDS_LINE_SIZE
#define UART_COMMANDS_LINE_SIZE 32U
#endif

typedef enum
{
  UART_COMMAND_TYPE_NONE = 0,
  UART_COMMAND_TYPE_PWM,
  UART_COMMAND_TYPE_RPM,
  UART_COMMAND_TYPE_INVALID
} UartCommandType;

typedef struct
{
  UartCommandType type;
  int16_t value;
} UartCommand;

HAL_StatusTypeDef UartCommands_Start(UART_HandleTypeDef *huart);
void UartCommands_RxCpltCallback(UART_HandleTypeDef *huart);
void UartCommands_ErrorCallback(UART_HandleTypeDef *huart);
uint8_t UartCommands_Pop(UartCommand *command);

#endif /* UART_COMMANDS_H */
