#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <stdint.h>

class CommandParser {
private:
    enum EstadoLinea { Valida, DemasiadoLarga, CaracterInvalido };

    char linea[64] = {};
    uint16_t longitud = 0;
    EstadoLinea estado = Valida;

    char *FinalizarLinea();

public:
    char *ProcesarByte(uint8_t dato);
    void Reiniciar();
};

#endif
