#pragma warning(disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <Windows.h>
#include <math.h>
#include <io.h>
#include <time.h>
#include <direct.h>
#include <cstdio>
#include <conio.h>

typedef struct strs {
    char strs[255];
}; strs;

typedef struct _element {
    int cols;
    float w;
}; _element;

typedef struct _SR {
    _element element;
}; _SR;

typedef struct _SM {
    int n;
    _SR* SR;
}; _SM;

typedef struct _Output {
    float* feature_map;
}; _Output;

typedef struct _Output_B {
    byte* feature_map;
}; _Output_B;



typedef struct _bcelement {
    int x;
    int y;
    int w;
}; _bcelement;

typedef struct _bcSR {
    _bcelement element[16];
}; _bcSR;

typedef struct _bcSM {
    int n;
    _bcSR* SR;
}; _bcSM;


float u(float s, float a) {
    if ((fabs(s) >= 0) && (fabs(s) <= 1))
        return (a + 2) * (fabs(s) * fabs(s) * fabs(s)) - (a + 3) * (fabs(s) * fabs(s)) + 1;

    else if ((fabs(s) > 1) && (fabs(s) <= 2))
        return a * (fabs(s) * fabs(s) * fabs(s)) - (5 * a) * (fabs(s) * fabs(s)) + (8 * a) * fabs(s) - 4 * a;
    //return 0 - a * (fabs(s) * fabs(s) * fabs(s)) - (5 * a) * (fabs(s) * fabs(s)) + (8 * a) * fabs(s) - 4 * a;

    return 0;

}



