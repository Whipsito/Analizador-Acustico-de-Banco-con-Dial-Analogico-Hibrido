#ifndef HOLD_CONTROL_H
#define HOLD_CONTROL_H

#include <stdint.h>

class HoldControl {
private:
    volatile bool flancoPendiente = false;
    bool observandoBoton = false;
    bool ultimaLecturaPulsado = false;
    bool pulsacionConfirmada = false;
    uint32_t inicioLectura = 0;
    bool activo = false;

public:
    void RegistrarFlanco();
    void Actualizar();
    void activar();
    void desactivar();
    void alternar();
    bool obtenerEstado() const;
};

#endif
