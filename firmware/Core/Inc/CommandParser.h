#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include "DigitalOutput.h"
#include "HoldControl.h"
#include "PanelI2c.h"
#include "PwmChannel.h"

class CommandParser {
private:
    enum EstadoLinea { Valida, DemasiadoLarga, CaracterInvalido };

    PanelI2c &panel;
    DigitalOutput &salida1;
    DigitalOutput &salida2;
    HoldControl &hold;
    PwmChannel &buzzer;
    PwmChannel &dial;
    char linea[64] = {};
    uint16_t longitud = 0;
    EstadoLinea estado = Valida;
    char respuestaI2c[2 + (0x77 - 0x08 + 1) * 5 + 3] = {};
    uint16_t longitudEscaneo = 0;
    uint8_t direccionEscaneo = 0;
    uint32_t ultimoPasoEscaneo = 0;

    char *FinalizarLinea();
    char *ProcesarPcf();
    char *ProcesarSalida();
    char *ProcesarHold(const char *accion);
    char *ProcesarPwm();
    char *ProcesarDial(const char *accion);

public:
    CommandParser(PanelI2c &panelI2c, DigitalOutput &ledEstado,
                  DigitalOutput &salidaAlarma, HoldControl &controlHold,
                  PwmChannel &pwmBuzzer, PwmChannel &pwmDial);
    char *ProcesarByte(uint8_t dato);
    char *ProcesarEscaneo();
    bool EscaneoActivo() const;
    void Reiniciar();
};

#endif