// bicubic Sparse Matrix
void BC_MakeSparseMatrix(BYTE* Image, BYTE* Output, _bcSM SM, int W, int H, float scale)
{
    clock_t start;
    clock_t end;

    start = clock();

    float IH = H;
    float IW = W;
    float in_size = H * W;

    float OH = H * scale;
    float OW = W * scale;
    float out_size = OH * OW;
    printf("%f %f\n", OW, OH);
    float a = -0.5;
    float h = 1 / scale;

    float ux, ux1, ux2, ux3, uy, uy1, uy2, uy3;
    int cx, cy;

    float rpix = 0;

    float _x[4];
    float _y[4];

    for (int y = 0; y < OH; y++) {
        for (int x = 0; x < OW; x++) {
            int index = x + (OW * y);

            // 0-0. Output�� �;��ϴ� ���� �̹��� ��ǥ�� ������
            cx = x * h;
            cy = y * h;
            // printf("cx, cy: %f, %f\n", cx, cy);
            // ** ����ġ �ִ� �� �ʿ��� �κ� ** //
            // 0-1. �Է� �̹��� ��ġ�� ���� �ٸ� ����ġ �� �ο�
            if (fmodf(cx, 1) == 0) {
                ux = u(1, a);
                ux1 = u(0, a);
                ux2 = u(1, a);
                ux3 = u(2, a);
            }
            else {
                ux = u(1.5, a);
                ux1 = u(0.5, a);
                ux2 = u(0.5, a);
                ux3 = u(1.5, a);
            }

            if (fmodf(cy, 1) == 0) {
                uy = u(1, a);
                uy1 = u(0, a);
                uy2 = u(1, a);
                uy3 = u(2, a);
            }
            else {
                uy = u(1.5, a);
                uy1 = u(0.5, a);
                uy2 = u(0.5, a);
                uy3 = u(1.5, a);
            }

            // cx, cy : �����ؾ� �Ǵ� input �̹����� ��ġ
            // j,i : output �̹����� ��ġ
            // cx-1, cx-1 : ����ġ�� �������� ���� ��ġ(mat_w[0]�� ����)

            _x[0] = 1 + cx - floor(cx); // 1.5
            _x[1] = cx - floor(cx); // 0.5
            _x[2] = floor(cx) + 1 - cx; // 0.5
            _x[3] = floor(cx) + 2 - cx; // 1.5

            _y[0] = 1 + cy - floor(cy);
            _y[1] = cy - floor(cy);
            _y[2] = floor(cy) + 1 - cy;
            _y[3] = floor(cy) + 2 - cy;
            /*
            int px1 = cx - _x[0];
            int py1 = cy - _y[0];
            int w1 = ux3 * uy3 * 100;

            int px2 = cx - _x[1];
            int py2 = cy - _y[0];
            int w2 = ux3 * uy2 * 100;

            int px3 = cx + _x[2];
            int py3 = cy - _y[0];
            int w3 = ux3 * uy1 * 100;

            int px4 = cx + _x[3];
            int py4 = cy - _y[0];
            int w4 = ux3 * uy * 100;

            int px5 = cx - _x[0];
            int py5 = cy - _y[1];
            int w5 = ux2 * uy3 * 100;

            int px6 = cx - _x[1];
            int py6 = cy - _y[1];
            int w6 = ux2 * uy3 * 100;

            int px7 = cx + _x[2];
            int py7 = cy - _y[1];
            int w7 = ux2 * uy1 * 100;

            int px8 = cx + _x[3];
            int py8 = cy - _y[1];
            int w8 = ux2 * uy * 100;

            int px9 = cx - _x[0];
            int py9 = cy + _y[2];
            int w9 = ux1 * uy3 * 100;

            int px10 = cx - _x[1];
            int py10 = cy + _y[2];
            int w10 = ux1 * uy2 * 100;

            int px11 = cx + _x[2];
            int py11 = cy + _y[2];
            int w11 = ux1 * uy1 * 100;

            int px12 = cx + _x[3];
            int py12 = cy + _y[2];
            int w12 = ux1 * uy * 100;

            int px13 = cx - _x[0];
            int py13 = cy + _y[3];
            int w13 = ux * uy3 * 100;

            int px14 = cx - _x[1];
            int py14 = cy + _y[3];
            int w14 = ux * uy2 * 100;

            int px15 = cx + _x[2];
            int py15 = cy + _y[3];
            int w15 = ux * uy1 * 100;

            int px16 = cx + _x[3];
            int py16 = cy + _y[3];
            int w16 = ux * uy * 100;

            SM.SR[index].element[0].x = px1;
            SM.SR[index].element[0].y = py1;
            SM.SR[index].element[0].w = w1;

            SM.SR[index].element[1].x = px2;
            SM.SR[index].element[1].y = py2;
            SM.SR[index].element[1].w = w2;

            SM.SR[index].element[2].x = px3;
            SM.SR[index].element[2].y = py3;
            SM.SR[index].element[2].w = w3;

            SM.SR[index].element[3].x = px4;
            SM.SR[index].element[3].y = py4;
            SM.SR[index].element[3].w = w4;

            SM.SR[index].element[4].x = px5;
            SM.SR[index].element[4].y = py5;
            SM.SR[index].element[4].w = w5;

            SM.SR[index].element[5].x = px6;
            SM.SR[index].element[5].y = py6;
            SM.SR[index].element[5].w = w6;

            SM.SR[index].element[6].x = px7;
            SM.SR[index].element[6].y = py7;
            SM.SR[index].element[6].w = w7;

            SM.SR[index].element[7].x = px8;
            SM.SR[index].element[7].y = py8;
            SM.SR[index].element[7].w = w8;

            SM.SR[index].element[8].x = px9;
            SM.SR[index].element[8].y = py9;
            SM.SR[index].element[8].w = w9;

            SM.SR[index].element[9].x = px10;
            SM.SR[index].element[9].y = py10;
            SM.SR[index].element[9].w = w10;

            SM.SR[index].element[10].x = px11;
            SM.SR[index].element[10].y = py11;
            SM.SR[index].element[10].w = w11;

            SM.SR[index].element[11].x = px12;
            SM.SR[index].element[11].y = py12;
            SM.SR[index].element[11].w = w12;

            SM.SR[index].element[12].x = px13;
            SM.SR[index].element[12].y = py13;
            SM.SR[index].element[12].w = w13;

            SM.SR[index].element[13].x = px14;
            SM.SR[index].element[13].y = py14;
            SM.SR[index].element[13].w = w14;

            SM.SR[index].element[14].x = px15;
            SM.SR[index].element[14].y = py15;
            SM.SR[index].element[14].w = w15;

            SM.SR[index].element[15].x = px16;
            SM.SR[index].element[15].y = py16;
            SM.SR[index].element[15].w = w16;
            */

            SM.SR[index].element[0].x = cx - _x[0];
            SM.SR[index].element[0].y = cy - _y[0];
            SM.SR[index].element[0].w = ux3 * uy3 * 100;

            SM.SR[index].element[1].x = cx - _x[1];
            SM.SR[index].element[1].y = cy - _y[0];
            SM.SR[index].element[1].w = ux3 * uy2 * 100;

            SM.SR[index].element[2].x = cx + _x[2];
            SM.SR[index].element[2].y = cy - _y[0];
            SM.SR[index].element[2].w = ux3 * uy1 * 100;

            SM.SR[index].element[3].x = cx + _x[3];
            SM.SR[index].element[3].y = cy - _y[0];
            SM.SR[index].element[3].w = ux3 * uy * 100;



            SM.SR[index].element[4].x = cx - _x[0];
            SM.SR[index].element[4].y = cy - _y[1];
            SM.SR[index].element[4].w = ux2 * uy3 * 100;

            SM.SR[index].element[5].x = cx - _x[1];
            SM.SR[index].element[5].y = cy - _y[1];
            SM.SR[index].element[5].w = ux2 * uy2 * 100;

            SM.SR[index].element[6].x = cx + _x[2];
            SM.SR[index].element[6].y = cy - _y[1];
            SM.SR[index].element[6].w = ux2 * uy1 * 100;

            SM.SR[index].element[7].x = cx + _x[3];
            SM.SR[index].element[7].y = cy - _y[1];
            SM.SR[index].element[7].w = ux2 * uy * 100;



            SM.SR[index].element[8].x = cx - _x[0];
            SM.SR[index].element[8].y = cy + _y[2];
            SM.SR[index].element[8].w = ux1 * uy3 * 100;

            SM.SR[index].element[9].x = cx - _x[1];
            SM.SR[index].element[9].y = cy + _y[2];
            SM.SR[index].element[9].w = ux1 * uy2 * 100;

            SM.SR[index].element[10].x = cx + _x[2];
            SM.SR[index].element[10].y = cy + _y[2];
            SM.SR[index].element[10].w = ux1 * uy1 * 100;

            SM.SR[index].element[11].x = cx + _x[2];
            SM.SR[index].element[11].y = cy + _y[2];
            SM.SR[index].element[11].w = ux1 * uy * 100;



            SM.SR[index].element[12].x = cx - _x[0];
            SM.SR[index].element[12].y = cy + _y[3];
            SM.SR[index].element[12].w = ux * uy3 * 100;

            SM.SR[index].element[13].x = cx - _x[1];
            SM.SR[index].element[13].y = cy + _y[3];
            SM.SR[index].element[13].w = ux * uy2 * 100;

            SM.SR[index].element[14].x = cx + _x[2];
            SM.SR[index].element[14].y = cy + _y[2];
            SM.SR[index].element[14].w = ux * uy1 * 100;

            SM.SR[index].element[15].x = cx + _x[3];
            SM.SR[index].element[15].y = cy + _y[3];
            SM.SR[index].element[15].w = ux * uy * 100;

            if (cx - _x[0] < 0) {
                SM.SR[index].element[0].x = 0;
                SM.SR[index].element[0].y = 0;
                SM.SR[index].element[0].w = 0;

                SM.SR[index].element[4].x = 0;
                SM.SR[index].element[4].y = 0;
                SM.SR[index].element[4].w = 0;

                SM.SR[index].element[8].x = 0;
                SM.SR[index].element[8].y = 0;
                SM.SR[index].element[8].w = 0;

                SM.SR[index].element[12].x = 0;
                SM.SR[index].element[12].y = 0;
                SM.SR[index].element[12].w = 0;
            }

            if (cy - _y[0] < 0) {
                SM.SR[index].element[0].x = 0;
                SM.SR[index].element[0].y = 0;
                SM.SR[index].element[0].w = 0;

                SM.SR[index].element[1].x = 0;
                SM.SR[index].element[1].y = 0;
                SM.SR[index].element[1].w = 0;

                SM.SR[index].element[2].x = 0;
                SM.SR[index].element[2].y = 0;
                SM.SR[index].element[2].w = 0;

                SM.SR[index].element[3].x = 0;
                SM.SR[index].element[3].y = 0;
                SM.SR[index].element[3].w = 0;
            }

            if (cy + _y[2] >= IH) {
                SM.SR[index].element[8].x = 0;
                SM.SR[index].element[8].y = 0;
                SM.SR[index].element[8].w = 0;

                SM.SR[index].element[9].x = 0;
                SM.SR[index].element[9].y = 0;
                SM.SR[index].element[9].w = 0;

                SM.SR[index].element[10].x = 0;
                SM.SR[index].element[10].y = 0;
                SM.SR[index].element[10].w = 0;

                SM.SR[index].element[11].x = 0;
                SM.SR[index].element[11].y = 0;
                SM.SR[index].element[11].w = 0;

            }

            if (cy + _y[3] >= IH) {

                SM.SR[index].element[12].x = 0;
                SM.SR[index].element[12].y = 0;
                SM.SR[index].element[12].w = 0;

                SM.SR[index].element[13].x = 0;
                SM.SR[index].element[13].y = 0;
                SM.SR[index].element[13].w = 0;

                SM.SR[index].element[14].x = 0;
                SM.SR[index].element[14].y = 0;
                SM.SR[index].element[14].w = 0;

                SM.SR[index].element[15].x = 0;
                SM.SR[index].element[15].y = 0;
                SM.SR[index].element[15].w = 0;
            }
        }


    }

    end = clock();
    printf("BICUIBC ������ ���� �Ϸ�: %.3f\n", (float)(end - start) / CLOCKS_PER_SEC);

}

