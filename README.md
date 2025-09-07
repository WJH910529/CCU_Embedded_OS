# CCU_Embedded_System [![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/WJH910529/CCU_Embedded_OS)

![FreeRTOS](https://github.com/WJH910529/CCU_Embedded_System/blob/c7cfd1d76f9becc386576dab2497d93e551ca45e/picture/FREERTOS.jpg)

![nuc140](https://github.com/WJH910529/CCU_Embedded_System/blob/cac4da5d3b81eb787f6e2adf06113e29ae906f57/picture/NUC-140.jpg)

Demo projects for the Embedded Operating System coursework at Chung Cheng University, implemented using FreeRTOS and NUC140.

# LAB1: Task Fundamentals (任務基礎)
LAB1 建立了 FreeRTOS 多任務系統的基礎架構。系統創建四個獨立任務：兩個訊息列印任務（500ms 和 1000ms 間隔）和兩個 LED 控制任務（PC12 和 PC13 引腳）。
這個實驗展示了基本的任務創建、調度和硬體控制。

# LAB2: Inter-Task Communication (任務間通訊)
LAB2 引入 FreeRTOS queue 機制實現任務間通訊。
系統使用 xCommandQueueA 佇列讓 TaskA 發送指令給 TaskB，實現生產者-消費者模式。
進階版本還包含雙向通訊和指令-回應機制。

# LAB3: Timers and Event Handling (計時器與事件處理)
LAB3 實現 FreeRTOS 軟體計時器功能。
基礎版本使用 3 秒週期計時器，
進階版本結合 GPIO 中斷處理，實現按鍵控制的計時器啟停功能。

# LAB4: Synchronization Mechanisms (同步機制)
LAB4 展示 FreeRTOS 同步的應用。
基礎版本使用計數信號量管理停車場資源，支援進入/離開按鈕控制。
進階版本使用互斥鎖保護資源，展示不同優先級任務的競爭與協調。

# LAB5: Event Groups (事件群組)
LAB5 使用 FreeRTOS Event Group 實現多模式事件同步。
系統定義四個模式位元控制不同的 LED 閃爍模式，TaskA 設定事件，TaskB 控制標準 LED，TaskC 控制 RGB LED 顏色輸出。
這展示了一對多事件廣播機制 。

# Conclusion
整個專案從基礎任務管理逐步進階到複雜的同步機制，涵蓋了 FreeRTOS 的核心功能：任務調度、佇列通訊、軟體計時器、信號量/互斥鎖、和事件群組。
每個 LAB 都結合實際硬體控制（LED、按鍵、UART），展示完整的嵌入式系統開發。
