환경 모니터링 시스템
---
이 프로젝트는 STM32F103C8T6 보드와 FreeRTOS를 기반으로 온습도 센서(SHT31)의 데이터를 읽고,  
OLED(SSD1306)에 실시간 출력하며, UART 명령과 버튼으로 제어하는 환경 모니터링 시스템입니다.

- MCU  : STM32F103C8T6
- RTOS : FreeRTOS
- 센서 : SHT31 온습도 센서 (I2C 통신)
- OLED : SSD1306
- UART : FT232rl (USB to UART)
- 개발 환경 : STM32CubeIDE

---
기능 요약
---
- UART "start" → 센서 측정 시작
- UART "stop" → 센서 측정 중단 및 OLED 초기화
- 버튼 입력으로 OLED 출력 모드 변경 (온습도 → 온도 → 습도)
- 온습도 값이 임계치 초과 시 LED 및 UART 메세지 경고
---
 
HAL_UART_RxCpltCallback  : UART명령 수신 및 Uart Task에 queue전송


Uart_startTask  : queue수신, Sensor_startTask를 Resume 또는 Suspend




Sensor_startTask  : 센서값 수신 및 Oled Task에 queue전송


Oled_startTask  : queue수신, Oled화면 출력, display mode구분, 센서값 임계치 초과 시 Warning Task에 queue전송




HAL_GPIO_EXTI_Callback  : Button 입력 수신 및 Button Task에 queue전송


Button_startTask  : queue수신 및 Oled Task로 queue 전송(display mode 변경)




Warining_startTask  : Oled Task로부터 queue수신, Led on


---
주요 구조
---
myProject
>Core
>> Src
>>> main.c : 각 Task 로직 및 UART/Button 수신 로직

>> Lib
>>> Inc : SSD1306 및 Sht31 헤더

>>> Src : SSD1306 및 Sht31 소스코드
---