void TESTBC(BYTE* Image, BYTE* Output, int W, int H, int size, float scale, _bcSM SM)
{
    /*
    Bicubic matrix �� ���� �� ������ ���� �Ҵ��� �� Output ���� �����ϴ� �� �� 0.1~ 0.2 �� ���� �� �ҿ��
    ���� ���� �ٷ� �θ������� ������ ���� �� Output �� �����ϱ�!
    */
    clock_t start;
    clock_t end;


    start = clock();

    int IW = W;
    int IH = H;

    int OW = scale * W;
    int OH = scale * H;

    printf("%d\n", OW);

    for (int y = 0; y < OH; y++) {
        for (int x = 0; x < OW; x++) {

            int sm_index = x + (y * OW);
            int out_index = x * 3 + (y * OW * 3);

            // ���� ���� �������� �ٷ� �����ڱ�
            // w: float --> int
            // w�� ���ϱ� 100�� �ؼ� integer �� �ٲٰ�
            // �� ���ϰ� output image�� ���� �� �����ֱ� /100�� �ϱ�(�Ҽ����� �����ϱ�)
            // 100, 1000, 10000 ��������

            /*
            printf("%d\n", Image[(SM.SR[sm_index].element[0].x * 3) + (SM.SR[sm_index].element[0].y * W * 3)]);
            printf("%d\n", Image[(SM.SR[sm_index].element[1].x * 3) + (SM.SR[sm_index].element[1].y * W * 3)]);
            printf("%d\n", Image[(SM.SR[sm_index].element[2].x * 3) + (SM.SR[sm_index].element[2].y * W * 3)]);
            printf("%d\n", Image[(SM.SR[sm_index].element[3].x * 3) + (SM.SR[sm_index].element[3].y * W * 3)]);
            printf("%d\n", Image[(SM.SR[sm_index].element[4].x * 3) + (SM.SR[sm_index].element[4].y * W * 3)]);
            printf("%d\n", Image[(SM.SR[sm_index].element[5].x * 3) + (SM.SR[sm_index].element[5].y * W * 3)]);
            printf("%d\n", Image[(SM.SR[sm_index].element[6].x * 3) + (SM.SR[sm_index].element[6].y * W * 3)]);
            printf("%d\n", Image[(SM.SR[sm_index].element[7].x * 3) + (SM.SR[sm_index].element[7].y * W * 3)]);
            printf("%d\n", Image[(SM.SR[sm_index].element[8].x * 3) + (SM.SR[sm_index].element[8].y * W * 3)]);
            printf("%d\n", Image[(SM.SR[sm_index].element[9].x * 3) + (SM.SR[sm_index].element[9].y * W * 3)]);
            printf("%d\n", Image[(SM.SR[sm_index].element[10].x * 3) + (SM.SR[sm_index].element[10].y * W * 3)]);
            printf("%d\n", Image[(SM.SR[sm_index].element[11].x * 3) + (SM.SR[sm_index].element[11].y * W * 3)]);
            printf("%d\n", Image[(SM.SR[sm_index].element[12].x * 3) + (SM.SR[sm_index].element[12].y * W * 3)]);
            printf("%d\n", Image[(SM.SR[sm_index].element[13].x * 3) + (SM.SR[sm_index].element[13].y * W * 3)]);
            printf("%d\n", Image[(SM.SR[sm_index].element[14].x * 3) + (SM.SR[sm_index].element[14].y * W * 3)]);
            printf("%d\n", Image[(SM.SR[sm_index].element[15].x * 3) + (SM.SR[sm_index].element[15].y * W * 3)]);
            */

            int px1 = SM.SR[sm_index].element[0].x;
            int py1 = SM.SR[sm_index].element[0].y;
            int w1 = SM.SR[sm_index].element[0].w;

            int px2 = SM.SR[sm_index].element[1].x;
            int py2 = SM.SR[sm_index].element[1].y;
            int w2 = SM.SR[sm_index].element[1].w;

            int px3 = SM.SR[sm_index].element[2].x;
            int py3 = SM.SR[sm_index].element[2].y;
            int w3 = SM.SR[sm_index].element[2].w;

            int px4 = SM.SR[sm_index].element[3].x;
            int py4 = SM.SR[sm_index].element[3].y;
            int w4 = SM.SR[sm_index].element[3].w;

            int px5 = SM.SR[sm_index].element[4].x;
            int py5 = SM.SR[sm_index].element[4].y;
            int w5 = SM.SR[sm_index].element[4].w;

            int px6 = SM.SR[sm_index].element[5].x;
            int py6 = SM.SR[sm_index].element[5].y;
            int w6 = SM.SR[sm_index].element[5].w;

            int px7 = SM.SR[sm_index].element[6].x;
            int py7 = SM.SR[sm_index].element[6].y;
            int w7 = SM.SR[sm_index].element[6].w;

            int px8 = SM.SR[sm_index].element[7].x;
            int py8 = SM.SR[sm_index].element[7].y;
            int w8 = SM.SR[sm_index].element[7].w;

            int px9 = SM.SR[sm_index].element[8].x;
            int py9 = SM.SR[sm_index].element[8].y;
            int w9 = SM.SR[sm_index].element[8].w;

            int px10 = SM.SR[sm_index].element[9].x;
            int py10 = SM.SR[sm_index].element[9].y;
            int w10 = SM.SR[sm_index].element[9].w;

            int px11 = SM.SR[sm_index].element[10].x;
            int py11 = SM.SR[sm_index].element[10].y;
            int w11 = SM.SR[sm_index].element[10].w;

            int px12 = SM.SR[sm_index].element[11].x;
            int py12 = SM.SR[sm_index].element[11].y;
            int w12 = SM.SR[sm_index].element[11].w;

            int px13 = SM.SR[sm_index].element[12].x;
            int py13 = SM.SR[sm_index].element[12].y;
            int w13 = SM.SR[sm_index].element[12].w;

            int px14 = SM.SR[sm_index].element[13].x;
            int py14 = SM.SR[sm_index].element[13].y;
            int w14 = SM.SR[sm_index].element[13].w;

            int px15 = SM.SR[sm_index].element[14].x;
            int py15 = SM.SR[sm_index].element[14].y;
            int w15 = SM.SR[sm_index].element[14].w;

            int px16 = SM.SR[sm_index].element[15].x;
            int py16 = SM.SR[sm_index].element[15].y;
            int w16 = SM.SR[sm_index].element[15].w;

            int b_v = (Image[(px1 * 3) + (py1 * W * 3)] * w1
                + Image[(px2 * 3) + (py2 * W * 3)] * w2
                + Image[(px3 * 3) + (py3 * W * 3)] * w3
                + Image[(px4 * 3) + (py4 * W * 3)] * w4
                + Image[(px5 * 3) + (py5 * W * 3)] * w5
                + Image[(px6 * 3) + (py6 * W * 3)] * w6
                + Image[(px7 * 3) + (py7 * W * 3)] * w7
                + Image[(px8 * 3) + (py8 * W * 3)] * w8
                + Image[(px9 * 3) + (py9 * W * 3)] * w9
                + Image[(px10 * 3) + (py10 * W * 3)] * w10
                + Image[(px11 * 3) + (py11 * W * 3)] * w11
                + Image[(px12 * 3) + (py12 * W * 3)] * w12
                + Image[(px13 * 3) + (py13 * W * 3)] * w13
                + Image[(px14 * 3) + (py14 * W * 3)] * w14
                + Image[(px15 * 3) + (py15 * W * 3)] * w15
                + Image[(px16 * 3) + (py16 * W * 3)] * w16) / 100;

            int g_v = (Image[(px1 * 3) + (py1 * W * 3) + 1] * w1
                + Image[(px2 * 3) + (py2 * W * 3) + 1] * w2
                + Image[(px3 * 3) + (py3 * W * 3) + 1] * w3
                + Image[(px4 * 3) + (py4 * W * 3) + 1] * w4
                + Image[(px5 * 3) + (py5 * W * 3) + 1] * w5
                + Image[(px6 * 3) + (py6 * W * 3) + 1] * w6
                + Image[(px7 * 3) + (py7 * W * 3) + 1] * w7
                + Image[(px8 * 3) + (py8 * W * 3) + 1] * w8
                + Image[(px9 * 3) + (py9 * W * 3) + 1] * w9
                + Image[(px10 * 3) + (py10 * W * 3) + 1] * w10
                + Image[(px11 * 3) + (py11 * W * 3) + 1] * w11
                + Image[(px12 * 3) + (py12 * W * 3) + 1] * w12
                + Image[(px13 * 3) + (py13 * W * 3) + 1] * w13
                + Image[(px14 * 3) + (py14 * W * 3) + 1] * w14
                + Image[(px15 * 3) + (py15 * W * 3) + 1] * w15
                + Image[(px16 * 3) + (py16 * W * 3) + 1] * w16) / 100;

            int r_v = (Image[(px1 * 3) + (py1 * W * 3) + 2] * w1
                + Image[(px2 * 3) + (py2 * W * 3) + 2] * w2
                + Image[(px3 * 3) + (py3 * W * 3) + 2] * w3
                + Image[(px4 * 3) + (py4 * W * 3) + 2] * w4
                + Image[(px5 * 3) + (py5 * W * 3) + 2] * w5
                + Image[(px6 * 3) + (py6 * W * 3) + 2] * w6
                + Image[(px7 * 3) + (py7 * W * 3) + 2] * w7
                + Image[(px8 * 3) + (py8 * W * 3) + 2] * w8
                + Image[(px9 * 3) + (py9 * W * 3) + 2] * w9
                + Image[(px10 * 3) + (py10 * W * 3) + 2] * w10
                + Image[(px11 * 3) + (py11 * W * 3) + 2] * w11
                + Image[(px12 * 3) + (py12 * W * 3) + 2] * w12
                + Image[(px13 * 3) + (py13 * W * 3) + 2] * w13
                + Image[(px14 * 3) + (py14 * W * 3) + 2] * w14
                + Image[(px15 * 3) + (py15 * W * 3) + 2] * w15
                + Image[(px16 * 3) + (py16 * W * 3) + 2] * w16) / 100;

            /*
            printf("%d, %d\n", px1, py1);
            printf("%d, %d\n", px2, py2);
            printf("%d, %d\n", px3, py3);
            printf("%d, %d\n", px4, py4);
            printf("%d, %d\n", px5, py5);
            printf("%d, %d\n", px6, py6);
            printf("%d, %d\n", px7, py7);
            printf("%d, %d\n", px8, py8);
            printf("%d, %d\n", px9, py9);
            printf("%d, %d\n", px10, py10);
            printf("%d, %d\n", px11, py11);
            printf("%d, %d\n", px12, py12);
            printf("%d, %d\n", px13, py13);
            printf("%d, %d\n", px14, py14);
            printf("%d, %d\n", px15, py15);
            printf("%d, %d\n", px16, py16);
            */
            /*
            int b_v = (Image[(SM.SR[sm_index].element[0].x * 3) + (SM.SR[sm_index].element[0].y * W * 3)] * SM.SR[sm_index].element[0].w
                + Image[(SM.SR[sm_index].element[1].x * 3) + (SM.SR[sm_index].element[1].y * W * 3)] * SM.SR[sm_index].element[1].w
                + Image[(SM.SR[sm_index].element[2].x * 3) + (SM.SR[sm_index].element[2].y * W * 3)] * SM.SR[sm_index].element[2].w
                + Image[(SM.SR[sm_index].element[3].x * 3) + (SM.SR[sm_index].element[3].y * W * 3)] * SM.SR[sm_index].element[3].w
                + Image[(SM.SR[sm_index].element[4].x * 3) + (SM.SR[sm_index].element[4].y * W * 3)] * SM.SR[sm_index].element[4].w
                + Image[(SM.SR[sm_index].element[5].x * 3) + (SM.SR[sm_index].element[5].y * W * 3)] * SM.SR[sm_index].element[5].w
                + Image[(SM.SR[sm_index].element[6].x * 3) + (SM.SR[sm_index].element[6].y * W * 3)] * SM.SR[sm_index].element[6].w
                + Image[(SM.SR[sm_index].element[7].x * 3) + (SM.SR[sm_index].element[7].y * W * 3)] * SM.SR[sm_index].element[7].w
                + Image[(SM.SR[sm_index].element[8].x * 3) + (SM.SR[sm_index].element[8].y * W * 3)] * SM.SR[sm_index].element[8].w
                + Image[(SM.SR[sm_index].element[9].x * 3) + (SM.SR[sm_index].element[9].y * W * 3)] * SM.SR[sm_index].element[9].w
                + Image[(SM.SR[sm_index].element[10].x * 3) + (SM.SR[sm_index].element[10].y * W * 3)] * SM.SR[sm_index].element[10].w
                + Image[(SM.SR[sm_index].element[11].x * 3) + (SM.SR[sm_index].element[11].y * W * 3)] * SM.SR[sm_index].element[11].w
                + Image[(SM.SR[sm_index].element[12].x * 3) + (SM.SR[sm_index].element[12].y * W * 3)] * SM.SR[sm_index].element[12].w
                + Image[(SM.SR[sm_index].element[13].x * 3) + (SM.SR[sm_index].element[13].y * W * 3)] * SM.SR[sm_index].element[13].w
                + Image[(SM.SR[sm_index].element[14].x * 3) + (SM.SR[sm_index].element[14].y * W * 3)] * SM.SR[sm_index].element[14].w
                + Image[(SM.SR[sm_index].element[15].x * 3) + (SM.SR[sm_index].element[15].y * W * 3)] * SM.SR[sm_index].element[15].w);

            int g_v = (Image[(SM.SR[sm_index].element[0].x * 3) + (SM.SR[sm_index].element[0].y * W * 3) + 1] * SM.SR[sm_index].element[0].w
                + Image[(SM.SR[sm_index].element[1].x * 3) + (SM.SR[sm_index].element[1].y * W * 3) + 1] * SM.SR[sm_index].element[1].w
                + Image[(SM.SR[sm_index].element[2].x * 3) + (SM.SR[sm_index].element[2].y * W * 3) + 1] * SM.SR[sm_index].element[2].w
                + Image[(SM.SR[sm_index].element[3].x * 3) + (SM.SR[sm_index].element[3].y * W * 3) + 1] * SM.SR[sm_index].element[3].w
                + Image[(SM.SR[sm_index].element[4].x * 3) + (SM.SR[sm_index].element[4].y * W * 3) + 1] * SM.SR[sm_index].element[4].w
                + Image[(SM.SR[sm_index].element[5].x * 3) + (SM.SR[sm_index].element[5].y * W * 3) + 1] * SM.SR[sm_index].element[5].w
                + Image[(SM.SR[sm_index].element[6].x * 3) + (SM.SR[sm_index].element[6].y * W * 3) + 1] * SM.SR[sm_index].element[6].w
                + Image[(SM.SR[sm_index].element[7].x * 3) + (SM.SR[sm_index].element[7].y * W * 3) + 1] * SM.SR[sm_index].element[7].w
                + Image[(SM.SR[sm_index].element[8].x * 3) + (SM.SR[sm_index].element[8].y * W * 3) + 1] * SM.SR[sm_index].element[8].w
                + Image[(SM.SR[sm_index].element[9].x * 3) + (SM.SR[sm_index].element[9].y * W * 3) + 1] * SM.SR[sm_index].element[9].w
                + Image[(SM.SR[sm_index].element[10].x * 3) + (SM.SR[sm_index].element[10].y * W * 3) + 1] * SM.SR[sm_index].element[10].w
                + Image[(SM.SR[sm_index].element[11].x * 3) + (SM.SR[sm_index].element[11].y * W * 3) + 1] * SM.SR[sm_index].element[11].w
                + Image[(SM.SR[sm_index].element[12].x * 3) + (SM.SR[sm_index].element[12].y * W * 3) + 1] * SM.SR[sm_index].element[12].w
                + Image[(SM.SR[sm_index].element[13].x * 3) + (SM.SR[sm_index].element[13].y * W * 3) + 1] * SM.SR[sm_index].element[13].w
                + Image[(SM.SR[sm_index].element[14].x * 3) + (SM.SR[sm_index].element[14].y * W * 3) + 1] * SM.SR[sm_index].element[14].w
                + Image[(SM.SR[sm_index].element[15].x * 3) + (SM.SR[sm_index].element[15].y * W * 3) + 1] * SM.SR[sm_index].element[15].w);

            int r_v = (Image[(SM.SR[sm_index].element[0].x * 3) + (SM.SR[sm_index].element[0].y * W * 3) + 2] * SM.SR[sm_index].element[0].w
                + Image[(SM.SR[sm_index].element[1].x * 3) + (SM.SR[sm_index].element[1].y * W * 3) + 2] * SM.SR[sm_index].element[1].w
                + Image[(SM.SR[sm_index].element[2].x * 3) + (SM.SR[sm_index].element[2].y * W * 3) + 2] * SM.SR[sm_index].element[2].w
                + Image[(SM.SR[sm_index].element[3].x * 3) + (SM.SR[sm_index].element[3].y * W * 3) + 2] * SM.SR[sm_index].element[3].w
                + Image[(SM.SR[sm_index].element[4].x * 3) + (SM.SR[sm_index].element[4].y * W * 3) + 2] * SM.SR[sm_index].element[4].w
                + Image[(SM.SR[sm_index].element[5].x * 3) + (SM.SR[sm_index].element[5].y * W * 3) + 2] * SM.SR[sm_index].element[5].w
                + Image[(SM.SR[sm_index].element[6].x * 3) + (SM.SR[sm_index].element[6].y * W * 3) + 2] * SM.SR[sm_index].element[6].w
                + Image[(SM.SR[sm_index].element[7].x * 3) + (SM.SR[sm_index].element[7].y * W * 3) + 2] * SM.SR[sm_index].element[7].w
                + Image[(SM.SR[sm_index].element[8].x * 3) + (SM.SR[sm_index].element[8].y * W * 3) + 2] * SM.SR[sm_index].element[8].w
                + Image[(SM.SR[sm_index].element[9].x * 3) + (SM.SR[sm_index].element[9].y * W * 3) + 2] * SM.SR[sm_index].element[9].w
                + Image[(SM.SR[sm_index].element[10].x * 3) + (SM.SR[sm_index].element[10].y * W * 3) + 2] * SM.SR[sm_index].element[10].w
                + Image[(SM.SR[sm_index].element[11].x * 3) + (SM.SR[sm_index].element[11].y * W * 3) + 2] * SM.SR[sm_index].element[11].w
                + Image[(SM.SR[sm_index].element[12].x * 3) + (SM.SR[sm_index].element[12].y * W * 3) + 2] * SM.SR[sm_index].element[12].w
                + Image[(SM.SR[sm_index].element[13].x * 3) + (SM.SR[sm_index].element[13].y * W * 3) + 2] * SM.SR[sm_index].element[13].w
                + Image[(SM.SR[sm_index].element[14].x * 3) + (SM.SR[sm_index].element[14].y * W * 3) + 2] * SM.SR[sm_index].element[14].w
                + Image[(SM.SR[sm_index].element[15].x * 3) + (SM.SR[sm_index].element[15].y * W * 3) + 2] * SM.SR[sm_index].element[15].w);
            */

            if (b_v < 0) {
                b_v = 0;
            }
            if (b_v > 255) {
                b_v = 255;
            }
            if (g_v < 0) {
                g_v = 0;
            }
            if (g_v > 255) {
                g_v = 255;
            }
            if (r_v < 0) {
                r_v = 0;
            }
            if (r_v > 255) {
                r_v = 255;
            }


            Output[out_index] = b_v;
            Output[out_index + 1] = g_v;
            Output[out_index + 2] = r_v;

        }

    }

    end = clock();
    printf("BICUBIC ����: %.3f\n", (float)(end - start) / CLOCKS_PER_SEC);
}


