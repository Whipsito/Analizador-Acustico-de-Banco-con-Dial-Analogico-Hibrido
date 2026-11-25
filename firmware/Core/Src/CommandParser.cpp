#include "CommandParser.h"

#include <string.h>

namespace {
constexpr uint8_t primeraDireccionI2c = 0x08;
constexpr uint8_t ultimaDireccionI2c = 0x77;
char respuestaPing[] = "OK PONG\r\n";
char respuestaStatus[] = "OK STATUS READY\r\n";
char respuestaOk[] = "OK\r\n";
char respuestaSinDispositivos[] = "OK NONE\r\n";
char errorComando[] = "ERR UNKNOWN_CMD\r\n";
char errorLongitud[] = "ERR LINE_TOO_LONG\r\n";
char errorCaracter[] = "ERR INVALID_CHAR\r\n";
char errorArgumentos[] = "ERR BAD_ARGS\r\n";
char errorI2c[] = "ERR I2C\r\n";

// --- Conversion de bytes hexadecimales ---
int8_t LeerDigitoHex(char caracter)
{
    if (caracter >= '0' && caracter <= '9') {
        return caracter - '0';
    }
    if (caracter >= 'A' && caracter <= 'F') {
        return caracter - 'A' + 10;
    }
    if (caracter >= 'a' && caracter <= 'f') {
        return caracter - 'a' + 10;
    }
    return -1;
}

bool LeerHex(const char *&texto, uint8_t &valor)
{
    if (*texto != '0' || (texto[1] != 'x' && texto[1] != 'X')) {
        return false;
    }
    texto += 2;

    int8_t alto = LeerDigitoHex(*texto);
    if (alto < 0) {
        return false;
    }
    texto++;
    int8_t bajo = LeerDigitoHex(*texto);
    if (bajo < 0) {
        return false;
    }
    texto++;
    valor = (alto << 4) | bajo;
    return true;
}

void EscribirHex(char *texto, uint8_t valor)
{
    const char digitos[] = "0123456789ABCDEF";
    texto[0] = '0';
    texto[1] = 'x';
    texto[2] = digitos[valor >> 4];
    texto[3] = digitos[valor & 0x0F];
}
}

CommandParser::CommandParser(PanelI2c &panelI2c) : panel(panelI2c)
{
}

void CommandParser::Reiniciar()
{
    longitud = 0;
    estado = Valida;
    direccionEscaneo = 0;
}

char *CommandParser::ProcesarByte(uint8_t dato)
{
    if (dato == '\n') {
        char *respuesta = FinalizarLinea();
        longitud = 0;
        estado = Valida;
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
    if (strcmp(linea, "I2C SCAN") == 0) {
        memcpy(respuestaI2c, "OK", 2);
        longitudEscaneo = 2;
        direccionEscaneo = primeraDireccionI2c;
        ultimoPasoEscaneo = HAL_GetTick();
        return nullptr;
    }
    if (strncmp(linea, "PCF ", 4) == 0 || strcmp(linea, "PCF") == 0) {
        return ProcesarPcf();
    }

    return errorComando;
}

char *CommandParser::ProcesarPcf()
{
    const char *argumentos = linea + 7;
    uint8_t direccion = 0;
    uint8_t valor = 0;

    if (strncmp(linea, "PCF RD ", 7) == 0) {
        if (!LeerHex(argumentos, direccion) || *argumentos != '\0') {
            return errorArgumentos;
        }
        if (panel.LeerRegistro(direccion, valor) != HAL_OK) {
            return errorI2c;
        }
        memcpy(respuestaI2c, "OK ", 3);
        EscribirHex(respuestaI2c + 3, valor);
        memcpy(respuestaI2c + 7, "\r\n", 3);
        return respuestaI2c;
    }
    if (strncmp(linea, "PCF WR ", 7) == 0) {
        if (!LeerHex(argumentos, direccion) || *argumentos != ' ') {
            return errorArgumentos;
        }
        argumentos++;
        if (!LeerHex(argumentos, valor) || *argumentos != '\0') {
            return errorArgumentos;
        }
        if (panel.EscribirRegistro(direccion, valor) != HAL_OK) {
            return errorI2c;
        }
        return respuestaOk;
    }
    return errorArgumentos;
}

bool CommandParser::EscaneoActivo() const
{
    return direccionEscaneo != 0;
}

char *CommandParser::ProcesarEscaneo()
{
    // Se prueba una direccion por tick para dejar tiempo al bucle y al USB.
    uint32_t ahora = HAL_GetTick();
    if (ahora == ultimoPasoEscaneo) {
        return nullptr;
    }
    ultimoPasoEscaneo = ahora;

    HAL_StatusTypeDef resultado = panel.ProbarDireccion(direccionEscaneo);
    if (resultado == HAL_BUSY || resultado == HAL_TIMEOUT) {
        direccionEscaneo = 0;
        return errorI2c;
    }
    if (resultado == HAL_OK) {
        respuestaI2c[longitudEscaneo] = ' ';
        longitudEscaneo++;
        EscribirHex(respuestaI2c + longitudEscaneo, direccionEscaneo);
        longitudEscaneo += 4;
    }
    if (direccionEscaneo < ultimaDireccionI2c) {
        direccionEscaneo++;
        return nullptr;
    }

    direccionEscaneo = 0;
    if (longitudEscaneo == 2) {
        return respuestaSinDispositivos;
    }
    memcpy(respuestaI2c + longitudEscaneo, "\r\n", 3);
    return respuestaI2c;
}
