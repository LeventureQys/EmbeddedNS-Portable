#include <stdio.h>
#include <string.h>
#include<math.h>
#include <stdint.h>
void vec_k_div(float* out, float* in, float k, uint32_t n);

#define N_FFT 256
#define N_SELEC (N_FFT/2 + 1)
#define N_table (N_FFT/4)
#define LOG 8 //log2(N_FFT) = 10
const int32_t BRTable[N_FFT] = {
0, 128, 64, 192, 32, 160, 96, 224, 16, 144, 80, 208, 48, 176, 112, 240,
8, 136, 72, 200, 40, 168, 104, 232, 24, 152, 88, 216, 56, 184, 120, 248,
4, 132, 68, 196, 36, 164, 100, 228, 20, 148, 84, 212, 52, 180, 116, 244,
12, 140, 76, 204, 44, 172, 108, 236, 28, 156, 92, 220, 60, 188, 124, 252,
2, 130, 66, 194, 34, 162, 98, 226, 18, 146, 82, 210, 50, 178, 114, 242,
10, 138, 74, 202, 42, 170, 106, 234, 26, 154, 90, 218, 58, 186, 122, 250,
6, 134, 70, 198, 38, 166, 102, 230, 22, 150, 86, 214, 54, 182, 118, 246,
14, 142, 78, 206, 46, 174, 110, 238, 30, 158, 94, 222, 62, 190, 126, 254,
1, 129, 65, 193, 33, 161, 97, 225, 17, 145, 81, 209, 49, 177, 113, 241,
9, 137, 73, 201, 41, 169, 105, 233, 25, 153, 89, 217, 57, 185, 121, 249,
5, 133, 69, 197, 37, 165, 101, 229, 21, 149, 85, 213, 53, 181, 117, 245,
13, 141, 77, 205, 45, 173, 109, 237, 29, 157, 93, 221, 61, 189, 125, 253,
3, 131, 67, 195, 35, 163, 99, 227, 19, 147, 83, 211, 51, 179, 115, 243,
11, 139, 75, 203, 43, 171, 107, 235, 27, 155, 91, 219, 59, 187, 123, 251,
7, 135, 71, 199, 39, 167, 103, 231, 23, 151, 87, 215, 55, 183, 119, 247,
15, 143, 79, 207, 47, 175, 111, 239, 31, 159, 95, 223, 63, 191, 127, 255,
};

const float cos_table[N_table + 1] = {
1, 0.9997, 0.9988, 0.99729, 0.99518, 0.99248, 0.98918, 0.98528, 0.98079, 0.9757, 0.97003, 0.96378, 0.95694, 0.94953, 0.94154, 0.93299,
0.92388, 0.91421, 0.90399, 0.89322, 0.88192, 0.87009, 0.85773, 0.84485, 0.83147, 0.81758, 0.80321, 0.78835, 0.77301, 0.75721, 0.74095, 0.72425,
0.70711, 0.68954, 0.67156, 0.65317, 0.63439, 0.61523, 0.5957, 0.57581, 0.55557, 0.535, 0.5141, 0.4929, 0.4714, 0.44961, 0.42756, 0.40524,
0.38268, 0.3599, 0.33689, 0.31368, 0.29028, 0.26671, 0.24298, 0.2191, 0.19509, 0.17096, 0.14673, 0.12241, 0.098017, 0.073565, 0.049068, 0.024541,
6.1232e-17
};

const float sin_table[N_table + 1] = {
0, 0.024541, 0.049068, 0.073565, 0.098017, 0.12241, 0.14673, 0.17096, 0.19509, 0.2191, 0.24298, 0.26671, 0.29028, 0.31368, 0.33689, 0.3599,
0.38268, 0.40524, 0.42756, 0.44961, 0.4714, 0.4929, 0.5141, 0.535, 0.55557, 0.57581, 0.5957, 0.61523, 0.63439, 0.65317, 0.67156, 0.68954,
0.70711, 0.72425, 0.74095, 0.75721, 0.77301, 0.78835, 0.80321, 0.81758, 0.83147, 0.84485, 0.85773, 0.87009, 0.88192, 0.89322, 0.90399, 0.91421,
0.92388, 0.93299, 0.94154, 0.94953, 0.95694, 0.96378, 0.97003, 0.9757, 0.98079, 0.98528, 0.98918, 0.99248, 0.99518, 0.99729, 0.9988, 0.9997,
1
};