void padding_initialize(float* Padding, float* Image, int PW, int PH, int pad) {

    clock_t padding_S = clock();
    //bitmap ������ ���� �������� ����Ǳ� ������ padding�� ���ÿ� ���� �����ǵ��� ���� ����
    int W = PW - (pad * 2);
    int H = PH - (pad * 2);
    // Zero Padding
    /*
    for (int i = 0; i < PH; i++) {
        for (int j = 0; j < PW; j++) {
            Padding[i * PW + j] = 0;
        }
    }
    */
    // Origin Pixel Value 
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            Padding[(i + pad) * PW + (j + pad)] = Image[i * W + j]; // y
            
        }
    }

    clock_t padding_E = clock();
    printf("�е� �ҿ� �ð�: %.3f\n", (float)(padding_E - padding_S) / CLOCKS_PER_SEC);
}

void transpose_padding_initialize(float* Padding, float* Image, int W, int H, int PW, int PH, int pad, int stride) {

    clock_t padding_S = clock();
   

    // Origin Pixel Value 
    int px = pad;
    int py = pad;
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            Padding[py * PW + px] = Image[i * W + j];
            px = px + stride;
        }
        px = pad;
        py = py + stride;
    }

    clock_t padding_E = clock();
    printf("�е� �ҿ� �ð�: %.3f\n", (float)(padding_E - padding_S) / CLOCKS_PER_SEC);
}

