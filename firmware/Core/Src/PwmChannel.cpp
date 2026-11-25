#include "PwmChannel.h"

namespace {
constexpr uint32_t frecuenciaMinBuzzer = 200;
constexpr uint32_t frecuenciaMaxBuzzer = 4000;
constexpr uint32_t frecuenciaServo = 50;
constexpr uint32_t maxCuentasPeriodo = 65535;
constexpr float pulsoMinServo = 1000.0f;
constexpr float pulsoMaxServo = 2000.0f;

// --- Reloj real de TIM3 y TIM4 ---
uint32_t RelojTimersApb1()
{
    RCC_ClkInitTypeDef reloj = {};
    uint32_t latenciaFlash = 0;
    HAL_RCC_GetClockConfig(&reloj, &latenciaFlash);
    uint32_t relojApb1 = HAL_RCC_GetPCLK1Freq();

    if ((RCC->DCKCFGR & RCC_DCKCFGR_TIMPRE) != 0) {
        if (reloj.APB1CLKDivider == RCC_HCLK_DIV1 || reloj.APB1CLKDivider == RCC_HCLK_DIV2) {
            return HAL_RCC_GetHCLKFreq();
        }
        return relojApb1 * 4;
    }
    if (reloj.APB1CLKDivider == RCC_HCLK_DIV1) {
        return relojApb1;
    }
    return relojApb1 * 2;
}
}

PwmChannel::PwmChannel(TIM_HandleTypeDef *timer, uint32_t canalTimer, Tipo tipoSalida)
    : temporizador(timer), canal(canalTimer), tipo(tipoSalida)
{
}

HAL_StatusTypeDef PwmChannel::iniciar()
{
    // Carga PSC, ARR y CCR antes de que empiece a contar.
    temporizador->Instance->EGR = TIM_EGR_UG;
    __HAL_TIM_CLEAR_FLAG(temporizador, TIM_FLAG_UPDATE);
    return HAL_TIM_PWM_Start(temporizador, canal);
}

HAL_StatusTypeDef PwmChannel::detener()
{
    return HAL_TIM_PWM_Stop(temporizador, canal);
}

void PwmChannel::configurarFrecuencia(uint32_t frecuenciaHz)
{
    // Usamos el divisor mas pequeno que deja el periodo dentro de 16 bits.
    uint32_t relojTimer = RelojTimersApb1();
    uint32_t limiteReloj = frecuenciaHz * maxCuentasPeriodo;
    uint32_t divisor = (relojTimer + limiteReloj - 1) / limiteReloj;
    uint32_t factorPeriodo = frecuenciaHz * divisor;
    uint32_t cuentasPeriodo = (relojTimer + factorPeriodo / 2) / factorPeriodo;

    // Dejamos espacio en CCR para escribir ARR + 1 al pedir duty de 100 %.
    uint32_t prescaler = divisor - 1;
    uint32_t periodo = cuentasPeriodo - 1;
    __HAL_TIM_SET_PRESCALER(temporizador, prescaler);
    temporizador->Init.Prescaler = prescaler;
    __HAL_TIM_SET_AUTORELOAD(temporizador, periodo);
}

void PwmChannel::configurarDuty(float porcentaje)
{
    uint32_t cuentasPeriodo = __HAL_TIM_GET_AUTORELOAD(temporizador) + 1;
    float cuentasEnAlto = cuentasPeriodo * porcentaje / 100.0f;
    uint32_t comparacion = static_cast<uint32_t>(cuentasEnAlto + 0.5f);
    __HAL_TIM_SET_COMPARE(temporizador, canal, comparacion);
}

PwmChannel::Resultado PwmChannel::configurar(uint32_t frecuenciaHz, float porcentaje)
{
    if (!(porcentaje >= 0.0f && porcentaje <= 100.0f)) {
        return Resultado::FueraDeRango;
    }

    if (tipo == Tipo::Servo) {
        // TIM4 sigue a 50 Hz. Un pulso de 1 a 2 ms ocupa del 5 al 10 %.
        bool pulsoValido = porcentaje >= 5.0f && porcentaje <= 10.0f;
        if (frecuenciaHz != frecuenciaServo || (!pulsoValido && porcentaje != 0.0f)) {
            return Resultado::FueraDeRango;
        }
        configurarDuty(porcentaje);
        return Resultado::Ok;
    }

    if (frecuenciaHz < frecuenciaMinBuzzer || frecuenciaHz > frecuenciaMaxBuzzer) {
        return Resultado::FueraDeRango;
    }

    // El buzzer retoma la salida con la frecuencia y el duty ya cargados.
    if (detener() != HAL_OK) {
        return Resultado::Error;
    }
    configurarFrecuencia(frecuenciaHz);
    configurarDuty(porcentaje);
    if (iniciar() != HAL_OK) {
        return Resultado::Error;
    }
    return Resultado::Ok;
}

bool PwmChannel::moverAngulo(float grados)
{
    if (tipo != Tipo::Servo || !(grados >= 0.0f && grados <= 180.0f)) {
        return false;
    }

    // TIM4 cuenta en pasos de 1 us, asi que el pulso va directo a CCR.
    float recorrido = pulsoMaxServo - pulsoMinServo;
    float pulsoUs = pulsoMinServo + grados * recorrido / 180.0f;
    uint32_t comparacion = static_cast<uint32_t>(pulsoUs + 0.5f);
    __HAL_TIM_SET_COMPARE(temporizador, canal, comparacion);
    return true;
}

void PwmChannel::fijarModoAutomatico(bool activar)
{
    automatico = activar;
}

bool PwmChannel::esAutomatico() const
{
    return automatico;
}
