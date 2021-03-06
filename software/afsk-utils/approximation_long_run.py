import numpy
import matplotlib.pyplot as plot

import definitions
import afsk_modulation
import data_generation
import afsk_modulation_fixedpoint_fast_div

bitsCount = int(128 * 8 + 128 / 5 * 8 + 1)

errors = []
bestMaxError = float('+inf')
worstMaxError = float('-inf')

ITERATIONS_COUNT = 10000
BUCKETS_COUNT = 500

clampingNeeded = False

for i in range(ITERATIONS_COUNT):
    data = data_generation.generateBytes(bitsCount)

    floatingPointModulation = afsk_modulation.AfskModulation(data, bitsCount)
    afskSignalData = floatingPointModulation.afskModulate()

    x = numpy.linspace(1, len(afskSignalData), len(afskSignalData))
    y = numpy.array(afskSignalData)

    fixedPointModulation = afsk_modulation_fixedpoint_fast_div.AfskModulationFixedPointFastDiv(definitions.FixedPointBase2FastDivPrecisionData(), data, bitsCount)
    afskSignalDataFixedPoint = fixedPointModulation.afskModulate()
    yFixedPoint = numpy.array(afskSignalDataFixedPoint)

    clampingNeeded = clampingNeeded or fixedPointModulation.CLAMPED_VALUE

    yError = []
    for j in range(len(afskSignalData)):
        yError.append(numpy.abs(afskSignalData[j] - afskSignalDataFixedPoint[j]))

    maxError = numpy.max(yError)

    errors.append(maxError)

    if maxError < bestMaxError:
        bestMaxError = maxError
        data_generation.saveBytes(data, "data-best-max-{:0>4}.txt".format(bestMaxError))

    if maxError > worstMaxError:
        worstMaxError = maxError
        data_generation.saveBytes(data, "data-worst-max-{:0>4}.txt".format(worstMaxError))

    print("iteration " + str(i) + " out of " + str(ITERATIONS_COUNT) + ", max error = " + str(maxError))

data_generation.saveBytes(errors, "errors.txt")

print("best max error = " + str(bestMaxError))
print("worst max error = " + str(worstMaxError))
print("clamping needed = " + str(clampingNeeded))
print("errors median = " + str(numpy.median(numpy.array(errors))))
print("errors standard deviation = " + str(numpy.std(numpy.array(errors))))
print()

plot.title("trig size = " + str(definitions.TRIG_TABLE_SIZE) + ", inverse trig size = " + str(definitions.INVERSE_TRIG_TABLE_SIZE))
plot.hist(numpy.array(errors), bins = BUCKETS_COUNT, range = (0, 4100))
plot.grid(True)
plot.show()
