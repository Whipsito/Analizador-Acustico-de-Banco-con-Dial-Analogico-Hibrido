#ifndef PANEL_I2C_H
#define PANEL_I2C_H

#include "PCF8574.h"

class PanelI2c {
private:
    I2C_HandleTypeDef *hi2c;
    PCF8574 leds;
    PCF8574 botones;
    uint32_t ultimaLectura = 0;
    uint8_t lecturaBotones = 0xFF;
    HAL_StatusTypeDef estadoLectura = HAL_ERROR;
    bool botonesConfigurados = false;
    bool lecturaPendiente = true;

public:
    explicit PanelI2c(I2C_HandleTypeDef *i2c);
    void Inicializar();
    void ActualizarBotones(bool inmediata = false);
    HAL_StatusTypeDef LeerRegistro(uint8_t direccion, uint8_t &valor);
    HAL_StatusTypeDef EscribirRegistro(uint8_t direccion, uint8_t valor);
    HAL_StatusTypeDef ProbarDireccion(uint8_t direccion);
};

#endif
