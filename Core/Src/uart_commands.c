#include "uart_commands.h"

#include <stddef.h>
#include <stdlib.h>

static UART_HandleTypeDef *uart_commands_huart;
static uint8_t uart_commands_rx_byte;

static char uart_commands_rx_line[UART_COMMANDS_LINE_SIZE + 1U];
static uint16_t uart_commands_rx_len;
static volatile uint8_t uart_commands_drop_line;

static char uart_commands_pending_line[UART_COMMANDS_LINE_SIZE + 1U];
static volatile uint8_t uart_commands_pending_ready;
static volatile uint8_t uart_commands_pending_invalid;

static char UartCommands_ToLower(char ch)
{
  if ((ch >= 'A') && (ch <= 'Z'))
  {
    return (char)(ch + ('a' - 'A'));
  }

  return ch;
}

static uint8_t UartCommands_IsSeparator(char ch)
{
  return ((ch == ' ') || (ch == '\t') || (ch == '='));
}

static const char *UartCommands_SkipSeparators(const char *text)
{
  while (UartCommands_IsSeparator(*text) != 0U)
  {
    text++;
  }

  return text;
}

static uint8_t UartCommands_MatchKeyword(const char *line,
                                         const char *keyword,
                                         const char **value_text)
{
  uint32_t index = 0U;

  while (keyword[index] != '\0')
  {
    if (UartCommands_ToLower(line[index]) != keyword[index])
    {
      return 0U;
    }

    index++;
  }

  if (UartCommands_IsSeparator(line[index]) == 0U)
  {
    return 0U;
  }

  *value_text = &line[index];
  return 1U;
}

static uint8_t UartCommands_ParseInt16(const char *text, int16_t *value)
{
  char *endptr;
  long parsed;

  text = UartCommands_SkipSeparators(text);
  if (*text == '\0')
  {
    return 0U;
  }

  parsed = strtol(text, &endptr, 10);
  if (endptr == text)
  {
    return 0U;
  }

  while ((*endptr == ' ') || (*endptr == '\t'))
  {
    endptr++;
  }

  if (*endptr != '\0')
  {
    return 0U;
  }

  if ((parsed < -32768L) || (parsed > 32767L))
  {
    return 0U;
  }

  *value = (int16_t)parsed;
  return 1U;
}

static uint8_t UartCommands_ParseLine(const char *line, UartCommand *command)
{
  const char *value_text;

  command->type = UART_COMMAND_TYPE_INVALID;
  command->value = 0;

  if (UartCommands_MatchKeyword(line, "pwm", &value_text) != 0U)
  {
    command->type = UART_COMMAND_TYPE_PWM;
  }
  else if (UartCommands_MatchKeyword(line, "rpm", &value_text) != 0U)
  {
    command->type = UART_COMMAND_TYPE_RPM;
  }
  else
  {
    return 1U;
  }

  if (UartCommands_ParseInt16(value_text, &command->value) == 0U)
  {
    command->type = UART_COMMAND_TYPE_INVALID;
    command->value = 0;
  }

  return 1U;
}

static void UartCommands_ResetRxLine(void)
{
  uart_commands_rx_len = 0U;
  uart_commands_drop_line = 0U;
}

static void UartCommands_StorePendingLine(uint8_t invalid)
{
  uint16_t index;

  if (uart_commands_pending_ready != 0U)
  {
    return;
  }

  for (index = 0U; index < uart_commands_rx_len; index++)
  {
    uart_commands_pending_line[index] = uart_commands_rx_line[index];
  }

  uart_commands_pending_line[index] = '\0';
  uart_commands_pending_invalid = invalid;
  uart_commands_pending_ready = 1U;
}

static void UartCommands_ProcessByte(uint8_t byte)
{
  if ((byte == '\r') || (byte == '\n'))
  {
    if (uart_commands_drop_line != 0U)
    {
      uart_commands_rx_len = 0U;
      UartCommands_StorePendingLine(1U);
    }
    else if (uart_commands_rx_len > 0U)
    {
      UartCommands_StorePendingLine(0U);
    }

    UartCommands_ResetRxLine();
    return;
  }

  if (uart_commands_drop_line != 0U)
  {
    return;
  }

  if (uart_commands_rx_len >= UART_COMMANDS_LINE_SIZE)
  {
    uart_commands_rx_len = 0U;
    uart_commands_drop_line = 1U;
    return;
  }

  uart_commands_rx_line[uart_commands_rx_len] = (char)byte;
  uart_commands_rx_len++;
}

HAL_StatusTypeDef UartCommands_Start(UART_HandleTypeDef *huart)
{
  if (huart == NULL)
  {
    return HAL_ERROR;
  }

  uart_commands_huart = huart;
  UartCommands_ResetRxLine();
  uart_commands_pending_ready = 0U;
  uart_commands_pending_invalid = 0U;

  return HAL_UART_Receive_IT(uart_commands_huart, &uart_commands_rx_byte, 1U);
}

void UartCommands_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if ((uart_commands_huart == NULL) || (huart != uart_commands_huart))
  {
    return;
  }

  UartCommands_ProcessByte(uart_commands_rx_byte);
  (void)HAL_UART_Receive_IT(uart_commands_huart, &uart_commands_rx_byte, 1U);
}

void UartCommands_ErrorCallback(UART_HandleTypeDef *huart)
{
  if ((uart_commands_huart == NULL) || (huart != uart_commands_huart))
  {
    return;
  }

  UartCommands_ResetRxLine();
  (void)HAL_UART_Receive_IT(uart_commands_huart, &uart_commands_rx_byte, 1U);
}

uint8_t UartCommands_Pop(UartCommand *command)
{
  char line[UART_COMMANDS_LINE_SIZE + 1U];
  uint8_t invalid;
  uint32_t primask;
  uint16_t index;

  if (command == NULL)
  {
    return 0U;
  }

  primask = __get_PRIMASK();
  __disable_irq();

  if (uart_commands_pending_ready == 0U)
  {
    __set_PRIMASK(primask);
    return 0U;
  }

  for (index = 0U; index <= UART_COMMANDS_LINE_SIZE; index++)
  {
    line[index] = uart_commands_pending_line[index];
    if (line[index] == '\0')
    {
      break;
    }
  }

  invalid = uart_commands_pending_invalid;
  uart_commands_pending_ready = 0U;
  uart_commands_pending_invalid = 0U;

  __set_PRIMASK(primask);

  if (invalid != 0U)
  {
    command->type = UART_COMMAND_TYPE_INVALID;
    command->value = 0;
    return 1U;
  }

  return UartCommands_ParseLine(line, command);
}
