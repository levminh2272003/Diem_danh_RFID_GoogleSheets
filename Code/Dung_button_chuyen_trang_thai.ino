#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RFID
#define SS_PIN 16
#define RST_PIN 17
MFRC522 rfid(SS_PIN, RST_PIN);

// Hệ thống
#define On_Board_LED_PIN 2
#define BTN_IO 15
#define LED_IO 4
#define BUZZER 5

// WiFi và Google Sheets
const char* ssid = "Tho";  
const char* password = "99999999";  
String Web_App_URL = "https://script.google.com/macros/s/AKfycbzlcfhPNHppaWHHNMgLovYjB1UwICyjCX2uEn44xlS4mZFPkMLU0Edaj7TBSLfJCflT/exec";

// Cấu trúc dữ liệu sinh viên
#define MAX_STUDENTS 10

struct Student {
  int id;
  char code[10];
  char name[30];
  char student_id[15];
};

Student students[MAX_STUDENTS];
int studentCount = 0;

// Trạng thái VAO/RA của từng sinh viên
struct StudentStatus {
  char student_id[15];
  bool lastState; // 0: VAO, 1: RA
  bool isRecorded; // Đánh dấu nếu đã ghi trạng thái hiện tại
};

StudentStatus studentStatus[MAX_STUDENTS];

// Biến toàn cục
String uidString;
int runMode = 2;
bool btnIOState = HIGH;
bool InOutState = 0;
unsigned long timeDelay1 = millis();
unsigned long timeDelay2 = millis();

// Keypad 4x4
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {32, 33, 25, 26};
byte colPins[COLS] = {27, 14, 12, 13};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String enteredStudentID = "";  // Lưu mã sinh viên nhập từ keypad

void setup() {
  Serial.begin(115200);
  
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
  pinMode(BTN_IO, INPUT_PULLUP);
  pinMode(LED_IO, OUTPUT);
  pinMode(On_Board_LED_PIN, OUTPUT);

  // Khởi tạo OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  updateDisplay(); // Hiển thị tiêu đề & trạng thái ban đầu

  // Kết nối WiFi
  display.setCursor(0, 16);
  display.print("Ket noi WiFi...");
  display.display();
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(1000);
  }

  // Kết quả kết nối WiFi
  display.clearDisplay();
  updateDisplay();
  display.setCursor(0, 16);
  if (WiFi.status() == WL_CONNECTED) {
    display.print("WiFi OK");
  } else {
    display.print("WiFi Failed!");
  }
  display.display();
  delay(2000);

  // Hiển thị thông báo bắt đầu đọc dữ liệu từ Google Sheets
  display.clearDisplay();
  updateDisplay();
  display.setCursor(0, 16);
  display.print("Dang doc du lieu...");
  display.display();
  delay(2000);

  // Thực hiện đọc dữ liệu lần đầu
  bool readSuccess = readDataSheet();
  
  while (!readSuccess) { // Nếu đọc thất bại, chờ bấm nút A để thử lại
    display.clearDisplay();
    updateDisplay();
    display.setCursor(0, 16);
    display.print("Loi readDataSheet!");
    display.setCursor(0, 32);
    display.print("Bam 'A' de thu lai");
    display.display();

    char key;
    do {
      key = keypad.getKey();  // Chờ người dùng bấm nút trên Keypad
    } while (key != 'A'); // Khi bấm 'A', thử lại đọc dữ liệu

    display.clearDisplay();
    updateDisplay();
    display.setCursor(0, 16);
    display.print("Thu lai doc du lieu...");
    display.display();
    delay(1000);

    readSuccess = readDataSheet();
  }

  // Nếu đọc thành công
  display.clearDisplay();
  updateDisplay();
  display.setCursor(0, 16);
  display.print("Doc thanh cong!");
  display.setCursor(0, 32);
  display.print("Du lieu OK");
  display.display();
  delay(2000);
  showDefaultScreen();

  // Khởi tạo RFID
  SPI.begin();
  rfid.PCD_Init();
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  // Tiêu đề hệ thống
  display.setCursor(0, 0);
  display.println("HT Diem danh");

  // Hiển thị trạng thái VAO/RA
  display.setCursor(100, 0);
  display.print(InOutState ? "RA" : "VAO");
  
  display.display();
}
void showDefaultScreen() {
  display.clearDisplay();
  updateDisplay();
  display.setCursor(0, 16);
  display.print("Nhap MSV hoac");
  display.setCursor(0, 32);
  display.print("Quet the RFID");
  display.display();
}

