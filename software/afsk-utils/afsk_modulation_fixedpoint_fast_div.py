import numpy

import fixedpoint
import trigtables
import definitions
import definitions_derived
import afsk_modulation_fixedpoint

class AfskModulationFixedPointFastDiv(afsk_modulation_fixedpoint.AfskModulationFixedPoint):
    def __init__(self, precisionData, data, bitsCount):
        afsk_modulation_fixedpoint.AfskModulationFixedPoint.__init__(self, precisionData, data, bitsCount)
        self.divData = {}
        self.CLAMPED_VALUE = False

    def findBestFastDivision(self, divisor, maxValue):
        firstDivisorPowerOfTwo = 0
        while (divisor & 1) == 0 and divisor != 0:
            firstDivisorPowerOfTwo += 1
            divisor = divisor >> 1
        if firstDivisorPowerOfTwo > 0:
            maxValue = maxValue >> firstDivisorPowerOfTwo
        if divisor != 1:
            for i in range(1, 31):
                testLastDivisorPowerOfTwo = i
                testMultiplier = numpy.uint32((1 << testLastDivisorPowerOfTwo) / divisor + 0.5)
                if testMultiplier > 0:
                    maxMultipliedValue = numpy.uint64(testMultiplier) * maxValue
                    if maxMultipliedValue > numpy.iinfo(numpy.uint32).max:
                        break
                    multiplier = testMultiplier
                    lastDivisorPowerOfTwo = testLastDivisorPowerOfTwo
            return (firstDivisorPowerOfTwo, multiplier, lastDivisorPowerOfTwo)
        else:
            return (firstDivisorPowerOfTwo, 1, 0)

    def getBestFastDivision(self, fastDivisionAlias):
        return self.divData.get(fastDivisionAlias)

    def fastDiv(self, value, fastDivData):
        (firstDivisorPowerOfTwo, multiplier, lastDivisorPowerOfTwo, precisionSummand, clampValue) = fastDivData
        r0 = numpy.uint64(value) + precisionSummand
        if r0 > numpy.iinfo(numpy.uint32).max:
            raise RuntimeError(str(r0) + ' number is too large')
        r1 = numpy.uint32(numpy.uint32(r0) >> firstDivisorPowerOfTwo)
        r2 = numpy.uint64(r1) * multiplier
        if r2 > numpy.iinfo(numpy.uint32).max:
            raise RuntimeError(str(r2) + ' number is too large (' + str(r1) + ' * ' + str(multiplier) + ')')
        r3 = numpy.uint32(numpy.uint32(r2) >> lastDivisorPowerOfTwo)
        if r3 > clampValue:
            self.CLAMPED_VALUE = True
            return clampValue
        else:
            return r3

    def calculateTrigArg(self, isF1200, currentQuant):
        if isF1200:
            result = (self.CONST_TRIG_PARAM_SCALER_F1200 * currentQuant)
            fastDivData = self.divData.get('CONST_PRECISION_TRIG_PARAM_DIVISOR_F1200')
            if fastDivData is None:
                (trigArg, self.CONST_PRECISION_TRIG_PARAM_ROUND_SUMMAND, self.CONST_PRECISION_TRIG_PARAM_DIVISOR) = \
                    result.convert2Precision(self.precisionData.PRECISION_TRIG_ARG)
                maxValue = \
                    self.CONST_TRIG_PARAM_SCALER_F1200.getInternalRepresentation() * self.CONST_F1200_QUANTS_COUNT_PER_SYMBOL.getInternalRepresentation()
                precisionSummand = self.CONST_PRECISION_TRIG_PARAM_ROUND_SUMMAND
                (firstDivisorPowerOfTwo, multiplier, lastDivisorPowerOfTwo) = \
                    self.findBestFastDivision(self.CONST_PRECISION_TRIG_PARAM_DIVISOR, maxValue + precisionSummand)
                clampValue = len(self.trigTables.sineValues) - 1
                fastDivData = self.divData['CONST_PRECISION_TRIG_PARAM_DIVISOR_F1200'] = \
                    (firstDivisorPowerOfTwo, multiplier, lastDivisorPowerOfTwo, precisionSummand, clampValue)
        else:
            result = (self.CONST_TRIG_PARAM_SCALER_F2200 * currentQuant)
            fastDivData = self.divData.get('CONST_PRECISION_TRIG_PARAM_DIVISOR_F2200')
            if 'CONST_PRECISION_TRIG_PARAM_DIVISOR_F2200' not in self.divData.keys():
                (trigArg, self.CONST_PRECISION_TRIG_PARAM_ROUND_SUMMAND, self.CONST_PRECISION_TRIG_PARAM_DIVISOR) = \
                    result.convert2Precision(self.precisionData.PRECISION_TRIG_ARG)
                maxValue = \
                    self.CONST_TRIG_PARAM_SCALER_F2200.getInternalRepresentation() * self.CONST_F2200_QUANTS_COUNT_PER_SYMBOL.getInternalRepresentation()
                precisionSummand = self.CONST_PRECISION_TRIG_PARAM_ROUND_SUMMAND
                (firstDivisorPowerOfTwo, multiplier, lastDivisorPowerOfTwo) = \
                    self.findBestFastDivision(self.CONST_PRECISION_TRIG_PARAM_DIVISOR, maxValue + precisionSummand)
                clampValue = len(self.trigTables.sineValues) - 1
                fastDivData = self.divData['CONST_PRECISION_TRIG_PARAM_DIVISOR_F2200'] = \
                    (firstDivisorPowerOfTwo, multiplier, lastDivisorPowerOfTwo, precisionSummand, clampValue)
        return self.precisionData.createFixedPoint(self.fastDiv(result.getInternalRepresentation(), fastDivData), self.precisionData.PRECISION_TRIG_ARG)

    def calculateQuantIdxFromAmplitude(self, isF1200, reciprocalAngularFrequency, amplitude):
        result = (amplitude * self.CONST_INVERSE_TRIG_SCALER)
        fastDivData = self.divData.get('CONST_PRECISION_INVERSE_TRIG_PARAM_DIVISOR')
        if fastDivData is None:
            (inverseTrigArg, self.CONST_PRECISION_INVERSE_TRIG_PARAM_ROUND_SUMMAND, self.CONST_PRECISION_INVERSE_TRIG_PARAM_DIVISOR) = \
                result.convert2Precision(self.precisionData.PRECISION_INVERSE_TRIG_ARG)
            maxValue = max(self.trigTables.sineValues).getInternalRepresentation() * self.CONST_INVERSE_TRIG_SCALER.getInternalRepresentation()
            precisionSummand = self.CONST_PRECISION_INVERSE_TRIG_PARAM_ROUND_SUMMAND
            (firstDivisorPowerOfTwo, multiplier, lastDivisorPowerOfTwo) = \
                self.findBestFastDivision(self.CONST_PRECISION_INVERSE_TRIG_PARAM_DIVISOR, maxValue + precisionSummand)
            clampValue = len(self.trigTables.arcSineValues) - 1
            fastDivData = self.divData['CONST_PRECISION_INVERSE_TRIG_PARAM_DIVISOR'] = \
                (firstDivisorPowerOfTwo, multiplier, lastDivisorPowerOfTwo, precisionSummand, clampValue)
        inverseTrigArg = self.fastDiv(result.getInternalRepresentation(), fastDivData)
        result = (self.trigTables.getScaledArcSineValue(inverseTrigArg) * reciprocalAngularFrequency)
        if isF1200:
            fastDivData = self.divData.get('CONST_PRECISION_QUANT_DIVISOR_F1200')
            if fastDivData is None:
                (inverseTrigArg, self.CONST_PRECISION_QUANT_ROUND_SUMMAND, self.CONST_PRECISION_QUANT_DIVISOR) = \
                    result.convert2Precision(self.precisionData.PRECISION_QUANT)
                maxValue = \
                    max(self.trigTables.arcSineValues).getInternalRepresentation() * self.CONST_RECIPROCAL_ANGULAR_FREQUENCY_F1200.getInternalRepresentation()
                precisionSummand = self.CONST_PRECISION_QUANT_ROUND_SUMMAND
                (firstDivisorPowerOfTwo, multiplier, lastDivisorPowerOfTwo) = self.findBestFastDivision(self.CONST_PRECISION_QUANT_DIVISOR, maxValue + precisionSummand)
                clampValue = self.CONST_F1200_QUANTS_COUNT_PER_SYMBOL.getInternalRepresentation()
                fastDivData = self.divData['CONST_PRECISION_QUANT_DIVISOR_F1200'] = \
                    (firstDivisorPowerOfTwo, multiplier, lastDivisorPowerOfTwo, precisionSummand, clampValue)
        else:
            fastDivData = self.divData.get('CONST_PRECISION_QUANT_DIVISOR_F2200')
            if fastDivData is None:
                (inverseTrigArg, self.CONST_PRECISION_QUANT_ROUND_SUMMAND, self.CONST_PRECISION_QUANT_DIVISOR) = \
                    result.convert2Precision(self.precisionData.PRECISION_QUANT)
                maxValue = \
                    max(self.trigTables.arcSineValues).getInternalRepresentation() * self.CONST_RECIPROCAL_ANGULAR_FREQUENCY_F2200.getInternalRepresentation()
                precisionSummand = self.CONST_PRECISION_QUANT_ROUND_SUMMAND
                (firstDivisorPowerOfTwo, multiplier, lastDivisorPowerOfTwo) = self.findBestFastDivision(self.CONST_PRECISION_QUANT_DIVISOR, maxValue + precisionSummand)
                clampValue = self.CONST_F2200_QUANTS_COUNT_PER_SYMBOL.getInternalRepresentation()
                fastDivData = self.divData['CONST_PRECISION_QUANT_DIVISOR_F2200'] = \
                    (firstDivisorPowerOfTwo, multiplier, lastDivisorPowerOfTwo, precisionSummand, clampValue)
        return self.precisionData.createFixedPoint(self.fastDiv(result.getInternalRepresentation(), fastDivData), self.precisionData.PRECISION_QUANT)
