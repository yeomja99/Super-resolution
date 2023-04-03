#include <stdio.h>

int main() {
    int OW = 1920 * 2;
    int OH = 1080 * 2;
    int ks3 = 9;
    
    int out_size = OH * OW;
    int PW3 = 3848;
    int y3 = 0;
    int k = ks3 * PW3;
    for (int z = 0; z < OH; z++) { // z: Height, yÃà
        int h_start = z * 1;
        int h_end = h_start + k;
        for (int t = 0; t < OW; t++) { // t: Width, xÃà
            int w_start = t * 1;
            int w_end = w_start + ks3;
            float sumb = 0;
            int kx = 0;
            int ky = 0;
            for (int i = h_start; i < h_end; i += PW3) {
                for (int j = w_start; j < w_end; j++) {
                    //sumb = sumb + kernelT[ky + kx] * float(Padding3[i + j]);
                    printf("kernel index: %d, padding index: %d\n", kx + ky, i + j);
                    kx++;

                }
                kx = 0;
                ky += ks3;
            }

        }
        y3 += OW;

    }

    return 0;
}