void padding_initialize_b(float* Padding, byte* Image, int PW, int PH, int pad) {

    clock_t padding_S = clock();
    //bitmap ������ ���� �������� ����Ǳ� ������ padding�� ���ÿ� ���� �����ǵ��� ���� ����
    int W = PW - (pad * 2);
    int H = PH - (pad * 2);
    // Zero Padding
    for (int i = 0; i < PH; i++) {
        for (int j = 0; j < PW; j++) {
            Padding[i * PW + j] = 0;
        }
    }

    // Origin Pixel Value 
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            Padding[(i + pad) * PW + (j + pad)] = Image[i * W + j] / 255.0; // y
        }
    }

    clock_t padding_E = clock();
    printf("�е� �ҿ� �ð�: %.3f\n", (float)(padding_E - padding_S) / CLOCKS_PER_SEC);
}

void realign(BYTE* Y, BYTE* Image, int W, int H) {
    clock_t start = clock();
    // Origin Pixel Value 
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            //Row ���� ���ÿ�
            Y[i * W * 3 + j * 3] = Image[(H - i - 1) * W * 3 + j * 3]; // Blue
            Y[i * W * 3 + j * 3 + 1] = Image[(H - i - 1) * W * 3 + j * 3 + 1]; // Green
            Y[i * W * 3 + j * 3 + 2] = Image[(H - i - 1) * W * 3 + j * 3 + 2]; // Red
        }
    }
    printf("�̹��� ���Ϲ��� �ҿ� �ð�: %.3f\n", (float)(clock() - start) / CLOCKS_PER_SEC);
}