void loop() {
  const unsigned long READ_UID_INTERVAL = 500;
  const unsigned long BUTTON_DEBOUNCE_TIME = 500;

  // Đọc UID từ RFID theo chu kỳ
  if (millis() - timeDelay2 >= READ_UID_INTERVAL) {
    readUID();
    timeDelay2 = millis();
  }

  // Xử lý nút bấm chuyển đổi trạng thái VAO/RA
  bool buttonState = digitalRead(BTN_IO);
  if (buttonState == LOW && btnIOState == HIGH) {
    if (millis() - timeDelay1 >= BUTTON_DEBOUNCE_TIME) {
      InOutState = !InOutState;
      digitalWrite(LED_IO, InOutState);
      showDefaultScreen();

      timeDelay1 = millis();
    }
    btnIOState = LOW;
  } 
  else if (buttonState == HIGH) {
    btnIOState = HIGH;
  }

// Nhập mã sinh viên từ Keypad
char key = keypad.getKey();

// Thêm biến static để xử lý xác nhận reset
static bool doiXacNhanReset = false;
static unsigned long thoiGianReset = 0;

if (key) {
  display.clearDisplay();
  updateDisplay();
  display.setCursor(0, 16);

  if (doiXacNhanReset) {
    if (key == '#') {
      display.print("Xac nhan reset");
      display.display();
      delay(1000);
      ESP.restart();  // Gọi hàm reset ESP32
    } else {
      display.print("Huy reset");
      doiXacNhanReset = false;  // Hủy nếu nhập khác #
    }
  }
  else if (key == 'D') {
    display.print("Nhan # de reset");
    doiXacNhanReset = true;
    thoiGianReset = millis();
  } 
  else if (key == '#') {  // Xác nhận nhập mã sinh viên
    display.print("Ma SV: " + enteredStudentID);
    display.setCursor(0, 32);
    display.print("Dang kiem tra...");
    display.display();
    checkStudentID(enteredStudentID);
    enteredStudentID = "";
  } 
  else if (key == '*') {  // Xóa mã nhập
    enteredStudentID = "";
    display.print("Da xoa ma nhap");
  } 
  else {
    enteredStudentID += key;
    display.print("Nhap: " + enteredStudentID);
  }

  display.display();
}

// Hủy trạng thái chờ xác nhận nếu quá 5 giây
if (doiXacNhanReset && millis() - thoiGianReset > 5000) {
  doiXacNhanReset = false;
}
  
}

void beep(unsigned int n, unsigned int d) {
  if (n == 0 || d == 0) return; // Không beep nếu số lần hoặc độ trễ bằng 0
  
  for (unsigned int i = 0; i < n; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(d);
    digitalWrite(BUZZER, LOW);
    
    if (i < n - 1) { // Chỉ delay giữa các lần beep (không delay sau lần cuối cùng)
      delay(d);
    }
  }
}

void readUID() {
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  uidString = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidString.concat(String(rfid.uid.uidByte[i] < 0x10 ? "0" : "")); 
    uidString.concat(String(rfid.uid.uidByte[i], HEX));
  }
  uidString.toUpperCase();
  beep(1, 200);

  display.clearDisplay();
  updateDisplay();
  display.setCursor(0, 16);
  display.print("UID: " + uidString);
  display.display();
  delay(500);

  if (runMode == 1) {
    writeUIDSheet();
    return;
  }

  if (runMode == 2) {
    Student* student = getStudentById(uidString.c_str());
    if (student != nullptr) {
      display.clearDisplay();
      updateDisplay();
      display.setCursor(0, 16);
      display.print("SV: " + String(student->name));

      StudentStatus* status = getStudentStatus(student->student_id);
      if (status != nullptr) {
        if (status->isRecorded && status->lastState == InOutState) {
          display.setCursor(0, 32);
          display.print("Da diem danh roi!");
          display.display();
          beep(2, 500);
          delay(2000);
          showDefaultScreen();
          return;
        }

        bool success = writeLogSheet(); // Gửi dữ liệu lên Google Sheets
        if (success) {
          status->lastState = InOutState;
          status->isRecorded = true;
          display.setCursor(0, 32);
          display.print("Diem danh thanh cong!");
        } else {
          display.setCursor(0, 32);
          display.print("Loi readUID ghi log!");
        }

        display.display();
        delay(2000);
      }
    } else {
      display.clearDisplay();
      updateDisplay();
      display.setCursor(0, 16);
      display.print("The khong hop le!");
      display.display();
      beep(3, 500);
      delay(2000);
    }
  }
  showDefaultScreen();
}

