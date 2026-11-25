#include "DigitalOutput.h"

DigitalOutput::DigitalOutput(GPIO_TypeDef *puertoGpio, uint16_t pinGpio, bool logicaInvertida)
    : puerto(puertoGpio), pin(pinGpio), activoBajo(logicaInvertida)
{
}

// --- Traduccion entre encendido y el nivel del pin ---
void DigitalOutput::encender()
{
    if (activoBajo) {
        HAL_GPIO_WritePin(puerto, pin, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(puerto, pin, GPIO_PIN_SET);
    }
}

void DigitalOutput::apagar()
{
    if (activoBajo) {
        HAL_GPIO_WritePin(puerto, pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(puerto, pin, GPIO_PIN_RESET);
    }
}

void DigitalOutput::alternar()
{
    HAL_GPIO_TogglePin(puerto, pin);
}

bool DigitalOutput::obtenerEstado() const
{
    GPIO_PinState nivel = HAL_GPIO_ReadPin(puerto, pin);
    if (activoBajo) {
        return nivel == GPIO_PIN_RESET;
    }
    return nivel == GPIO_PIN_SET;
}
