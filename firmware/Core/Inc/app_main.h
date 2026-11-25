#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void App_Iniciar(void);
void App_Procesar(void);
void App_UsbRecibir(const uint8_t *datos, uint32_t longitud);
void App_UsbReiniciar(void);

#ifdef __cplusplus
}
#endif

#endif
