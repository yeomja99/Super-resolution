#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <math.h>
#include <time.h>

#pragma warning(disable : 4996)

void padding_initialize(BYTE* Padding, BYTE* Image, int PW, int PH, int pad) {


	int W = PW - 12, H = PH - 12; // pad 입히지 않은 Origin Iamge Size (12 = 2*2(양쪽)*3(채널))

	// Zero Padding
	for (int i = 0; i < PH; i++) {
		for (int j = 0; j < PW; j++) {
			Padding[i * PW * 3 + j * 3] = 0;
			Padding[i * PW * 3 + j * 3 + 1] = 0;
			Padding[i * PW * 3 + j * 3 + 2] = 0;
		}
	}

	// Origin Pixel Value 
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			Padding[(i + pad) * PW * 3 + (j + pad) * 3] = Image[i * W * 3 + j * 3]; // Blue
			Padding[(i + pad) * PW * 3 + (j + pad) * 3 + 1] = Image[i * W * 3 + j * 3 + 1]; // Green
			Padding[(i + pad) * PW * 3 + (j + pad) * 3 + 2] = Image[i * W * 3 + j * 3 + 2]; // Red

		}
	}
}

float u(float s, float a) {
	if ((fabs(s) >= 0) && (fabs(s) <= 1))
		return (a + 2) * (fabs(s) * fabs(s) * fabs(s)) - (a + 3) * (fabs(s) * fabs(s)) + 1;

	else if ((fabs(s) > 1) && (fabs(s) <= 2))
		return 0 - a * (fabs(s) * fabs(s) * fabs(s)) - (5 * a) * (fabs(s) * fabs(s)) + (8 * a) * fabs(s) - 4 * a;

	return 0;

}


void bicubic(BYTE* Image, BYTE* Output, int W, int H, int ratio, float a)
{
	int OW = (W - 12) * ratio; // Output Width
	int OH = (H - 12) * ratio; // Output Height

	float cx, cy; // 원본 이미지의 center x, y 좌표 
	float x[4];
	float y[4];
	float temp_v = 0;
	float mat_l[1][4];
	float mat_m[4][4];
	float mat_temp[1][4];
	float mat_r[4][1];
	float rpix = 0;

	float h = 1 / (float)ratio; // 1/ratio
	//printf("\nh:%f\n", h);

	for (int c = 0; c < 3; c++) {
		for (int i = 0; i < OH; i++) {
			for (int j = 0; j < OW; j++) {

				// Output에 와야하는 원본 이미지 좌표를 역연산
				cx = j * h + 6 - 4.3; // j = 3 cx = 3.5 
				cy = i * h + 6 - 4.3;


				// 역연산된 좌표에서 상, 하, 좌, 우를 보기위한 좌표값 할당
				x[0] = 1 + cx - floor(cx); // 1.5
				x[1] = cx - floor(cx); // 0.5
				x[2] = floor(cx) + 1 - cx; // 0.5
				x[3] = floor(cx) + 2 - cx; // 1.5



				y[0] = 1 + cy - floor(cy);
				y[1] = cy - floor(cy);
				y[2] = floor(cy) + 1 - cy;
				y[3] = floor(cy) + 2 - cy;



				// Matrix Setting

				for (int k = 0; k < 4; k++) {
					// printf("\nx[%d] : %f\n", k, x[k]);
					// printf("mat_l[0][%d] = u(x[%d],a) = %f\n",k,k,u(x[k],a));
					mat_l[0][k] = u(x[k], a);
				}


				for (int ki = 0; ki < 4; ki++) {
					for (int kj = 0; kj < 4; kj++) {

						if (ki < 2 && kj < 2)
							mat_m[ki][kj] = Image[(int)(cy - y[kj]) * 3 * W + (int)(cx - x[ki]) * 3 + c];

						else if (ki < 2 && kj >= 2)
							mat_m[ki][kj] = Image[(int)(cy + y[kj]) * 3 * W + (int)(cx - x[ki]) * 3 + c];

						else if (ki >= 2 && kj < 2)
							mat_m[ki][kj] = Image[(int)(cy - y[kj]) * 3 * W + (int)(cx + x[ki]) * 3 + c];

						else
							mat_m[ki][kj] = Image[(int)(cy + y[kj]) * 3 * W + (int)(cx + x[ki]) * 3 + c];

					}
				} // 이게 나은 지 값들 다 때려박는 게 나은지

				for (int k = 0; k < 4; k++)
					mat_r[k][0] = u(y[k], a);


				// Matrix Produntion

				for (int pi = 0; pi < 4; pi++) {
					for (int pj = 0; pj < 4; pj++) {
						temp_v += mat_l[0][pj] * mat_m[pj][pi];

					}
					rpix += temp_v * mat_r[pi][0];
					temp_v = 0;
				}


				if (rpix > 255) rpix = 255;
				if (rpix < 0) rpix = 0;

				Output[i * OW * 3 + j * 3 + c] = rpix;

				rpix = 0;
			}
		}
	}


}


