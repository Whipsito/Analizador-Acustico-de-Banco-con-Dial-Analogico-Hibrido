#ifndef PWM_CHANNEL_H
#define PWM_CHANNEL_H

#include "main.h"

class PwmChannel {
public:
    enum class Tipo { Buzzer, Servo };
    enum class Resultado { Ok, FueraDeRango, Error };

    PwmChannel(TIM_HandleTypeDef *timer, uint32_t canalTimer, Tipo tipoSalida);
    HAL_StatusTypeDef iniciar();
    Resultado configurar(uint32_t frecuenciaHz, float porcentaje);
    bool moverAngulo(float grados);
    void fijarModoAutomatico(bool activar);
    bool esAutomatico() const;

private:
    TIM_HandleTypeDef *temporizador;
    uint32_t canal;
    Tipo tipo;
    bool automatico = false;

    HAL_StatusTypeDef detener();
    void configurarFrecuencia(uint32_t frecuenciaHz);
    void configurarDuty(float porcentaje);
};

#endif
