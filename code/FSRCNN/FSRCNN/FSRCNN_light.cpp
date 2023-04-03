#include "util.h"


int scale = 2;

int main() {

    clock_t getimage_S = clock();

    char file_dir[255] = "./museum-01_spp_1_0.bmp";
    BITMAPFILEHEADER hf; // 14바이트
    BITMAPINFOHEADER hInfo; // 40바이트
    RGBQUAD hRGB[256]; // 1024바이트
    FILE* fp;
    fp = fopen(file_dir, "rb");
    printf("%s\n", file_dir);

    if (fp == NULL) {
        printf("File not found!\n");
        return -1;
    }

    fread(&hf, sizeof(BITMAPFILEHEADER), 1, fp);
    fread(&hInfo, sizeof(BITMAPINFOHEADER), 1, fp);

    int ImgSize = hInfo.biWidth * hInfo.biHeight;
    int size = hInfo.biSizeImage; // 픽셀 데이터 크기
    int H = hInfo.biHeight, W = hInfo.biWidth;

    BYTE* Image;

    if (hInfo.biBitCount == 24) { // 트루컬러
        Image = (BYTE*)malloc(ImgSize * 3);
        fread(Image, sizeof(BYTE), ImgSize * 3, fp);
    }

    else { // 인덱스(그레이)
        fread(hRGB, sizeof(RGBQUAD), 256, fp);
        Image = (BYTE*)malloc(ImgSize);
        fread(Image, sizeof(BYTE), ImgSize, fp);
    }

    clock_t getimage_E = clock();
    printf("이미지 GET 소요 시간: \t%.3f\n", (float)(getimage_E - getimage_S) / CLOCKS_PER_SEC);

    int outsize = W * H * scale * scale;
    int OW = W * scale;
    int OH = H * scale;

    // bicubic 으로 upscaling 후 Cb, Cr 값 추출
    BYTE* bc_output = (BYTE*)malloc(outsize * 3);
    _bcSM SM; // Sparse Matrix 선언
    SM.n = scale * scale * W * H;
    SM.SR = (_bcSR*)calloc(SM.n, sizeof(_bcSR));
    BC_MakeSparseMatrix(Image, bc_output, SM, W, H, (float)scale);
    TESTBC(Image, bc_output, W, H, ImgSize, (float)scale, SM);

    // 상하 반전 제거
    BYTE* realign_ = (BYTE*)malloc(sizeof(BYTE) * ImgSize * 3);
    BYTE* bc_realign = (BYTE*)malloc(sizeof(BYTE) * outsize * 3);

    realign(realign_, Image, W, H);
    realign(bc_realign, bc_output, W * scale, H * scale);

    // bgr --> YCbCr로 변환하기(Y만 쓸 것)
    float* Y = (float*)malloc(sizeof(float) * ImgSize); // YCbCr 중에 Y만 사용
    float* Cb = (float*)malloc(sizeof(float) * outsize); // YCbCr 중에 Y만 사용
    float* Cr = (float*)malloc(sizeof(float) * outsize); // YCbCr 중에 Y만 사용

    clock_t toycbcr_S = clock();
    int k = 0;
    for (int i = 0; i < ImgSize; i++) {
        Y[i] = (16.0 + (realign_[k] * 25.064 + realign_[k + 1] * 129.057 + realign_[k + 2] * 64.738) / 256.0) / 255.0;
        k += 3;
    }
    k = 0;
    for (int i = 0; i < ImgSize * scale * scale; i++) {
        Cb[i] = 128.0 + (bc_realign[k] * 112.439 + bc_realign[k + 1] * -74.494 + bc_realign[k + 2] * -37.945) / 256.0;
        Cr[i] = 128.0 + (bc_realign[k] * -18.285 + bc_realign[k + 1] * -94.154 + bc_realign[k + 2] * 112.439) / 256.0;
        k += 3;
    }
    clock_t toycbcr_E = clock();
    printf("채널 변환 소요시간: %.3f\n", (float)(toycbcr_E - toycbcr_S) / CLOCKS_PER_SEC);

    /* first part */
    clock_t totalS = clock();

    printf("\n========== FIRST PART ==========\n");
    // conv1 layer
    printf("\nCONV1 LAYER\n");
    int kn1 = 3;
    int ks1 = 3;
    float* kernel1 = (float*)malloc(sizeof(float) * ks1 * ks1);
    int stride1 = 1;
    int pad1 = 1;
    int PW1 = W + (pad1 * 2);
    int PH1 = H + (pad1 * 2);
    float* Padding1 = (float*)malloc(sizeof(float) * PW1 * PH1);
    memset(Padding1, 0, sizeof(float) * PW1 * PH1);
    padding_initialize(Padding1, Y, PW1, PH1, pad1);

    char buffer[300];
    // bias 불러오기
    float bias1[3];
    getbias_("./parameter_light/bias/first/bias0_0.txt", bias1);

    // PReLU weight 불러오기
    float PReLU_weight1[3];
    getbias_("./parameter_light/PReLU/first/weight0_0.txt", PReLU_weight1);

    // first_part: Convolution 연산 수행(conv1: kernel 56개)
    clock_t conv1_s = clock();
    _Output output1[3];

    for (int fn = 0; fn < kn1; fn++) {

        output1[fn].feature_map = (float*)malloc(sizeof(float) * ImgSize);
        memset(output1[fn].feature_map, 0, sizeof(float) * ImgSize);

        // kernel 1개씩 불러오기 
        char kernelfile_dir[255] = "./parameter_light/kernel/first/kernel0_";
        strcat(kernelfile_dir, strcat(itoa(fn, buffer, 10), ".txt"));
        getbias_(kernelfile_dir, kernel1);

        clock_t start;
        clock_t end;

        start = clock();

        for (int z = 0; z < H; z++) { // z: Height, y축
            for (int t = 0; t < W; t++) { // t: Width, x축
                _element sr[9];
                for (int i = 0; i < ks1; i++) { // y축 kernel size
                    for (int j = 0; j < ks1; j++) { // x축 kernel size
                        sr[i * ks1 + j].cols = (t + ((W + pad1 + pad1) * i) + j + (z * (W + pad1 + pad1)));
                        sr[i * ks1 + j].w = kernel1[i * ks1 + j];
                    }
                }

                float sumb = 0;
                sumb = 0;
                for (int j = 0; j < ks1; j++) {
                    for (int k = 0; k < ks1; k++) {
                        sumb = sumb + sr[j * ks1 + k].w * (float)Padding1[sr[j * ks1 + k].cols];
                    }
                }
                output1[fn].feature_map[z * W + t] = sumb;

            }
        }
        end = clock();
        printf("convolution: %.3f\n", (float)(end - start) / CLOCKS_PER_SEC);


        // bias 더하기 진행 후 PReLU 진행
        clock_t relu_S = clock();
        for (int index = 0; index < ImgSize; index++) {
            if (output1[fn].feature_map[index] + bias1[fn] >= 0) { output1[fn].feature_map[index] = output1[fn].feature_map[index] + bias1[fn]; }
            else { output1[fn].feature_map[index] = PReLU_weight1[fn] * (output1[fn].feature_map[index] + bias1[fn]); }

        }

        clock_t relu_E = clock();
        printf("Bias 덧셈 후 ReLU 소요 시간: %.3f\n", (float)(relu_E - relu_S) / CLOCKS_PER_SEC);
        //SaveFeatureMap_f(W, H, output1[fn].feature_map, fn, "first_part/");

    }
    free(Y);
    free(realign_);
    free(Padding1);
    free(Image);
    clock_t conv1_e = clock();
    printf("convolution layer 1 소요 시간: %.3f\n", (float)(conv1_e - conv1_s) / CLOCKS_PER_SEC);


    /* mid part */
    printf("\n========== MID PART ==========\n");

    int kn2 = 3;
    int ks2 = 3;
    float* kernel2 = (float*)malloc(sizeof(float) * ks2 * ks2);
    int stride2 = 1;
    int pad2 = 1;
    int PW2 = W + (pad2 * 2);
    int PH2 = H + (pad2 * 2);

    // bias 불러오기
    float bias2[3];
    getbias_("./parameter_light/bias/mid/bias0_0.txt", bias2);

    // PReLU weight 불러오기
    float PReLU_weight2[3];
    getbias_("./parameter_light/PReLU/mid/weight0_0.txt", PReLU_weight2);

    // conv2 layer: Convolution 연산 수행(conv1: kernel 56개)
    clock_t conv2S = clock();
    _Output output2[3];

    for (int on = 0; on < kn2; on++) {
        output2[on].feature_map = (float*)malloc(sizeof(float) * ImgSize);
        memset(output2[on].feature_map, 0, sizeof(float) * ImgSize);

        for (int kn = 0; kn < kn2; kn++) {

            float* Padding2_2 = (float*)malloc(sizeof(float) * PW2 * PH2);
            float* TempY = (float*)malloc(sizeof(float) * ImgSize);
            memset(Padding2_2, 0, sizeof(float) * PW2 * PH2);
            memset(TempY, 0, sizeof(float) * ImgSize);

            char onnum[10];
            char temponnum[10];
            itoa(on, temponnum, 10);
            itoa(on, onnum, 10);
            char kernelfile_dir[255] = "./parameter_light/kernel/mid/";
            //strcat(kernelfile_dir, strcat(onnum, "/"));
            //char buffer[30];
            char kernel[30] = "kernel";
            itoa(kn, buffer, 10);
            printf("%d\n", kn);
            char sign[10] = "_";
            strcat(kernelfile_dir, strcat(kernel, strcat(buffer, strcat(strcat(sign, temponnum), ".txt"))));
            getbias_(kernelfile_dir, kernel2);

            // featuremap 불러오기

            padding_initialize(Padding2_2, output1[kn].feature_map, PW2, PH2, pad2);

            clock_t start;
            clock_t end;

            start = clock();

            for (int z = 0; z < H; z++) { // z: Height, y축
                for (int t = 0; t < W; t++) { // t: Width, x축
                    _element sr[9];
                    for (int i = 0; i < ks2; i++) { // y축 kernel size
                        for (int j = 0; j < ks2; j++) { // x축 kernel size
                            sr[i * ks2 + j].cols = (t + ((W + pad2 + pad2) * i) + j + (z * (W + pad2 + pad2)));
                            sr[i * ks2 + j].w = kernel2[i * ks2 + j];
                            // cnt = cnt + 1;
                        }
                    }

                    float sumb = 0;
                    sumb = 0;
                    for (int j = 0; j < ks2; j++) {
                        for (int k = 0; k < ks2; k++) {
                            sumb = sumb + sr[j * ks2 + k].w * float(Padding2_2[sr[j * ks2 + k].cols]);

                        }
                    }
                    TempY[z * W + t] = sumb;
                }
            }
            clock_t plus_S = clock();
            for (int index = 0; index < ImgSize; index++) {
                output2[on].feature_map[index] = output2[on].feature_map[index] + TempY[index];
            }

            end = clock();
            printf("convolution 연산 소요 시간: %.3f\n", (float)(end - start) / CLOCKS_PER_SEC);

            free(Padding2_2);
            free(TempY);
        }
        clock_t relu2_s = clock();

        for (int index = 0; index < ImgSize; index++) {
            if (output2[on].feature_map[index] + bias2[on] >= 0) { output2[on].feature_map[index] = output2[on].feature_map[index] + bias2[on]; }
            else { output2[on].feature_map[index] = PReLU_weight2[on] * (output2[on].feature_map[index] + bias2[on]); }
        }
        //SaveFeatureMap_f(W, H, output2_2[on].feature_map, on, "mid_part/conv2/");

        printf("ReLU 및 bias 덧셈 소요 시간: %.3f\n", (float)(clock() - relu2_s) / CLOCKS_PER_SEC);

    }

    clock_t conv2e = clock();
    printf("MID PART - conv layer 2 소요 시간: %.3f\n", (float)(conv2e - conv2S) / CLOCKS_PER_SEC);


    // last part
    printf("\n===== LAST PART =====\n");
    int kn3 = 1;
    int ks3 = 3;
    int stride3 = 2;
    int pad3 = 1;
    int out_pad3 = 1;
    //int OH = H * scale;
    //int OW = W * scale;
    //int outsize = OH * OW;

    // padding 전용 변수
    int _pad3 = 1 * (ks3 - 1) - pad3;
    int _stride3 = stride3 - 1;
    int PW3 = 2 * W - 1 + (_pad3 * 2) + out_pad3;
    int PH3 = 2 * H - 1 + (_pad3 * 2) + out_pad3;

    // bias 불러오기
    float bias3[1];
    getbias_("./parameter_light/bias/last/bias0_0.txt", bias3);

    float* kernel3 = (float*)malloc(sizeof(float) * ks3 * ks3);
    _Output output3[1];
    output3[0].feature_map = (float*)malloc(sizeof(float) * W * H * scale * scale);
    memset(output3[0].feature_map, 0, sizeof(float) * W * H * scale * scale);


    for (int on = 0; on < kn3; on++) {
        for (int kn = 0; kn < kn2; kn++) {
            // 1) padding 추가

            float* TempY = (float*)malloc(sizeof(float) * outsize);
            memset(TempY, 0, sizeof(float) * outsize);
            float* Padding3 = (float*)malloc(sizeof(float) * PW3 * PH3);
            memset(Padding3, 0, sizeof(float) * PW3 * PH3);

            transpose_padding_initialize(Padding3, output2[kn].feature_map, W, H, PW3, PH3, _pad3, stride3);

            // 2) kernel 불러오기 + 상하좌우 반전 시키기
            char buffer[10];
            char kernelfile_dir[255] = "./parameter_light/kernel/last/kernel";
            strcat(kernelfile_dir, itoa(kn, buffer, 10));
            strcat(kernelfile_dir, "_");
            strcat(kernelfile_dir, "0");
            strcat(kernelfile_dir, ".txt");
            printf("kernel file: %s\n", kernelfile_dir);
            getbias_(kernelfile_dir, kernel3);

            
            // 상하좌우 반전
            float* kernelT = (float*)malloc(sizeof(float) * ks3 * ks3);
            memset(kernelT, 0, sizeof(float) * ks3 * ks3);
            for (int i = 0; i < ks3; i++) {
                for (int j = 0; j < ks3; j++) {
                    kernelT[i * ks3 + j] = kernel3[(ks3 - 1 - i) * ks3 + (ks3 - 1 - j)];
                }
            }
            

            // 3) convoluiton
            clock_t start = clock();

            int y3 = 0;
            int k = PW3 * ks3;
            for (int z = 0; z < OH; z++) { // z: Height, y축
                int h_start = z * PW3 * 1;
                int h_end = h_start + k;
                for (int t = 0; t < OW; t++) { // t: Width, x축
                    int w_start = t * 1;
                    int w_end = w_start + ks3;
                    float sumb = 0;
                    int kx = 0;
                    int ky = 0;
                    for (int i = h_start; i < h_end; i += PW3) {
                        for (int j = w_start; j < w_end; j++) {
                            sumb = sumb + kernel3[ky + kx] * float(Padding3[i + j]);
                            //printf("kernel index: %d, padding[%d]: %f\n", kx + ky, i + j, Padding3[i + j]);
                            kx++;

                        }
                        kx = 0;
                        ky += ks3;
                    }
                    TempY[y3 + t] = sumb;
                    //printf("%d\n", y3 + t);
                }
                y3 += OW;
            }

            for (int i = 0; i < outsize; i++) {
                output3[on].feature_map[i] = output3[on].feature_map[i] + TempY[i];
            }
            printf("convoltuion 연산 소요시간: %.3f\n", (float)(clock() - start) / CLOCKS_PER_SEC);

            free(Padding3);
            free(TempY);
        }
        clock_t bias_s = clock();
        for (int i = 0; i < W * H * scale * scale; i++) {
            output3[0].feature_map[i] = output3[0].feature_map[i] + bias3[0];
        }
        printf("bias 덧셈 소요 시간 : %.3f\n", (float)(clock() - bias_s) / CLOCKS_PER_SEC);
    }
    //SaveFeatureMap_f(W * scale, H * scale, output3[0].feature_map, 0, "last_part/");

    int OutSize = W * H * scale * scale;

    float* R = (float*)malloc(sizeof(float) * OutSize);
    float* G = (float*)malloc(sizeof(float) * OutSize);
    float* B = (float*)malloc(sizeof(float) * OutSize);

    float* Output = (float*)malloc(sizeof(float) * OutSize * 3);

    clock_t torgb_S = clock();
    for (int i = 0; i < OutSize; i++) {
        R[i] = 298.082 * 255.0 * output3[0].feature_map[i] / 256. + 408.583 * Cr[i] / 256. - 222.921;
        G[i] = 298.082 * 255.0 * output3[0].feature_map[i] / 256. - 100.291 * Cb[i] / 256. - 208.120 * Cr[i] / 256. + 135.576;
        B[i] = 298.082 * 255.0 * output3[0].feature_map[i] / 256. + 516.412 * Cb[i] / 256. - 276.836;
    }

    int t = 0;
    for (int i = 0; i < OutSize * 3; i += 3) {
        Output[i] = B[t];
        Output[i + 1] = G[t];
        Output[i + 2] = R[t];
        t++;
    }
    printf("YCbCr --> RGB 소요 시간: %.3f\n", (float)(clock() - torgb_S) / CLOCKS_PER_SEC);

    clock_t clip_S = clock();
    for (int i = 0; i < OutSize * 3; i++) {
        if (Output[i] > 255.0) {
            Output[i] = 255.0;
        }
        else if (Output[i] < 0.0) {
            Output[i] = 0.0;
        }
    }
    printf("0 미만, 255 초과 값 clip: %.3f\n", (float)(clock() - clip_S) / CLOCKS_PER_SEC);

    clock_t realign_S = clock();
    BYTE* SOutput = (BYTE*)malloc(sizeof(BYTE) * OutSize * 3);

    realign_F(SOutput, Output, OW, OH);
    printf("상하반전 소요 시간: %.3f\n", clock() - realign_S);

    SaveBMPFile(hf, hInfo, hRGB, SOutput, OW, OH, "./image.bmp", 1);
    printf("TOTAL TIME: %f\n", (float)(clock() - totalS) / CLOCKS_PER_SEC);
    free(G);
    free(B);
    free(SOutput);

    return 0;
}