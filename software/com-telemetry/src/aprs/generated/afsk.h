#pragma once

#include <stdint.h>
#include <stdbool.h>

//
// To figure out what those values mean see afsk-utils Python project,
// code_generation_v2.py file
//

#define APRS_SIGNAL_GENERATION_FREQUENCY 96000

#define QUANT_MIN_VALUE 0
#define QUANT_STEP_SIZE 10000

#define SCALED_PI_OVER_TWO    3072
#define SCALED_THREE_HALFS_PI 2304

#define HALF_PERIOD_F1200 400000
#define HALF_PERIOD_F2200 218182

#define PRECISION_QUANT_DIVISOR       1000
#define PRECISION_QUANT_ROUND_SUMMAND 500

#define TRIG_PARAM_SCALER_F1200            3840
#define TRIG_PARAM_SCALER_F2200            7040
#define PRECISION_TRIG_PARAM_DIVISOR       1000000
#define PRECISION_TRIG_PARAM_ROUND_SUMMAND 500000

#define INVERSE_TRIG_PARAM_SCALER                  75
#define PRECISION_INVERSE_TRIG_PARAM_DIVISOR       10000
#define PRECISION_INVERSE_TRIG_PARAM_ROUND_SUMMAND 5000

#define QUANTS_COUNT_PER_SYMBOL_F1200 800000
#define QUANTS_COUNT_PER_SYMBOL_F2200 436364

#define RECIPROCAL_ANGULAR_FREQUENCY_F1200 127324
#define RECIPROCAL_ANGULAR_FREQUENCY_F2200 69449

#define PRECISION_OUTPUT_AMPLITUDE_DIVISOR       100
#define PRECISION_OUTPUT_AMPLITUDE_ROUND_SUMMAND 50

#define LEADING_WARMUP_QUANTS_COUNT 960

extern const uint16_t amplitudeFromTable[];
extern const uint32_t scaledSineValueFromTable[];
extern const uint16_t scaledArcSineValueFromTable[];

uint32_t calculateQuantIndexFromOtherFrequencyQuantIdxAndAmplitude(uint32_t otherFrequencyCurrentQuant,
                                                                   uint32_t otherFrequencyTrigParamScaler,
                                                                   uint32_t targetFrequencyReciprocalAngularFrequency,
                                                                   uint32_t targetFrequencyHalfPeriod,
                                                                   uint32_t targetFrequencyQuantsCountPerSymbol);

#define REDUCE_PRECISION(value, precision_rounding, precision_divisor) \
    ((value) + (precision_rounding)) / (precision_divisor)

#define IS_COSINE_GREATER_OR_EQUAL_TO_ZERO(value) \
    ((value) > SCALED_PI_OVER_TWO && (value) < SCALED_THREE_HALFS_PI)

#define CALCULATE_F1200_AMPLITUDE_FROM_QUANT_IDX(currentF1200Quant) \
    amplitudeFromTable[REDUCE_PRECISION(TRIG_PARAM_SCALER_F1200 * (currentF1200Quant), PRECISION_TRIG_PARAM_ROUND_SUMMAND, PRECISION_TRIG_PARAM_DIVISOR)]

#define CALCULATE_F2200_AMPLITUDE_FROM_QUANT_IDX(currentF2200Quant) \
    amplitudeFromTable[REDUCE_PRECISION(TRIG_PARAM_SCALER_F2200 * (currentF2200Quant), PRECISION_TRIG_PARAM_ROUND_SUMMAND, PRECISION_TRIG_PARAM_DIVISOR)]

#define CALCULATE_F1200_QUANT_IDX_FROM_F2200_QUANT_IDX(currentF2200Quant) \
    calculateQuantIndexFromOtherFrequencyQuantIdxAndAmplitude(TRIG_PARAM_SCALER_F2200, currentF2200Quant, RECIPROCAL_ANGULAR_FREQUENCY_F1200, HALF_PERIOD_F1200, QUANTS_COUNT_PER_SYMBOL_F1200)

#define CALCULATE_F2200_QUANT_IDX_FROM_F1200_QUANT_IDX(currentF1200Quant) \
    calculateQuantIndexFromOtherFrequencyQuantIdxAndAmplitude(TRIG_PARAM_SCALER_F1200, currentF1200Quant, RECIPROCAL_ANGULAR_FREQUENCY_F2200, HALF_PERIOD_F2200, QUANTS_COUNT_PER_SYMBOL_F2200)

