#ifndef INC_PCF8574_H_
#define INC_PCF8574_H_

#include "main.h"

class PCF8574 {
private:
    I2C_HandleTypeDef *hi2c;
    uint8_t direccion;
    uint8_t salida;

public:
    PCF8574(I2C_HandleTypeDef *i2c, uint8_t direccion7bits = 0x20);
    HAL_StatusTypeDef LeerRegistro(uint8_t &valor);
    HAL_StatusTypeDef EscribirRegistro(uint8_t valor);
    HAL_StatusTypeDef LeerPin(uint8_t pin, bool &valor);
    HAL_StatusTypeDef EscribirPin(uint8_t pin, bool valor);
    HAL_StatusTypeDef VerificarDireccion();
};

#endif
