#include "aprs_board_impl.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define APRS_PAYLOAD_BUFFER_MAX_LENGTH 128

static uint8_t g_aprsPayloadBuffer[APRS_PAYLOAD_BUFFER_MAX_LENGTH];

const Callsign CALLSIGN_SOURCE = 
{
    {"HABHAB"},
    '\xF6' // 111 1011 0
           //          ^ not a last address
           //     ^^^^ SSID (11 - balloon)
           // ^^^ some reserved values and command/response
};

const Callsign CALLSIGN_DESTINATION_1 = 
{
    {"WIDE1 "},
    '\xE2' // 111 0001 0
           //          ^ not a last address
           //     ^^^^ SSID (1 - wide1-1)
           // ^^^ some reserved values and command/response
};

const Callsign CALLSIGN_DESTINATION_2 = 
{
    {"WIDE2 "},
    '\xE5' // 111 0010 1
           //          ^ last address
           //     ^^^^ SSID (2 - wide2-2)
           // ^^^ some reserved values and command/response
};

void advanceBitstreamBit(BitstreamSize* pResultBitstreamSize)
{
    if (pResultBitstreamSize->lastCharBitsCount >= 7)
    {
        ++pResultBitstreamSize->charsCount;
        pResultBitstreamSize->lastCharBitsCount = 0;
    }
    else
    {
        ++pResultBitstreamSize->lastCharBitsCount;
    }
}

bool encodeAndAppendBits(const uint8_t* pMessageData,
                         uint16_t messageDataSize,
                         STUFFING_TYPE stuffingType,
                         FCS_TYPE fcsType,
                         SHIFT_ONE_LEFT_TYPE shiftOneLeftType,
                         EncodingContext* pEncodingContext,
                         AprsEncodedMessage* pResultAprsEncodedMessage)
{
    if (!pResultAprsEncodedMessage || !pEncodingContext || MAX_APRS_MESSAGE_LENGTH < messageDataSize)
    {
        return false;
    }
    if (messageDataSize == 0)
    {
        return true;
    }
    if (!pMessageData)
    {
        return false;
    }

    for (uint16_t iByte = 0; iByte < messageDataSize; ++iByte)
    {
        uint8_t currentByte = pMessageData[iByte];

        if (shiftOneLeftType == SHIFT_ONE_LEFT)
        {
            currentByte <<= 1;
        }

        for (uint8_t iBit = 0; iBit < 8; ++iBit)
        {
            const uint8_t currentBit = currentByte & (1 << iBit);

            // TODO
            // add FCS calculation based on tables
            // to improve speed (need to migrate to GCC due to 
            // 32Kb application size limit in Keil)
            // STM32 has CRC hardware can reuse that

            if (fcsType == FCS_CALCULATE)
            {
                const uint16_t shiftBit = pEncodingContext->fcs & 0x0001;
                pEncodingContext->fcs = pEncodingContext->fcs >> 1;
                if (shiftBit != ((currentByte >> iBit) & 0x01))
                {
                    pEncodingContext->fcs ^= FCS_POLYNOMIAL;
                }
            }

            if (currentBit)
            {
                if (pResultAprsEncodedMessage->size.charsCount >= MAX_APRS_MESSAGE_LENGTH)
                {
                    return false;
                }
                // as we are encoding 1 keep current bit as is
                if (pEncodingContext->lastBit)
                {
                    pResultAprsEncodedMessage->buffer[pResultAprsEncodedMessage->size.charsCount] |= 1 << (pResultAprsEncodedMessage->size.lastCharBitsCount);
                }
                else
                {
                    pResultAprsEncodedMessage->buffer[pResultAprsEncodedMessage->size.charsCount] &= ~(1 << (pResultAprsEncodedMessage->size.lastCharBitsCount));
                }

                advanceBitstreamBit(&pResultAprsEncodedMessage->size);

                if (stuffingType == ST_PERFORM_STUFFING)
                {
                    ++pEncodingContext->numberOfOnes;
                    
                    if (pEncodingContext->numberOfOnes == 5)
                    {
                        if (pResultAprsEncodedMessage->size.charsCount >= MAX_APRS_MESSAGE_LENGTH)
                        {
                            return false;
                        }

                        // we need to insert 0 after 5 consecutive ones
                        if (pEncodingContext->lastBit)
                        {
                            pResultAprsEncodedMessage->buffer[pResultAprsEncodedMessage->size.charsCount] &= ~(1 << (pResultAprsEncodedMessage->size.lastCharBitsCount));
                            pEncodingContext->lastBit = 0;
                        }
                        else
                        {
                            pResultAprsEncodedMessage->buffer[pResultAprsEncodedMessage->size.charsCount] |= 1 << (pResultAprsEncodedMessage->size.lastCharBitsCount);
                            pEncodingContext->lastBit = 1;
                        }
                        
                        pEncodingContext->numberOfOnes = 0;
                        
                        advanceBitstreamBit(&pResultAprsEncodedMessage->size); // insert zero as we had 5 ones
                    }
                }
            }
            else
            {
                if (pResultAprsEncodedMessage->size.charsCount >= MAX_APRS_MESSAGE_LENGTH)
                {
                    return false;
                }
                
                // as we are encoding 0 we need to flip bit
                if (pEncodingContext->lastBit)
                {
                    pResultAprsEncodedMessage->buffer[pResultAprsEncodedMessage->size.charsCount] &= ~(1 << (pResultAprsEncodedMessage->size.lastCharBitsCount));
                    pEncodingContext->lastBit = 0;
                }
                else
                {
                    pResultAprsEncodedMessage->buffer[pResultAprsEncodedMessage->size.charsCount] |= 1 << (pResultAprsEncodedMessage->size.lastCharBitsCount);
                    pEncodingContext->lastBit = 1;
                }

                advanceBitstreamBit(&pResultAprsEncodedMessage->size);

                if (stuffingType == ST_PERFORM_STUFFING)
                {
                    pEncodingContext->numberOfOnes = 0;
                }
            }
        }
    }

    if (stuffingType == ST_NO_STUFFING)
    {
        // reset ones as we didn't do any stuffing while sending this data
        pEncodingContext->numberOfOnes = 0;
    }

    return true;
}