void checkStudentID(String studentID) {
  Student* student = getStudentByStudentID(studentID.c_str());

  display.clearDisplay();
  updateDisplay();
  display.setCursor(0, 16);

  if (student != nullptr) {
    display.print("SV: " + String(student->name));
    display.display();
    beep(1, 300);

    uidString = student->code;

    StudentStatus* status = getStudentStatus(student->student_id);
    if (status != nullptr) {
      if (status->isRecorded && status->lastState == InOutState) {
        display.setCursor(0, 32);
        display.print("Da diem danh roi!");
        display.display();
        beep(2, 500);
        delay(2000);
        showDefaultScreen();
        return;
      }

      bool success = writeLogSheet();
      if (success) {
        status->lastState = InOutState;
        status->isRecorded = true;
        display.setCursor(0, 32);
        display.print("Diem danh thanh cong!");
      } else {
        display.setCursor(0, 32);
        display.print("Loi check ghi log!");
      }

      display.display();
      delay(2000);
    }
  } else {
    display.print("Ma SV khong hop le!");
    display.display();
    beep(3, 200);
    delay(2000);
  }
  showDefaultScreen();
}

bool readDataSheet() {
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(On_Board_LED_PIN, HIGH);

    String Read_Data_URL = Web_App_URL + "?sts=read";

    display.clearDisplay();
    updateDisplay();
    display.setCursor(0, 16);
    display.print("Dang doc du lieu...");
    display.display();

    HTTPClient http;
    http.begin(Read_Data_URL.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    int httpCode = http.GET();
    
    String payload;
    studentCount = 0;

    if (httpCode == HTTP_CODE_OK) {
      payload = http.getString();

      char charArray[payload.length() + 1];
      payload.toCharArray(charArray, payload.length() + 1);

      int numberOfElements = countElements(charArray, ',');

      if (numberOfElements % 4 != 0) { // Kiểm tra số lượng cột hợp lệ
        display.setCursor(0, 32);
        display.print("Loi Du lieu readDataSheet!");
        display.display();
        delay(2000);
        http.end();
        digitalWrite(On_Board_LED_PIN, LOW);
        return false;
      }

      char *token = strtok(charArray, ",");
      while (token != NULL && studentCount < numberOfElements / 4) {
        students[studentCount].id = atoi(token);
        token = strtok(NULL, ",");
        if (token == NULL) break;
        strcpy(students[studentCount].code, token);
        token = strtok(NULL, ",");
        if (token == NULL) break;
        strcpy(students[studentCount].name, token);
        token = strtok(NULL, ",");
        if (token == NULL) break;
        strcpy(students[studentCount].student_id, token);

        studentCount++;
        token = strtok(NULL, ",");
      }
    } else {
      display.setCursor(0, 32);
      display.print("Loi Ket noi readDataSheet!");
      display.display();
      delay(2000);
    }

    http.end();
    digitalWrite(On_Board_LED_PIN, LOW);

    return studentCount > 0;
  }

  return false;
}

