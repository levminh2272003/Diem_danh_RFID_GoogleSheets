# Hệ thống điểm danh RFID & ESP32 kết nối Google Sheets  

## Giới thiệu  

Dự án này xây dựng **hệ thống điểm danh tự động** sử dụng **ESP32**, **RFID RC522**, **Keypad 4x4**, **màn hình OLED SSD1306**, và **Google Sheets** để **quản lý điểm danh sinh viên theo thời gian thực**.  

Điểm đặc biệt của hệ thống là khả năng **xác thực dữ liệu hai chiều**:  
- ESP32 **tải danh sách hợp lệ** từ Google Sheets.  
- Khi quét thẻ hoặc nhập mã, hệ thống **đối chiếu dữ liệu** trước khi ghi log.  

### Mục tiêu hệ thống  
- Tự động hóa quá trình điểm danh.  
- Giảm gian lận và sai sót.  
- Dễ dàng quản lý dữ liệu trên nền tảng đám mây.  

---

## Thành phần phần cứng  

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

## Phần mềm và công cụ  

- **Arduino IDE** – Môi trường lập trình cho ESP32.  
- **Google Sheets** – Lưu trữ dữ liệu điểm danh và danh sách sinh viên.  
- **Google Apps Script (Web App)** – Giao tiếp giữa ESP32 và Google Sheets thông qua HTTP.  

## Nguyên lý hoạt động  

### 1. Cập nhật danh sách hợp lệ  
ESP32 gửi yêu cầu **HTTP GET** đến **Google Apps Script** để nhận danh sách **UID và mã sinh viên hợp lệ** từ **Google Sheets**, sau đó lưu tạm trong bộ nhớ.  

---

### 2. Xác thực dữ liệu điểm danh  
- Khi **quét thẻ RFID** hoặc **nhập mã sinh viên** trên Keypad, ESP32 kiểm tra dữ liệu với danh sách đã tải về.  
- Nếu **hợp lệ** → hiển thị thông tin sinh viên và trạng thái **“Vào” / “Ra”** trên OLED, sau đó gửi log lên Google Sheets.  
- Nếu **không hợp lệ** → phát âm cảnh báo bằng **Buzzer** và hiển thị **“Thẻ không hợp lệ”**.  

---

### 3. Ghi dữ liệu lên Google Sheets  
ESP32 gửi dữ liệu qua **HTTP POST** đến Apps Script, bao gồm:  
Mã sinh viên | UID | Họ tên | Thời gian | Trạng thái (Vào/Ra)
Dữ liệu được ghi vào bảng tính **Google Sheets** trong thời gian thực.  

---

### 4. Theo dõi thời gian thực  
Người quản lý có thể theo dõi **danh sách điểm danh ngay lập tức** trên Google Sheets, thống kê theo **ngày** hoặc **sinh viên**.  

---

## Kết quả đạt được  

- Hệ thống hoạt động ổn định với hai chế độ điểm danh: **RFID** và **Keypad**.  
- Dữ liệu được **xác thực trước khi ghi**, đảm bảo chính xác và tránh gian lận.  
- **Đồng bộ hai chiều** giữa ESP32 và Google Sheets (ESP32 ↔ Sheets).  
- Hiển thị thông tin sinh viên, thời gian, trạng thái trên **màn hình OLED**.  
- Có thể mở rộng quy mô cho **nhiều lớp học, doanh nghiệp hoặc tổ chức**.  

---

## Hướng phát triển  

- Tích hợp **nhận diện khuôn mặt** để tăng tính bảo mật.  
- Xây dựng **web dashboard** hiển thị thống kê chuyên cần.  
- Gửi thông báo **qua Email hoặc Telegram** khi sinh viên điểm danh.  
- Cải thiện bảo mật bằng **Google OAuth 2.0** hoặc **token xác thực riêng**.  
