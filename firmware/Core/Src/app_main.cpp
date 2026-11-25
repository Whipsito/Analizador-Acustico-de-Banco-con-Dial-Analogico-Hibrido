#include "app_main.h"
#include "CommandParser.h"
#include "usbd_cdc_if.h"

#include <string.h>

namespace {
constexpr uint16_t capacidadRx = 256;
uint8_t bufferRx[capacidadRx];
volatile uint16_t indiceLectura = 0;
volatile uint16_t indiceEscritura = 0;
volatile bool reinicioPendiente = false;

CommandParser parser;
char *respuestaPendiente = nullptr;
}

void App_UsbRecibir(const uint8_t *datos, uint32_t longitud)
{
    for (uint32_t indice = 0; indice < longitud; indice++) {
        bufferRx[indiceEscritura] = datos[indice];
        indiceEscritura = (indiceEscritura + 1) % capacidadRx;
    }
}

void App_UsbReiniciar(void)
{
    indiceLectura = 0;
    indiceEscritura = 0;
    reinicioPendiente = true;
}

void App_Procesar(void)
{
    bool datoDisponible = false;
    uint8_t dato = 0;

    // Protege los indices y el estado USB frente a una interrupcion de desconexion.
    uint32_t mascara = __get_PRIMASK();
    __disable_irq();

    if (reinicioPendiente) {
        parser.Reiniciar();
        respuestaPendiente = nullptr;
        reinicioPendiente = false;
    }

    // El endpoint OUT solo se rearma si cabe un paquete USB completo.
    uint16_t ocupados = (indiceEscritura + capacidadRx - indiceLectura) % capacidadRx;
    uint16_t espacioLibre = capacidadRx - 1 - ocupados;
    if (espacioLibre >= CDC_DATA_FS_OUT_PACKET_SIZE) {
        CDC_RearmarRecepcion_FS();
    }

    if (respuestaPendiente != nullptr) {
        uint16_t longitud = static_cast<uint16_t>(strlen(respuestaPendiente));
        uint8_t *datos = reinterpret_cast<uint8_t *>(respuestaPendiente);
        if (CDC_Transmit_FS(datos, longitud) == USBD_OK) {
            respuestaPendiente = nullptr;
        }
    } else if (indiceLectura != indiceEscritura) {
        dato = bufferRx[indiceLectura];
        indiceLectura = (indiceLectura + 1) % capacidadRx;
        datoDisponible = true;
    }

    __set_PRIMASK(mascara);

    if (datoDisponible) {
        respuestaPendiente = parser.ProcesarByte(dato);
    }
}
