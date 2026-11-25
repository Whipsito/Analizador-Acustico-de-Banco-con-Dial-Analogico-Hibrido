#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include "PanelI2c.h"

class CommandParser {
private:
    enum EstadoLinea { Valida, DemasiadoLarga, CaracterInvalido };

    PanelI2c &panel;
    char linea[64] = {};
    uint16_t longitud = 0;
    EstadoLinea estado = Valida;
    char respuestaI2c[2 + (0x77 - 0x08 + 1) * 5 + 3] = {};
    uint16_t longitudEscaneo = 0;
    uint8_t direccionEscaneo = 0;
    uint32_t ultimoPasoEscaneo = 0;

    char *FinalizarLinea();
    char *ProcesarPcf();

public:
    explicit CommandParser(PanelI2c &panelI2c);
    char *ProcesarByte(uint8_t dato);
    char *ProcesarEscaneo();
    bool EscaneoActivo() const;
    void Reiniciar();
};

#endif