#define N_FFT_TS 128
#define N_SELEC_TS (N_FFT_TS/2 + 1)
#define N_table_TS (N_FFT_TS/4)
#define LOG_TS 7 //log2(N_FFT_TS) = 7
const int32_t BRTable_TS[N_FFT_TS] = {
    0, 64, 32, 96, 16, 80, 48, 112, 8, 72, 40, 104, 24, 88, 56, 120,
4, 68, 36, 100, 20, 84, 52, 116, 12, 76, 44, 108, 28, 92, 60, 124,
2, 66, 34, 98, 18, 82, 50, 114, 10, 74, 42, 106, 26, 90, 58, 122,
6, 70, 38, 102, 22, 86, 54, 118, 14, 78, 46, 110, 30, 94, 62, 126,
1, 65, 33, 97, 17, 81, 49, 113, 9, 73, 41, 105, 25, 89, 57, 121,
5, 69, 37, 101, 21, 85, 53, 117, 13, 77, 45, 109, 29, 93, 61, 125,
3, 67, 35, 99, 19, 83, 51, 115, 11, 75, 43, 107, 27, 91, 59, 123,
7, 71, 39, 103, 23, 87, 55, 119, 15, 79, 47, 111, 31, 95, 63, 127,
};

const float sin_table_TS[N_table_TS + 1] = {
    0, 0.0490677, 0.0980171, 0.14673, 0.19509, 0.24298, 0.290285, 0.33689, 0.382683, 0.427555, 0.471397, 0.514103, 0.55557, 0.595699, 0.634393, 0.671559,
0.707107, 0.740951, 0.77301, 0.803208, 0.83147, 0.857729, 0.881921, 0.903989, 0.92388, 0.941544, 0.95694, 0.970031, 0.980785, 0.989177, 0.995185, 0.998795, 1.0,
};

const float cos_table_TS[N_table_TS + 1] = {
    1.0, 0.998795, 0.995185, 0.989177, 0.980785, 0.970031, 0.95694, 0.941544, 0.92388, 0.903989, 0.881921, 0.857729, 0.83147, 0.803208, 0.77301, 0.740951,
0.707107, 0.671559, 0.634393, 0.595699, 0.55557, 0.514103, 0.471397, 0.427555, 0.382683, 0.33689, 0.290285, 0.24298, 0.19509, 0.14673, 0.0980171, 0.0490677, 0.0,
};

void FFT_table(float *arr, float *result) {
    uint32_t bb, p, i, j, k;
    float TReal, Timage, cos_value, sin_value;
    float FFTreal[N_FFT], FFTimage[N_FFT], fft_in[N_FFT];
    memset(FFTreal, 0, sizeof(float) * N_FFT);
    memset(FFTimage, 0, sizeof(float) * N_FFT);
    memcpy(fft_in, arr, sizeof(float) * N_FFT);

    for (i = 0; i < N_FFT; i++) {
        FFTreal[BRTable[i]] = fft_in[i];
        FFTimage[i] = 0;     
    }
  
    for (i = 1; i <= LOG; i++) {
        bb = 1;
        bb <<= (i - 1);
        for (j = 0; j <= (bb - 1); j++) {
            p = 1;
            p <<= (LOG -i);
            p *= j;
            for (k = j; k < N_FFT; k = k + 2 * bb) {
                if (p <= N_table) {
                    cos_value = cos_table[p];
                    sin_value = sin_table[p];
                }
                if (p > N_table && p <= 2 * N_table) {
                    cos_value = -cos_table[2 * N_table - p];
                    sin_value = sin_table[2 * N_table - p];
                }
                if (p > 2 * N_table && p <= 3 * N_table) {
                    cos_value = -cos_table[p - 2 * N_table];
                    sin_value = -sin_table[p - 2 * N_table];
                }
                if (p > 3 * N_table && p < N_FFT) {
                    cos_value = cos_table[N_FFT - p];
                    sin_value = -sin_table[N_FFT - p];
                }
                TReal = (FFTreal[k + bb] * cos_value) + (FFTimage[k + bb] * sin_value);

                Timage = (FFTimage[k + bb] * cos_value) - (FFTreal[k + bb] * sin_value);
                FFTreal[k + bb] = FFTreal[k] - TReal;
                FFTimage[k + bb] = FFTimage[k] - Timage;

                FFTreal[k] = FFTreal[k] + TReal;
                FFTimage[k] = FFTimage[k] + Timage;
            }
        }
    }
    memcpy(fft_in, FFTreal, sizeof(float) * N_FFT);
    for (i = 0; i < N_FFT / 2 + 1; i++) {
        result[2 * i] = FFTreal[i];
        result[2 * i + 1] =  -FFTimage[i];
    }
    return;
}

