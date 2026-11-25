#ifndef DIGITAL_OUTPUT_H
#define DIGITAL_OUTPUT_H

#include "main.h"

class DigitalOutput {
private:
    GPIO_TypeDef *puerto;
    uint16_t pin;
    bool activoBajo;

public:
    DigitalOutput(GPIO_TypeDef *puertoGpio, uint16_t pinGpio, bool logicaInvertida = false);
    void encender();
    void apagar();
    void alternar();
    bool obtenerEstado() const;
};

#endif
