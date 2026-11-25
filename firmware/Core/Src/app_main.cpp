#include "app_main.h"
#include "CommandParser.h"
#include "usbd_cdc_if.h"

#include <string.h>

extern "C" I2C_HandleTypeDef hi2c1;

namespace {
constexpr uint16_t capacidadRx = 256;
uint8_t bufferRx[capacidadRx];
volatile uint16_t indiceLectura = 0;
volatile uint16_t indiceEscritura = 0;
volatile bool reinicioPendiente = false;

PanelI2c panel(&hi2c1);
DigitalOutput ledEstado(STATUS_LED_GPIO_Port, STATUS_LED_Pin, true);
DigitalOutput salidaAlarma(ALARM_OUT_GPIO_Port, ALARM_OUT_Pin);
HoldControl hold;
CommandParser parser(panel, ledEstado, salidaAlarma, hold);
char *respuestaPendiente = nullptr;
}

void App_Iniciar(void)
{
    panel.Inicializar();
}

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
    // La interrupcion solo avisa; el bucle principal revisa el rebote.
    if (pin == HOLD_BUTTON_Pin) {
        hold.RegistrarFlanco();
    }
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
    } else if (!parser.EscaneoActivo() && indiceLectura != indiceEscritura) {
        dato = bufferRx[indiceLectura];
        indiceLectura = (indiceLectura + 1) % capacidadRx;
        datoDisponible = true;
    }

    __set_PRIMASK(mascara);

    hold.Actualizar();

    if (datoDisponible) {
        respuestaPendiente = parser.ProcesarByte(dato);
    } else if (parser.EscaneoActivo()) {
        respuestaPendiente = parser.ProcesarEscaneo();
    } else {
        panel.ActualizarBotones();
    }
}