void IFFT_table(float* arr, float* result) {
    uint32_t i;
    float temp_vect[N_FFT * 2], ifft_in[N_FFT + 2], fft_out1[N_FFT + 2], temp_data2[N_FFT];
    memcpy(ifft_in, arr, (N_FFT + 2) * sizeof(float));
    memset(temp_vect, 0, sizeof(float) * N_FFT * 2);


    float* temp_data1 = ifft_in;
    // memset(temp_data1, 0, sizeof(float) * N_FFT);
    memset(temp_data2, 0, sizeof(float) * N_FFT);
    memset(fft_out1, 0, sizeof(float) * (N_FFT + 2));
    // memset(fft_out2, 0, sizeof(float) * (N_FFT + 2));
    for (i = 0; i < N_FFT / 2 - 2; i++) {
        temp_vect[N_FFT + 2 + 2 * i] = ifft_in[2 * (N_SELEC - i) - 4];
        temp_vect[N_FFT + 2 + 2 * i + 1] = -ifft_in[2 * (N_SELEC - i) - 3];
    }
    memcpy(temp_vect, arr, (N_FFT + 2) * sizeof(float));
    for (i = 0; i < N_FFT; i++) {
        temp_data1[i] = temp_vect[2 * i];
        temp_data2[i] = temp_vect[2 * i + 1];
    }

    FFT_table(temp_data1, fft_out1);

    float* fft_out2 = ifft_in;

    FFT_table(temp_data2, fft_out2);
    vec_k_div(fft_out1, fft_out1, (float)N_FFT, N_FFT + 2);
    vec_k_div(fft_out2, fft_out2, (float)N_FFT, N_FFT + 2);
    memset(temp_vect, 0, sizeof(float) * N_FFT * 2);
    for (i = 0; i < N_FFT / 2 - 1; i++) {
        temp_vect[N_FFT + 2 + 2 * i] = fft_out1[2 * (N_SELEC - i) - 4];
        temp_vect[N_FFT + 2 + 2 * i + 1] = -fft_out2[2 * (N_SELEC - i) - 3];
    }
    memcpy(temp_vect, fft_out1, (N_FFT + 2) * sizeof(float));
    for (i = 0; i < N_FFT; i++) {
        if (i < N_SELEC) {
            result[i] = temp_vect[2 * i] + fft_out2[2 * i + 1];
        }
        else {
            result[i] = temp_vect[2 * i] + temp_vect[2 * i + 1];
        }
    }
    return;
}

void FFT_table_TS(float *arr, float *result) {
    uint32_t bb, p, i, j, k;
    float TReal, Timage, cos_value, sin_value;
    float FFTreal[N_FFT_TS], FFTimage[N_FFT_TS], fft_in[N_FFT_TS];
    memset(FFTreal, 0, sizeof(float) * N_FFT_TS);
    memset(FFTimage, 0, sizeof(float) * N_FFT_TS);
    memcpy(fft_in, arr, sizeof(float) * N_FFT_TS);

    for (i = 0; i < N_FFT_TS; i++) {
        FFTreal[BRTable_TS[i]] = fft_in[i];
        FFTimage[i] = 0;
    }
  
    for (i = 1; i <= LOG_TS; i++) {
        bb = 1;
        bb <<= (i - 1);
        for (j = 0; j <= (bb - 1); j++) {
            p = 1;
            p <<= (LOG_TS -i);
            p *= j;
            for (k = j; k < N_FFT_TS; k = k + 2 * bb) {
                if (p <= N_table) {
                    cos_value = cos_table_TS[p];
                    sin_value = sin_table_TS[p];
                }
                if (p > N_table_TS && p <= 2 * N_table_TS) {
                    cos_value = -cos_table_TS[2 * N_table_TS - p];
                    sin_value = sin_table_TS[2 * N_table_TS - p];
                }
                if (p > 2 * N_table_TS && p <= 3 * N_table_TS) {
                    cos_value = -cos_table_TS[p - 2 * N_table_TS];
                    sin_value = -sin_table_TS[p - 2 * N_table_TS];
                }
                if (p > 3 * N_table_TS && p < N_FFT_TS) {
                    cos_value = cos_table_TS[N_FFT_TS - p];
                    sin_value = -sin_table_TS[N_FFT_TS - p];
                }
                TReal = (FFTreal[k + bb] * cos_value) + (FFTimage[k + bb] * sin_value);
                Timage = (FFTimage[k + bb] * cos_value) - (FFTreal[k + bb] * sin_value);
                FFTreal[k + bb] = FFTreal[k] - TReal;
                FFTimage[k + bb] = FFTimage[k] - Timage;

                FFTreal[k] = FFTreal[k] + TReal;
                FFTimage[k] = FFTimage[k] + Timage;
            }
        }
    }
    memcpy(fft_in, FFTreal, sizeof(float) * N_FFT_TS);
    for (i = 0; i < N_FFT_TS / 2 + 1; i++) {
        result[2 * i] = FFTreal[i];
        result[2 * i + 1] = FFTimage[i];
    }
    return;
}

