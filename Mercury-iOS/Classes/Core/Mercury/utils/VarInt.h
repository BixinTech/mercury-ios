#pragma once
template<typename int_t = uint64_t>
size_t encodeVarint(int_t value, uint8_t* output) {
    size_t outputSize = 0;
    //While more than 7 bits of data are left, occupy the last output byte
    // and set the next byte flag
    while (value > 127) {
        //|128: Set the next byte flag
        output[outputSize] = ((uint8_t)(value & 127)) | 128;
        //Remove the seven bits we just wrote
        value >>= 7;
        outputSize++;
    }
    output[outputSize++] = ((uint8_t)value) & 127;
    return outputSize;
}
/**
 * Decodes an unsigned variable-length integer using the MSB algorithm.
 * @param input A variable-length encoded integer of arbitrary size.
 * @param inputSize How many bytes are
 */
template<typename int_t = uint64_t>
int_t decodeVarint(const uint8_t* input, size_t& inputSize) {
    int_t ret = 0;
    for (size_t i = 0; i < 8; i++) {
        ret |= (input[i] & 127) << (7 * i);
        // If the next-byte flag is set
        if(!(input[i] & 128)) {
            inputSize = i + 1;
            
            break;
        }
    }
    return ret;
}


