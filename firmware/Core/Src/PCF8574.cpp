#include "PCF8574.h"

// Adaptado del driver de clase ejemplos_clase/proyectomodbus.
namespace {
constexpr uint32_t timeoutI2c = 2;
}

PCF8574::PCF8574(I2C_HandleTypeDef *i2c, uint8_t direccion7bits)
    : hi2c(i2c), direccion(direccion7bits), salida(0xFF)
{
}

HAL_StatusTypeDef PCF8574::LeerRegistro(uint8_t &valor)
{
    if (hi2c == nullptr) {
        return HAL_ERROR;
    }
    if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BUSY)) {
        return HAL_BUSY;
    }

    // La lectura refleja los pines; salida conserva la ultima escritura.
    uint16_t direccionHal = direccion << 1;
    return HAL_I2C_Master_Receive(hi2c, direccionHal, &valor, 1, timeoutI2c);
}

HAL_StatusTypeDef PCF8574::EscribirRegistro(uint8_t valor)
{
    if (hi2c == nullptr) {
        return HAL_ERROR;
    }
    if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BUSY)) {
        return HAL_BUSY;
    }

    uint16_t direccionHal = direccion << 1;
    HAL_StatusTypeDef estado = HAL_I2C_Master_Transmit(hi2c, direccionHal, &valor, 1, timeoutI2c);
    if (estado == HAL_OK) {
        salida = valor;
    }
    return estado;
}

HAL_StatusTypeDef PCF8574::LeerPin(uint8_t pin, bool &valor)
{
    if (pin > 7) {
        return HAL_ERROR;
    }

    uint8_t puerto = 0;
    HAL_StatusTypeDef estado = LeerRegistro(puerto);
    if (estado == HAL_OK) {
        uint8_t mascara = 1U << pin;
        valor = (puerto & mascara) != 0;
    }
    return estado;
}

HAL_StatusTypeDef PCF8574::EscribirPin(uint8_t pin, bool valor)
{
    if (pin > 7) {
        return HAL_ERROR;
    }

    uint8_t mascara = 1U << pin;
    uint8_t nuevo;
    if (valor) {
        nuevo = salida | mascara;
    } else {
        nuevo = salida & ~mascara;
    }
    return EscribirRegistro(nuevo);
}

HAL_StatusTypeDef PCF8574::VerificarDireccion()
{
    if (hi2c == nullptr) {
        return HAL_ERROR;
    }
    if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BUSY)) {
        return HAL_BUSY;
    }

    uint16_t direccionHal = direccion << 1;
    return HAL_I2C_IsDeviceReady(hi2c, direccionHal, 1, timeoutI2c);
}