void writeUIDSheet() {
  if (WiFi.status() != WL_CONNECTED) {
    display.clearDisplay();
    updateDisplay();
    display.setCursor(0, 16);
    display.print("Loi writeUIDSheet WiFi chua ket noi!");
    display.display();
    delay(2000);
    return;
  }

  String Send_Data_URL = Web_App_URL + "?sts=writeuid";
  Send_Data_URL += "&uid=" + uidString;

  display.clearDisplay();
  updateDisplay();
  display.setCursor(0, 16);
  display.print("Dang gui du lieu...");
  display.display();

  HTTPClient http;
  http.begin(Send_Data_URL.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  int httpCode = http.GET();
  String payload;

  if (httpCode > 0) {
    payload = http.getString();
    display.clearDisplay();
    updateDisplay();
    display.setCursor(0, 16);
    display.print("Gui thanh cong!");
    display.display();
  } else {
    display.clearDisplay();
    updateDisplay();
    display.setCursor(0, 16);
    display.print("Loi write UID GGSheets!");
    display.display();
  }

  delay(2000);
  http.end();
}

bool writeLogSheet() {
  if (WiFi.status() != WL_CONNECTED) {
    display.clearDisplay();
    updateDisplay();
    display.setCursor(0, 16);
    display.print("Loi writeLogSheet WiFi chua ket noi!");
    display.display();
    delay(2000);
    return false;
  }

  char charArray[uidString.length() + 1];
  uidString.toCharArray(charArray, uidString.length() + 1);

  Student* student = getStudentById(charArray);
  if (student != nullptr) {
    display.clearDisplay();
    updateDisplay();
    display.setCursor(0, 16);
    display.print("Dang gui diem danh...");
    display.setCursor(0, 32);
    display.print(student->name);
    display.setCursor(0, 48);
    display.print(InOutState == 0 ? "VAO" : "RA");
    display.display();

    String Send_Data_URL = Web_App_URL + "?sts=writelog";
    Send_Data_URL += "&uid=" + urlencode(String(student->code));
    Send_Data_URL += "&name=" + urlencode(String(student->name));
    Send_Data_URL += "&masv=" + urlencode(String(student->student_id));
    Send_Data_URL += "&inout=" + urlencode(InOutState == 0 ? "VAO" : "RA");

    HTTPClient http;
    http.begin(Send_Data_URL.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    int httpCode = http.GET();
    String payload;
    bool success = false;

    if (httpCode > 0) {
      payload = http.getString();
      display.clearDisplay();
      updateDisplay();
      display.setCursor(0, 16);
      display.print("Ghi thanh cong!");
      display.display();
      success = true;
    } else {
      display.clearDisplay();
      updateDisplay();
      display.setCursor(0, 16);
      display.print("Loi writeLogSheet!");
      display.display();
    }

    delay(2000);
    http.end();
    return success;
  } else {
    beep(3, 500);

    display.clearDisplay();
    updateDisplay();
    display.setCursor(0, 16);
    display.print("SV khong hop le!");
    display.display();
    delay(2000);
    return false;
  }
}

// Hàm lấy trạng thái của sinh viên
StudentStatus* getStudentStatus(const char* studentID) {
  for (int i = 0; i < MAX_STUDENTS; i++) {
    if (strcmp(studentStatus[i].student_id, studentID) == 0) {
      return &studentStatus[i];
    }
  }

  for (int i = 0; i < MAX_STUDENTS; i++) {
    if (studentStatus[i].student_id[0] == '\0') { // Chưa có dữ liệu
      strcpy(studentStatus[i].student_id, studentID);
      studentStatus[i].lastState = InOutState;
      studentStatus[i].isRecorded = false;
      return &studentStatus[i];
    }
  }
  
  return nullptr;
}

String urlencode(String str) {
  String encodedString = "";
  char c;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      encodedString += '%';
      encodedString += String(c, HEX);
    }
    yield();
  }
  return encodedString;
}

Student* getStudentById(const char* uid) {
  for (int i = 0; i < studentCount; i++) {
    if (strcmp(students[i].code, uid) == 0) {
      return &students[i];  // Trả về con trỏ đến sinh viên tìm thấy
    }
  }
  return nullptr;  // Không tìm thấy
}

Student* getStudentByStudentID(const char* studentID) {
  for (int i = 0; i < studentCount; i++) {
    if (strcmp(students[i].student_id, studentID) == 0) {
      return &students[i];  // Trả về con trỏ đến sinh viên tìm thấy
    }
  }
  return nullptr;  // Không tìm thấy
}

int countElements(const char* data, char delimiter) {
  // Tạo một bản sao của chuỗi dữ liệu để tránh thay đổi chuỗi gốc
  char dataCopy[strlen(data) + 1];
  strcpy(dataCopy, data);
  
  int count = 0;
  char* token = strtok(dataCopy, &delimiter);
  while (token != NULL) {
    count++;
    token = strtok(NULL, &delimiter);
  }
  return count;
}