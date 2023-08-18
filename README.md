# Super-resolution

## 📖상세 설명

![Untitled](https://s3-us-west-2.amazonaws.com/secure.notion-static.com/640ce31f-aaf3-4716-afba-bc0804114b2a/Untitled.png)

<aside>
💡 “심층학습 기반의 영상 개선 기술을 연동한 저전력 Mobile GPU platform 개발” 연구 과제에 참여하여 진행한 프로젝트입니다.  
해당 프로젝트에서 저희 연구실은 **“딥러닝 기반 초해상화 기술” 부분 담당**으로, **심층학습 기반의 초해상화 모델을 C언어로 구현**하고 이를 GPU 가속을 받을 수 있도록 library를 사용하여 구현하는 역할을 담당했습니다. 
제가 맡은 역할은 **linux 환경에서 실행되는 심층학습 기반의 초해상화 기술을 C언어로 구현**하기 입니다.

</aside>

### 🔖**요약**

- linux 환경에서 실행되는 심층학습 기반의 초해상화 기술을 C로 구현

### 🛠 기술 스택

- Python
- PyTorch
- C/C++

### 💻**역할**

- 4명이 진행
- 맡은 역할
    - 딥러닝을 사용하지 않은 **고전 초해상화 기술 수집 및 C++ 구현**
        - Bilinear, Bicubic, Biliteral
    - **딥러닝 기반의 초해상화 모델 수집 및 코드 리뷰**
        - SRGAN, SRCNN, FSRCNN
    - SRGAN, SRCNN, FSRCN Python(PyTorch) 모델 포팅 및 훈련
    - **SRCNN, FSRCNN C/C++ 구현** 및 실행 파일 생성
    - 연구 노트 및 보고서 작성
    
- 기여도: 80%

### 📅**시기**

- 2021.09-2022.12
