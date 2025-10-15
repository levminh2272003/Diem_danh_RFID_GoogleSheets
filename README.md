# 🎓 Hệ thống điểm danh RFID & ESP32 kết nối Google Sheets  

## 🧠 Giới thiệu  

Dự án này xây dựng **hệ thống điểm danh tự động** sử dụng **ESP32**, **RFID RC522**, **Keypad 4x4**, **màn hình OLED SSD1306**, và **Google Sheets** để **quản lý điểm danh sinh viên theo thời gian thực**.  

Điểm đặc biệt của hệ thống là khả năng **xác thực dữ liệu hai chiều**:  
- ESP32 **tải danh sách hợp lệ** từ Google Sheets.  
- Khi quét thẻ hoặc nhập mã, hệ thống **đối chiếu dữ liệu** trước khi ghi log.  

### 🎯 Mục tiêu hệ thống  
- Tự động hóa quá trình điểm danh.  
- Giảm gian lận và sai sót.  
- Dễ dàng quản lý dữ liệu trên nền tảng đám mây.  

---

## ⚙️ Thành phần phần cứng  

| Linh kiện | Chức năng chính |
|------------|------------------|
| **ESP32** | Vi điều khiển trung tâm, kết nối WiFi và xử lý logic điểm danh |
| **RFID RC522** | Đọc UID thẻ RFID của sinh viên |
| **Keypad 4x4** | Nhập mã sinh viên thủ công khi không có thẻ |
| **OLED SSD1306 (0.96")** | Hiển thị thông tin người dùng, trạng thái, kết nối WiFi |
| **Button** | Chuyển trạng thái “Vào” / “Ra” |
| **LED & Buzzer** | Hiển thị và cảnh báo kết quả xác thực |
| **Adapter 5V** | Cấp nguồn cho toàn hệ thống |

---

## 💻 Phần mềm và công cụ  

- **Arduino IDE** – Môi trường lập trình cho ESP32.  
- **Google Sheets** – Lưu trữ dữ liệu điểm danh và danh sách sinh viên.  
- **Google Apps Script (Web App)** – Giao tiếp giữa ESP32 và Google Sheets thông qua HTTP.  

### 🧩 Thư viện sử dụng  
```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <MFRC522.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
