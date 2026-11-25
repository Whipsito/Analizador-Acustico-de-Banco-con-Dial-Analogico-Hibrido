#include "PanelI2c.h"

namespace {
constexpr uint8_t direccionLeds = 0x20;
constexpr uint8_t direccionBotones = 0x21;
constexpr uint32_t intervaloBotones = 10;
constexpr uint8_t pinesEnAlto = 0xFF;
}

PanelI2c::PanelI2c(I2C_HandleTypeDef *i2c)
    : hi2c(i2c), leds(i2c, direccionLeds), botones(i2c, direccionBotones)
{
}

void PanelI2c::Inicializar()
{
    // Las salidas de los LED son activas en bajo.
    leds.EscribirRegistro(pinesEnAlto);
    ultimaLectura = HAL_GetTick() - intervaloBotones;
}

void PanelI2c::ActualizarBotones(bool inmediata)
{
    uint32_t ahora = HAL_GetTick();
    uint32_t tiempoTranscurrido = ahora - ultimaLectura;
    if (!inmediata) {
        if (tiempoTranscurrido == 0) {
            return;
        }
        if (!lecturaPendiente && tiempoTranscurrido < intervaloBotones) {
            return;
        }
    }

    ultimaLectura = ahora;
    lecturaPendiente = false;
    if (!botonesConfigurados) {
        // Escribir 1 libera los pines para que S1-S4 puedan llevarlos a bajo.
        estadoLectura = botones.EscribirRegistro(pinesEnAlto);
        if (estadoLectura != HAL_OK) {
            return;
        }
        botonesConfigurados = true;
        lecturaPendiente = true;
        estadoLectura = HAL_BUSY;
        return;
    }

    estadoLectura = botones.LeerRegistro(lecturaBotones);
    if (estadoLectura == HAL_BUSY) {
        lecturaPendiente = true;
    }
}

HAL_StatusTypeDef PanelI2c::LeerRegistro(uint8_t direccion, uint8_t &valor)
{
    if (direccion == direccionLeds) {
        return leds.LeerRegistro(valor);
    }
    if (direccion == direccionBotones) {
        ActualizarBotones(lecturaPendiente);
        if (estadoLectura == HAL_OK) {
            valor = lecturaBotones;
        }
        return estadoLectura;
    }
    return HAL_ERROR;
}

HAL_StatusTypeDef PanelI2c::EscribirRegistro(uint8_t direccion, uint8_t valor)
{
    if (direccion == direccionLeds) {
        return leds.EscribirRegistro(valor);
    }
    if (direccion == direccionBotones) {
        HAL_StatusTypeDef estado = botones.EscribirRegistro(valor);
        if (estado == HAL_OK) {
            botonesConfigurados = true;
            lecturaPendiente = true;
            estadoLectura = HAL_BUSY;
            ultimaLectura = HAL_GetTick();
        }
        return estado;
    }
    return HAL_ERROR;
}

HAL_StatusTypeDef PanelI2c::ProbarDireccion(uint8_t direccion)
{
    PCF8574 dispositivo(hi2c, direccion);
    return dispositivo.VerificarDireccion();
}