bool isAprsMessageEmtpy(const AprsEncodedMessage* pMessage)
{
    if (!pMessage)
    {
        return true;
    }

    return pMessage->size.charsCount > 0 || pMessage->size.lastCharBitsCount > 0;
}

bool encodeAprsMessage(const Callsign* pCallsign, const uint8_t* aprsPayloadBuffer, uint8_t aprsPayloadBufferLen, AprsEncodedMessage* pEncdedMessage)
{
    if (!pCallsign || !aprsPayloadBuffer || !aprsPayloadBufferLen || !pEncdedMessage)
    {
        return false;
    }

    EncodingContext encodingCtx = { 0 };
    encodingCtx.lastBit = 1;
    encodingCtx.fcs = FCS_INITIAL_VALUE;

    for (uint8_t i = 0; i < PREFIX_FLAGS_COUNT; ++i)
    {
        encodeAndAppendBits((const uint8_t*) "\x7E", 1, ST_NO_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO, &encodingCtx, pEncdedMessage);
    }

    // addresses to and from

    encodeAndAppendBits(CALLSIGN_DESTINATION_1.callsign, 6, ST_PERFORM_STUFFING, FCS_CALCULATE, SHIFT_ONE_LEFT, &encodingCtx, pEncdedMessage);
    encodeAndAppendBits(&CALLSIGN_DESTINATION_1.ssid, 1, ST_PERFORM_STUFFING, FCS_CALCULATE, SHIFT_ONE_LEFT_NO, &encodingCtx, pEncdedMessage);
    encodeAndAppendBits(pCallsign->callsign, 6, ST_PERFORM_STUFFING, FCS_CALCULATE, SHIFT_ONE_LEFT, &encodingCtx, pEncdedMessage);
    encodeAndAppendBits(&pCallsign->ssid, 1, ST_PERFORM_STUFFING, FCS_CALCULATE, SHIFT_ONE_LEFT_NO, &encodingCtx, pEncdedMessage);
    encodeAndAppendBits(CALLSIGN_DESTINATION_2.callsign, 6, ST_PERFORM_STUFFING, FCS_CALCULATE, SHIFT_ONE_LEFT, &encodingCtx, pEncdedMessage);
    encodeAndAppendBits(&CALLSIGN_DESTINATION_2.ssid, 1, ST_PERFORM_STUFFING, FCS_CALCULATE, SHIFT_ONE_LEFT_NO, &encodingCtx, pEncdedMessage);

    // control bytes

    encodeAndAppendBits((const uint8_t*) "\x03", 1, ST_PERFORM_STUFFING, FCS_CALCULATE, SHIFT_ONE_LEFT_NO, &encodingCtx, pEncdedMessage);
    encodeAndAppendBits((const uint8_t*) "\xF0", 1, ST_PERFORM_STUFFING, FCS_CALCULATE, SHIFT_ONE_LEFT_NO, &encodingCtx, pEncdedMessage);

    // packet contents

    encodeAndAppendBits(aprsPayloadBuffer, aprsPayloadBufferLen, ST_PERFORM_STUFFING, FCS_CALCULATE, SHIFT_ONE_LEFT_NO, &encodingCtx, pEncdedMessage);

    // FCS

    encodingCtx.fcs ^= FCS_POST_PROCESSING_XOR_VALUE;
    uint8_t fcsByte = encodingCtx.fcs & 0x00FF; // get low byte
    encodeAndAppendBits(&fcsByte, 1, ST_PERFORM_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO, &encodingCtx, pEncdedMessage);
    fcsByte = (encodingCtx.fcs >> 8) & 0x00FF; // get high byte
    encodeAndAppendBits(&fcsByte, 1, ST_PERFORM_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO, &encodingCtx, pEncdedMessage);

    // suffix flags

    for (uint8_t i = 0; i < SUFFIX_FLAGS_COUNT; ++i)
    {
        encodeAndAppendBits((const uint8_t*) "\x7E", 1, ST_NO_STUFFING, FCS_NONE, SHIFT_ONE_LEFT_NO, &encodingCtx, pEncdedMessage);
    }

    return true;
}