void realign_F(BYTE* Y, float* Image, int W, int H) {
    clock_t start = clock();
    // Origin Pixel Value 
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            //Row ���� ���ÿ�
            Y[i * W * 3 + j * 3] = Image[(H - i - 1) * W * 3 + j * 3]; // Blue
            Y[i * W * 3 + j * 3 + 1] = Image[(H - i - 1) * W * 3 + j * 3 + 1]; // Green
            Y[i * W * 3 + j * 3 + 2] = Image[(H - i - 1) * W * 3 + j * 3 + 2]; // Red
        }
    }
    printf("�̹��� ���Ϲ��� �ҿ� �ð�: %.3f\n", (float)(clock() - start) / CLOCKS_PER_SEC);
}

void SaveBMPFile(BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo,
    RGBQUAD* hRGB, BYTE* Output, int W, int H, const char* FileName, int scale)
{
    clock_t start = clock();
    hInfo.biWidth = W;
    hInfo.biHeight = H;
    //printf("%d, %d\n", W, H);
    FILE* fp = fopen(FileName, "wb");
    if (hInfo.biBitCount == 24) {
        fwrite(&hf, sizeof(BYTE), sizeof(BITMAPFILEHEADER), fp);
        fwrite(&hInfo, sizeof(BYTE), sizeof(BITMAPINFOHEADER), fp);
        fwrite(Output, sizeof(BYTE), (W) * (H) * 3, fp);
    }
    else if (hInfo.biBitCount == 8) {
        fwrite(&hf, sizeof(BYTE), sizeof(BITMAPFILEHEADER), fp);
        fwrite(&hInfo, sizeof(BYTE), sizeof(BITMAPINFOHEADER), fp);
        fwrite(hRGB, sizeof(RGBQUAD), 256, fp);
        fwrite(Output, sizeof(BYTE), W * H, fp);
    }
    fclose(fp);
    printf("�̹��� ���� �ҿ� �ð�: %.3f\n", (float)(clock() - start) / CLOCKS_PER_SEC);
}

