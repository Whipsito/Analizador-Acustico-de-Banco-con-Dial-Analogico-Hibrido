#include "HoldControl.h"
#include "main.h"

namespace {
constexpr uint32_t antirreboteMs = 30;
}

void HoldControl::RegistrarFlanco()
{
    flancoPendiente = true;
}

// --- Antirrebote de la pulsacion y de la liberacion ---
void HoldControl::Actualizar()
{
    bool nuevoFlanco = flancoPendiente;
    if (nuevoFlanco) {
        flancoPendiente = false;
    }
    if (!nuevoFlanco && !observandoBoton) {
        return;
    }

    bool pulsado = HAL_GPIO_ReadPin(HOLD_BUTTON_GPIO_Port, HOLD_BUTTON_Pin) == GPIO_PIN_RESET;
    uint32_t ahora = HAL_GetTick();
    if (nuevoFlanco || pulsado != ultimaLecturaPulsado) {
        observandoBoton = true;
        ultimaLecturaPulsado = pulsado;
        inicioLectura = ahora;
        return;
    }
    if (ahora - inicioLectura < antirreboteMs) {
        return;
    }

    // Se necesita soltar el boton antes de aceptar otra pulsacion.
    if (!pulsado) {
        pulsacionConfirmada = false;
        observandoBoton = false;
    } else if (!pulsacionConfirmada) {
        pulsacionConfirmada = true;
        alternar();
    }
}

void HoldControl::activar()
{
    activo = true;
}

void HoldControl::desactivar()
{
    activo = false;
}

void HoldControl::alternar()
{
    activo = !activo;
}

bool HoldControl::obtenerEstado() const
{
    return activo;
}