void IFFT_table_TS(float* arr, float* result) {
    uint32_t i;
    float temp_vect[N_FFT_TS * 2], ifft_in[N_FFT_TS + 2], fft_out1[N_FFT_TS + 2], temp_data2[N_FFT_TS];
    memcpy(ifft_in, arr, (N_FFT_TS + 2) * sizeof(float));
    memset(temp_vect, 0, sizeof(float) * N_FFT_TS * 2);


    float* temp_data1 = ifft_in;
    // memset(temp_data1, 0, sizeof(float) * N_FFT_TS);
    memset(temp_data2, 0, sizeof(float) * N_FFT_TS);
    memset(fft_out1, 0, sizeof(float) * (N_FFT_TS + 2));
    // memset(fft_out2, 0, sizeof(float) * (N_FFT_TS + 2));
    for (i = 0; i < N_FFT_TS / 2 - 2; i++) {
        temp_vect[N_FFT_TS + 2 + 2 * i] = ifft_in[2 * (N_SELEC_TS - i) - 4];
        temp_vect[N_FFT_TS + 2 + 2 * i + 1] = -ifft_in[2 * (N_SELEC_TS - i) - 3];
    }
    memcpy(temp_vect, arr, (N_FFT_TS + 2) * sizeof(float));
    for (i = 0; i < N_FFT_TS; i++) {
        temp_data1[i] = temp_vect[2 * i];
        temp_data2[i] = temp_vect[2 * i + 1];
    }

    FFT_table_TS(temp_data1, fft_out1);

    float* fft_out2 = ifft_in;

    FFT_table_TS(temp_data2, fft_out2);
    vec_k_div(fft_out1, fft_out1, (float)N_FFT_TS, N_FFT_TS + 2);
    vec_k_div(fft_out2, fft_out2, (float)N_FFT_TS, N_FFT_TS + 2);
    memset(temp_vect, 0, sizeof(float) * N_FFT_TS * 2);
    for (i = 0; i < N_FFT_TS / 2 - 1; i++) {
        temp_vect[N_FFT_TS + 2 + 2 * i] = fft_out1[2 * (N_SELEC_TS - i) - 4];
        temp_vect[N_FFT_TS + 2 + 2 * i + 1] = -fft_out2[2 * (N_SELEC_TS - i) - 3];
    }
    memcpy(temp_vect, fft_out1, (N_FFT_TS + 2) * sizeof(float));
    for (i = 0; i < N_FFT_TS; i++) {
        if (i < N_SELEC_TS) {
            result[i] = temp_vect[2 * i] + fft_out2[2 * i + 1];
        }
        else {
            result[i] = temp_vect[2 * i] + temp_vect[2 * i + 1];
        }
    }
    return;
}

void vec_k_div(float* out, float* in, float k, uint32_t n) {
    uint32_t i;
    for (i = 0; i < n; i++) {
        *out = *in / k;
        out++;
        in++;
    }
}

//void IFFT_table(float *arr, float *result) {
//    uint32_t i;
//    float temp_vect[N_FFT * 2], temp_data1[N_FFT + 2], temp_data2[N_FFT + 2], ifft_in[N_FFT + 2];
//    float *fft_out = temp_vect;
//    memcpy(ifft_in, arr, (N_FFT + 2) * sizeof(float));
//    memset(temp_vect, 0, sizeof(float) * N_FFT * 2);
//    memset(temp_data1, 0, sizeof(float) * (N_FFT + 2));
//    memset(temp_data2, 0, sizeof(float) * (N_FFT + 2));
//    for (i = 0; i < N_FFT / 2 - 2; i++) {
//        temp_vect[N_FFT + 2 + 2 * i] = ifft_in[2 * (N_SELEC - i) - 4];
//        temp_vect[N_FFT + 2 + 2 * i + 1] = -ifft_in[2 * (N_SELEC - i) - 3];
//    }
//    memcpy(temp_vect, arr, (N_FFT + 2) * sizeof(float));
//    for (i = 0; i < N_FFT; i++) {
//        temp_data1[i] = temp_vect[2 * i];
//        temp_data2[i] = temp_vect[2 * i + 1];
//    }
//    memset(temp_vect, 0, sizeof(float) * N_FFT * 2);
//    FFT_table(temp_data1, fft_out);
//    FFT_table(temp_data2, temp_data1);
//
//    for (i = 0; i < N_FFT / 2 - 2; i++) {
//        fft_out[N_FFT + 2 + 2 * i] = fft_out[2 * (N_SELEC - i) - 3];
//        fft_out[N_FFT + 2 + 2 * i + 1] = - temp_data1[2 * (N_SELEC - i) - 2];
//    }
//
//    for (i = 0; i < N_FFT; i++) {
//        if (i < N_SELEC) {
//            result[i] = (fft_out[2 * i] + temp_data1[2 * i + 1]) / N_FFT;
//        } else {
//            result[i] = (fft_out[2 * i] + fft_out[2 * i + 1]) / N_FFT;
//        }
//    }
//    return;
//}