// �ð� �� �ɸ��� �ڵ�
/*
void SaveFeatureMap(int W, int H, byte* B, int k_n, const char* FileName) { //k_n: kernel list number(kernel ����Ű�� ����)

    clock_t savefm_S = clock(); //featuremap ���� �ð� ����
    FILE* file;
    char kernel_num[100];
    char result_feature[255] = "D:/������/SR(202109-202212)/SRCNN/feature/";
    strcat(result_feature, FileName);

    printf("%s\n", result_feature);
    int fResult = mkdir(result_feature);
    printf("%d\n", fResult);
    char feature[300] = "feature0_";
    char number[20] = "";
    strcat(result_feature, strcat(strcat(feature, itoa(k_n, number, 10)), ".txt"));
    printf("%s\n", result_feature);

    file = fopen(result_feature, "wb");
    int index = 0;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            char temp[300];
            index = (x + y * W);
            float k = B[index];
            sprintf(temp, "%f", k);
            //printf("%f\n", k);
            fputs(strcat(temp, " "), file);
        }
        fputs("\n", file);
    }

    fclose(file);

    clock_t savefm_E = clock(); //featuremap ���� �ð� ����
    printf("Featuremap ���� �ð�: %.3f\n", (float)(savefm_E - savefm_S) / CLOCKS_PER_SEC);
}
*/
/*
void SaveFeatureMap_f(int W, int H, float* B, int k_n, const char* FileName) { //k_n: kernel list number(kernel ����Ű�� ����)

    clock_t savefm_S = clock(); //featuremap ���� �ð� ����
    FILE* file;
    char kernel_num[100];
    //char result_feature[255] = "C:/Users/siewe/OneDrive/���� ȭ��/FSRCNN/feature/";
    char result_feature[255] = "D:/������/SR(202109-202212)/FSRCNN/feature/";
    strcat(result_feature, FileName);

    printf("%s\n", result_feature);
    int fResult = mkdir(result_feature);
    printf("%d\n", fResult);
    char feature[300] = "feature0_";
    char number[20] = "";
    strcat(result_feature, strcat(strcat(feature, itoa(k_n, number, 10)), ".txt"));
    printf("%s\n", result_feature);

    file = fopen(result_feature, "wb");
    int index = 0;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            char temp[300];
            index = (x + y * W);
            float k = B[index];
            sprintf(temp, "%f", k);
            //printf("%f\n", k);
            fputs(strcat(temp, " "), file);
        }
        fputs("\n", file);
    }

    fclose(file);

    clock_t savefm_E = clock(); //featuremap ���� �ð� ����
    printf("Featuremap ���� �ð�: %.3f\n", (float)(savefm_E - savefm_S) / CLOCKS_PER_SEC);
}
*/

// 9x9 kernel
int getMatrix_9(const char* filename, float(*matrix)[9])
{

    clock_t getkernel_S = clock();
    FILE* stream;

    int chk = 0;
    int i = 0, j = 0;
    int mj = 0;
    stream = fopen(filename, "r");
    if (stream == NULL)
        return 1;

    while (chk != EOF)
    {
        j = 0;
        chk = 0;
        while (chk != '\n' && chk != EOF)
        {
            fscanf(stream, "%f", &matrix[i][j]);
            chk = fgetc(stream);
            fseek(stream, -1, SEEK_CUR);
            j++;
        }
        i++;
        if (mj < j)
            mj = j;
    }

    fclose(stream);
    clock_t getkernel_E = clock();
    printf("Ŀ�� �ҷ����� �ҿ� �ð�(9x9): %.3f\n", getkernel_E - getkernel_S);

    return 0;

}

// 5x5 kernel
int getMatrix_5(const char* filename, float(*matrix)[5])
{
    clock_t start = clock();
    FILE* stream;

    int chk = 0;
    int i = 0, j = 0;
    int mj = 0;
    stream = fopen(filename, "r");
    if (stream == NULL)
        return 1;

    while (chk != EOF)
    {
        j = 0;
        chk = 0;
        while (chk != '\n' && chk != EOF)
        {
            fscanf(stream, "%f", &matrix[i][j]);
            printf("matrix[%d][%d]: %f\n", i, j, matrix[i][j]);
            chk = fgetc(stream);
            fseek(stream, -1, SEEK_CUR);
            j++;
        }
        i++;
        if (mj < j)
            mj = j;
    }

    fclose(stream);
    clock_t end = clock();
    printf("Ŀ�� �ҷ����� �ҿ� �ð�(5x5): %.3f\n", (float)(end - start) / CLOCKS_PER_SEC);
    return 0;
}

// 30�� ��ƸԴ� �ڵ�
int getFeatureMap(int W, int H, const char* filename, float* matrix)
{
    clock_t start = clock();
    FILE* stream;

    int chk = 0;
    int i = 0, j = 0;
    int mj = 0;
    stream = fopen(filename, "r");
    if (stream == NULL)
        return 1;

    while (chk != EOF)
    {
        j = 0;
        chk = 0;
        while (chk != '\n' && chk != EOF)
        {
            fscanf(stream, "%f", &matrix[i * W + j]);
            chk = fgetc(stream);
            fseek(stream, -1, SEEK_CUR);
            j++;
        }
        i++;
        if (mj < j)
            mj = j;
    }

    fclose(stream);
    clock_t end = clock();
    printf("feature map �ҷ����� �ҿ� �ð�: %.3f\n", (float)(end - start) / CLOCKS_PER_SEC);
    return 0;
}