uint8_t createGpsAprsPayload(const GpsData* pGpsData, uint8_t* pAprsPayloadBuffer, uint8_t aprsPayloadBufferMaxLength)
{
    uint8_t bufferStartIdx = 0;

    if (pGpsData->gpggaData.latitude.isValid && pGpsData->gpggaData.longitude.isValid)
    {
        if (pGpsData->gpggaData.utcTime.isValid)
        {
            if (bufferStartIdx + 8 > aprsPayloadBufferMaxLength)
            {
                return 0;
            }

            bufferStartIdx += sprintf((char*) &g_aprsPayloadBuffer[bufferStartIdx],
                                      "@%02u%02u%02uz",
                                      pGpsData->gpggaData.utcTime.hours,
                                      pGpsData->gpggaData.utcTime.minutes,
                                      pGpsData->gpggaData.utcTime.seconds / 100);
        }
        else
        {
            if (bufferStartIdx + 1 > aprsPayloadBufferMaxLength)
            {
                return 0;
            }

            g_aprsPayloadBuffer[bufferStartIdx++] = '!';
        }

        if (bufferStartIdx + 20 > aprsPayloadBufferMaxLength)
        {
            return 0;
        }

        const unsigned int latMinutesWhole = pGpsData->gpggaData.latitude.minutes / 1000000;
        const unsigned int latMinutesFraction = (pGpsData->gpggaData.latitude.minutes - latMinutesWhole * 1000000) / 10000;

        const unsigned int lonMinutesWhole = pGpsData->gpggaData.longitude.minutes / 1000000;
        const unsigned int lonMinutesFraction = (pGpsData->gpggaData.longitude.minutes - lonMinutesWhole * 1000000) / 10000;

        bufferStartIdx += sprintf((char*) &g_aprsPayloadBuffer[bufferStartIdx],
                                  "%02u%02u.%02u%1c/%03u%02u.%02u%1cO",
                                  pGpsData->gpggaData.latitude.degrees,
                                  latMinutesWhole,
                                  latMinutesFraction,
                                  pGpsData->gpggaData.latitude.hemisphere,
                                  pGpsData->gpggaData.longitude.degrees,
                                  lonMinutesWhole,
                                  lonMinutesFraction,
                                  pGpsData->gpggaData.longitude.hemisphere);

        if (bufferStartIdx + 7 > aprsPayloadBufferMaxLength)
        {
            return 0;
        }

        bufferStartIdx += sprintf((char*) &g_aprsPayloadBuffer[bufferStartIdx],
                                  ">%03u/%03u",
                                  (unsigned int) (pGpsData->gpvtgData.trueCourseDegrees / 10),
                                  (unsigned int) (pGpsData->gpvtgData.speedKph / 10));

        if (bufferStartIdx + 7 > aprsPayloadBufferMaxLength)
        {
            return 0;
        }

        bufferStartIdx += sprintf((char*) &g_aprsPayloadBuffer[bufferStartIdx],
                                  "@%05um",
                                  (unsigned int) pGpsData->gpggaData.altitudeMslMeters / 10);
    }

    return bufferStartIdx;
}

bool encodeGpsAprsMessage(const Callsign* pCallsign, const GpsData* pGpsData, AprsEncodedMessage* pEncdedMessage)
{
    uint8_t aprsPayloadBufferDataLength = createGpsAprsPayload(pGpsData, g_aprsPayloadBuffer, APRS_PAYLOAD_BUFFER_MAX_LENGTH);
    return encodeAprsMessage(pCallsign, g_aprsPayloadBuffer, aprsPayloadBufferDataLength, pEncdedMessage);
}

uint8_t createTelemetryAprsPayload(const Telemetry* pTelemetry, uint8_t* pAprsPayloadBuffer, uint8_t aprsPayloadBufferMaxLength)
{
    return 0; // TODO
}

bool encodeTelemetryAprsMessage(const Callsign* pCallsign, const Telemetry* pTelemetry, AprsEncodedMessage* pEncdedMessage)
{
    uint8_t aprsPayloadBufferDataLength = createTelemetryAprsPayload(pTelemetry, g_aprsPayloadBuffer, APRS_PAYLOAD_BUFFER_MAX_LENGTH);
    return encodeAprsMessage(pCallsign, g_aprsPayloadBuffer, aprsPayloadBufferDataLength, pEncdedMessage);
}

bool encodeAprsMessageAsAfsk(const AprsEncodedMessage* pMessage, uint16_t* pOutputBuffer, uint32_t outputBufferSize)
{
    return false; // TODO
}