void SaveBMPFile(BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo,
	RGBQUAD* hRGB, BYTE* Output, int W, int H, const char* FileName, float scale) {
	char result_dir[255] = "./results/";

	hInfo.biWidth = W;
	hInfo.biHeight = H;

	printf("%d, %d\n", hInfo.biWidth, hInfo.biHeight);
	FILE* fp = fopen(strcat(result_dir, FileName), "wb");
	printf("%s\n", result_dir);

	if (hInfo.biBitCount == 24) {
		fwrite(&hf, sizeof(BYTE), sizeof(BITMAPFILEHEADER), fp);
		fwrite(&hInfo, sizeof(BYTE), sizeof(BITMAPINFOHEADER), fp);
		fwrite(Output, sizeof(BYTE), W * H * 3, fp);
		printf("save\n");
	}
	else if (hInfo.biBitCount == 8) {
		fwrite(&hf, sizeof(BYTE), sizeof(BITMAPFILEHEADER), fp);
		fwrite(&hInfo, sizeof(BYTE), sizeof(BITMAPINFOHEADER), fp);
		fwrite(hRGB, sizeof(RGBQUAD), 256, fp);
		fwrite(Output, sizeof(BYTE), W * H, fp);
	}
	fclose(fp);
}


int main() {
	char file_dir[255] = "./test_imgs/";

	clock_t start;
	clock_t end;


	start = clock();

	BITMAPFILEHEADER hf;
	BITMAPINFOHEADER hInfo;
	RGBQUAD hRGB[256];
	FILE* fp;
	fp = fopen("./test/1920.bmp", "rb");

	if (fp == NULL) {
		printf("File not found!\n");
		return -1;
	}

	fread(&hf, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&hInfo, sizeof(BITMAPINFOHEADER), 1, fp);

	int ImgSize = hInfo.biWidth * hInfo.biHeight;
	int size = hInfo.biSizeImage;
	int H = hInfo.biHeight, W = hInfo.biWidth;

	BYTE* Image;
	BYTE* Output;

	float scale_factor = 2;

	if (hInfo.biBitCount == 24) {
		Image = (BYTE*)malloc(ImgSize * 3);
		fread(Image, sizeof(BYTE), ImgSize * 3, fp);
	}

	else {
		fread(hRGB, sizeof(RGBQUAD), 256, fp);
		Image = (BYTE*)malloc(ImgSize);
		Output = (BYTE*)malloc(ImgSize);

		fread(Image, sizeof(BYTE), ImgSize, fp);
	}

	Output = (BYTE*)malloc(scale_factor * scale_factor * ImgSize * 3);
	end = clock();
	float read_time = (float)(end - start) / CLOCKS_PER_SEC;

	int padding_size = 2;
	int padding_i = 2 * 2 * 3;
	int PW = W + padding_i; // Padding Image W , 위 아래 2패딩 = 4 &  * 3 채널
	int PH = H + padding_i; // Padding Image H
	float a = -(1 / 2);
	BYTE* Padding = (BYTE*)malloc(PW * PH * 3);


	clock_t start1 = clock();

	padding_initialize(Padding, Image, PW, PH, 2); // 2 Zero Padding 적용된 결과가 Padding에 저장
	bicubic(Padding, Output, PW, PH, scale_factor, a);

	clock_t end1 = clock();
	float ftime = (float)(end1 - start1) / CLOCKS_PER_SEC;

	printf("\ntime: %.3f\n", ftime);

	SaveBMPFile(hf, hInfo, hRGB, Output, W * scale_factor, H * scale_factor, "result.bmp", scale_factor);


	free(Image);
	free(Output);
	//free(Temp);
	return 0;
}