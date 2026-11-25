#include "CommandParser.h"

#include <string.h>

namespace {
// Las respuestas conservan su memoria hasta que termina la transferencia USB.
alignas(4) char respuestaPing[32] = "OK PONG\r\n";
alignas(4) char respuestaStatus[32] = "OK STATUS READY\r\n";
alignas(4) char errorComando[32] = "ERR UNKNOWN_CMD\r\n";
alignas(4) char errorLongitud[32] = "ERR LINE_TOO_LONG\r\n";
alignas(4) char errorCaracter[32] = "ERR INVALID_CHAR\r\n";
}

void CommandParser::Reiniciar()
{
    longitud = 0;
    estado = Valida;
}

char *CommandParser::ProcesarByte(uint8_t dato)
{
    if (dato == '\n') {
        char *respuesta = FinalizarLinea();
        Reiniciar();
        return respuesta;
    }

    if (estado != Valida) {
        return nullptr;
    }

    if (dato != '\r' && (dato < ' ' || dato > '~')) {
        estado = CaracterInvalido;
    } else if (longitud == sizeof(linea) - 1) {
        estado = DemasiadoLarga;
    } else {
        linea[longitud] = static_cast<char>(dato);
        longitud++;
    }

    return nullptr;
}

char *CommandParser::FinalizarLinea()
{
    if (estado == DemasiadoLarga) {
        return errorLongitud;
    }
    if (estado == CaracterInvalido) {
        return errorCaracter;
    }

    if (longitud > 0 && linea[longitud - 1] == '\r') {
        longitud--;
    }
    if (longitud == 0) {
        return nullptr;
    }

    linea[longitud] = '\0';
    if (strcmp(linea, "PING") == 0) {
        return respuestaPing;
    }
    if (strcmp(linea, "STATUS") == 0) {
        return respuestaStatus;
    }

    return errorComando;
}