int getBias(const char* filename, float* bias) {

    clock_t getbias_S = clock();

    FILE* stream;

    int chk = 0;
    int i = 0, j = 0;
    int mj = 0;
    stream = fopen(filename, "r");
    if (stream == NULL) {
        printf("BIAS FILE NOT FOUND!!\n");
        return 1;
    }

    while (chk != EOF)
    {
        j = 0;
        chk = 0;
        while (chk != '\n' && chk != EOF)
        {
            fscanf(stream, "%f", &bias[i]);
            //printf("bias[%d]: %lf\n",i, bias[i]);
            chk = fgetc(stream);
            fseek(stream, -1, SEEK_CUR);
            j++;
        }
        i++;
        if (mj < j)
            mj = j;
    }
    fclose(stream);

    return 0;

    clock_t getbias_E = clock();
    printf("bias GET �ҿ� �ð�: %.3f\n", (float)(getbias_E - getbias_S) / CLOCKS_PER_SEC);
}

void getbias_(const char* filename, float* bias) {
    clock_t start = clock();
    printf("%s\n", filename);
    FILE* fp = NULL;
    int i = 0;
    if (fopen_s(&fp, filename, "rt") == 0) {
        while (fscanf_s(fp, "%f", &bias[i]) == 1) {
            //printf("bias[%d]: %f\n", i, bias[i]);
            i++;
        }
        fclose(fp);
        printf("count: %d\n", i);
    }
    else {
        printf("FILE NOT FOUND!\n");
    }
    clock_t end = clock();
    printf("get bias �ҿ�ð�: %f\n", (float)(end - start) / CLOCKS_PER_SEC);
}


void Matmul(float* Image, float* Output, float* matrix, int PW, int PH, int ksize, int pad)
{
    clock_t start = clock();
    int W = PW - (pad * 2);
    int H = PH - (pad * 2);

    for (int Y = 0; Y < W * H; Y++) {
        float temp = 0;
        for (int X = 0; X < PW * PH; X++) {
            temp += matrix[X + Y * (PW * PH)] * Image[X];
        }
        Output[Y] = temp;
        //printf("output: %f\n", Output[Y]);
    }

    clock_t end = clock();
    printf("MatMuL �ҿ� �ð�: %f\n", (float)(end - start) / CLOCKS_PER_SEC);
}

/*
void MakeMatrix_9(float(*kernel)[9], float* matrix, int W, int H, int padding, int ksize) {
    clock_t start;
    clock_t end;
    int PW = W + (padding * 2);
    int PH = H + (padding * 2);
    int width = PW * PH;
    int height = W * H;
    printf("PW, PH: %d, %d\n", PW, PH);
    start = clock();

    for (int y = 0; y < H; y++) {
        int xcnt = PW * y;
        for (int x = 0; x < W; x++) { // matmul y��(output ������)
            int index = xcnt;

            for (int ky = 0; ky < ksize; ky++) {
                for (int kx = 0; kx < ksize; kx++) {

                    //printf("--%d\n", index);
                    matrix[(y * W + x) * width + index] = kernel[ky][kx];

                    //printf("%d\n", (y * W + x)* width + index);
                    index++;
                }
                index = index + (PW - ksize);
            }
            xcnt = xcnt + 1;
        }
    }


    end = clock();
    printf("Matrix ���� �ҿ� �ð�: %.3f\n", (float)(end - start) / CLOCKS_PER_SEC);

}
*/

/*
void MakeMatrix_5(float(*kernel)[5], float* matrix, int W, int H, int padding, int ksize) {
    clock_t start;
    clock_t end;
    int PW = W + (padding * 2);
    int PH = H + (padding * 2);
    int width = PW * PH;
    int height = W * H;

    start = clock();
    int cnt = 0;

    for (int y = 0; y < H; y++) {
        int xcnt = PW * y;
        for (int x = 0; x < W; x++) { // matmul y��(output ������)
            int index = xcnt;

            for (int ky = 0; ky < ksize; ky++) {
                for (int kx = 0; kx < ksize; kx++) {

                    //printf("--%d\n", index);
                    matrix[(y * W + x) * width + index] = kernel[ky][kx];

                    //printf("%d\n", (y * W + x)* width + index);
                    index++;
                }
                index = index + (PW - ksize);
            }
            xcnt = xcnt + 1;
        }
    }
    end = clock();
    printf("Matrix ���� �ҿ� �ð�: %.3f\n", (float)(end - start) / CLOCKS_PER_SEC);

}
*/

/*CONVOLUTION*/
/*
void Matmul_9(float* Image, float* Output, float(*kernel)[9], int PW, int PH, int ksize, int pad)
{
    clock_t start = clock();
    int W = PW - (pad * 2);
    int H = PH - (pad * 2);

    for (int y = pad; y < H + pad; y++) {
        for (int x = pad; x < W + pad; x++) {
            int index = (y - pad) * W + (x - pad);
            for (int ky = -(ksize/2); ky < (ksize/2)+1; ky++) {
                for (int kx = -(ksize / 2); kx < (ksize / 2) + 1; kx++) {
                    Output[index] += Image[x + kx + (y + ky) * PW] * kernel[ky + (ksize/2)][kx + (ksize / 2)];
                }
            }
        }
    }
    clock_t end = clock();
    printf("MatMuL �ҿ� �ð�: %f\n", (float)(end - start) / CLOCKS_PER_SEC);
}

void Matmul_5(float* Image, float* Output, float(*kernel)[5], int PW, int PH, int ksize, int pad)
{
    clock_t start = clock();
    int W = PW - (pad * 2);
    int H = PH - (pad * 2);

    for (int y = pad; y < H + pad; y++) {
        for (int x = pad; x < W + pad; x++) {
            int index = (y - pad) * W + (x - pad);
            for (int ky = -(ksize / 2); ky < (ksize / 2) + 1; ky++) {
                for (int kx = -(ksize / 2); kx < (ksize / 2) + 1; kx++) {
                    Output[index] += Image[x + kx + (y + ky) * PW] * kernel[ky + (ksize / 2)][kx + (ksize / 2)];
                }
            }
        }
    }
    clock_t end = clock();
    printf("MatMuL �ҿ� �ð�: %f\n", (float)(end - start) / CLOCKS_PER_SEC);
}
*/