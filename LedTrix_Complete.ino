// Program to exercise the MD_MAX72XX library
//
// Uses most of the functions in the library
#include <MD_MAX72xx.h>
#include <Wire.h>
#include <SPI.h>
#include "Font.h"
#include "RTClib.h"
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES  16

#define Max_row 16
#define Max_col (MAX_DEVICES*8)/2

#define CLK_PIN   18   //สำหรับ Arduino
#define DATA_PIN  23   //สำหรับ Arduino
#define CS_PIN    5   //สำหรับ Arduino

#define SW 16
#define Buzzer 27
#define PIR 34
#define SW_Secure 4
//#define CLK_PIN   D5 //สำหรับ NodeMcu
//#define CS_PIN    D8 //สำหรับ NodeMcu
//#define DATA_PIN  D7 //สำหรับ NodeMcu

char auth[] = "W1h0PeKNFi1nBM3i0kMlWTtM1xGjvrj1"; //Token key
char server[] = "blynk.honey.co.th";
char ssid[] = "ParallelWorld";
char pass[] = "pochenterprise";
int port = 8080;

bool insetupmode = false;
bool warningState = false;
bool timeInSc = true;
int text_Min;
int text_Hour;
String text_In;
int temp_Min;
int temp_Hour;
int countPIR = 0;

String warningMessage = "มีผู้บุกรุก!!!";

RTC_DS3231 clockk;

int gap_pixel = 1; //ระยะช่องไฟ
bool state = true; //ถ้าค่าเป็นเท็จจะแสดงผลตัวอักษรมืดแต่รอบด้านสว่าง

// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary pins
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);



void mysetpoint(uint16_t x, uint16_t y, bool z) //ลงจุด xy ใหม่สำหรับ dot matrix แบบ 2 แถว
{
  uint16_t my_x, my_y;

  if (x < 0 || x > Max_col - 1) return; //ถ้าเกินขอบเขตของแกน X ให้หยุดตรวจสอบ
  if (y < 0 || y > Max_row - 1) return; //ถ้าเกินขอบเขตของแกน y ให้หยุดตรวจสอบ

  my_x = Max_col - x - 1; //กำหนดให้แกน x ตำแหน่ง 0 อยู่ล่างซ้ายสุด
  my_y = 7 - y;
  if (y >= 8) {
    my_x = my_x + Max_col;  //กำหนดให้แกน y ตำแหน่ง 0 ล่างซ้ายสุด
    my_y = 15 - y;
  }
  mx.setPoint(my_y, my_x, z);
}

void DrawChar(int x, int y, unsigned char input_char, bool invert) //ฟังก์ชั่นพิมพ์ทีละอักษรตามตำแหน่ง ASCii ฟอนต์
{
  unsigned char mc;
  char my_x = 0, data = 0;

  if (!invert) //ให้แสดงผลปรกติ
  {
    while (data < 16) //แสกนจากบนลงล่างเพื่อลงจุด
    {
      mc = pgm_read_byte_near(&Matrix_16_Font[input_char][data]); //อ่านฟอนต์จากหน่วยความจำมาทีละไบต์
      if ((mc & 0x80) == 0x80) my_x++; // อ่านบิตที่1ถ้าค่าตรงให้ลงจุด
      else {
        mysetpoint(x + my_x, 15 - data + y, true);  // อ่านบิตที่1ถ้าค่าไม่ตรงไม่ต้องลงจุด
        my_x++;
      }
      if ((mc & 0x40) == 0x40) my_x++; // อ่านบิตที่2ถ้าค่าตรงให้ลงจุด
      else {
        mysetpoint(x + my_x, 15 - data + y, true);
        my_x++;
      }
      if ((mc & 0x20) == 0x20) my_x++; // อ่านบิตที่3ถ้าค่าตรงให้ลงจุด
      else {
        mysetpoint(x + my_x, 15 - data + y, true);
        my_x++;
      }
      if ((mc & 0x10) == 0x10) my_x++; // อ่านบิตที่4ถ้าค่าตรงให้ลงจุด
      else {
        mysetpoint(x + my_x, 15 - data + y, true);
        my_x++;
      }
      if ((mc & 0x08) == 0x08) my_x++; // อ่านบิตที่5ถ้าค่าตรงให้ลงจุด
      else {
        mysetpoint(x + my_x, 15 - data + y, true);
        my_x++;
      }
      if ((mc & 0x04) == 0x04) my_x++; // อ่านบิตที่6ถ้าค่าตรงให้ลงจุด
      else {
        mysetpoint(x + my_x, 15 - data + y, true);
        my_x++;
      }
      if ((mc & 0x02) == 0x02) my_x++; // อ่านบิตที่7ถ้าค่าตรงให้ลงจุด
      else {
        mysetpoint(x + my_x, 15 - data + y, true);
        my_x++;
      }
      if ((mc & 0x01) == 0x01) my_x = 0; // อ่านบิตที่8ถ้าค่าตรงให้ลงจุด
      else {
        mysetpoint(x + my_x, 15 - data + y, true);
        my_x = 0;
      }
      data++;
    }
  }
  else //ให้แสดงผลกลับการลงจุด
  {
    while (data < 16) //แสกนจากบนลงล่างเพื่อลงจุด
    {
      mc = pgm_read_byte_near(&Matrix_16_Font[input_char][data]); //อ่านฟอนต์จากหน่วยความจำมาทีละไบต์
      if ((mc & 0x80) == 0x80) {
        mysetpoint(x + my_x, 15 - data + y, true);  // อ่านบิตที่1ถ้าค่าตรงให้ลงจุด
        my_x++;
      }
      else my_x++; // อ่านบิตที่1ถ้าค่าไม่ตรงไม่ต้องลงจุด
      if ((mc & 0x40) == 0x40) {
        mysetpoint(x + my_x, 15 - data + y, true);  // อ่านบิตที่2ถ้าค่าตรงให้ลงจุด
        my_x++;
      }
      else my_x++;
      if ((mc & 0x20) == 0x20) {
        mysetpoint(x + my_x, 15 - data + y, true);  // อ่านบิตที่3ถ้าค่าตรงให้ลงจุด
        my_x++;
      }
      else my_x++;
      if ((mc & 0x10) == 0x10) {
        mysetpoint(x + my_x, 15 - data + y, true);  // อ่านบิตที่4ถ้าค่าตรงให้ลงจุด
        my_x++;
      }
      else my_x++;
      if ((mc & 0x08) == 0x08) {
        mysetpoint(x + my_x, 15 - data + y, true);  // อ่านบิตที่5ถ้าค่าตรงให้ลงจุด
        my_x++;
      }
      else my_x++;
      if ((mc & 0x04) == 0x04) {
        mysetpoint(x + my_x, 15 - data + y, true);  // อ่านบิตที่6ถ้าค่าตรงให้ลงจุด
        my_x++;
      }
      else my_x++;
      if ((mc & 0x02) == 0x02) {
        mysetpoint(x + my_x, 15 - data + y, true);  // อ่านบิตที่7ถ้าค่าตรงให้ลงจุด
        my_x++;
      }
      else my_x++;
      if ((mc & 0x01) == 0x01) {
        mysetpoint(x + my_x, 15 - data + y, true);  // อ่านบิตที่8ถ้าค่าตรงให้ลงจุด
        my_x = 0;
      }
      else my_x = 0;
      data++;
    }
  }
}

int Check_Char_Width(int font) //ตรวจสอบความกว้างของอักษร
{
  unsigned char  c;
  int w = 0, j = 0;

  for (int i = 0; i < 16; i++) //ตรวจฟอนต์ 8x16 จำนวน 16 รอบ
  {
    c =  pgm_read_byte_near(&Matrix_16_Font[font][i]); //ตรวจสอบฟอนต์ในหน่วยความจำ
    if ((c & 0x80) == 0x80)w = 1; //ถ้าบิตที่1มีจุด
    if ((c & 0x40) == 0x40)w = 2; //ถ้าบิตที่2มีจุด
    if ((c & 0x20) == 0x20)w = 3;
    if ((c & 0x10) == 0x10)w = 4;
    if ((c & 0x08) == 0x08)w = 5;
    if ((c & 0x04) == 0x04)w = 6;
    if ((c & 0x02) == 0x02)w = 7;
    if ((c & 0x01) == 0x01)w = 8; //ถ้าบิตที่8มีจุด

    if (w >= j)j = w;
  }
  return j; //คืนค่าที่ตรวจสอบความกว้างของตัวอักษร
}

void DrawText(int my_row, int my_column, String text_Input) //พิมพ์ข้อความ
{
  String my_text;
  unsigned char char1, char2, char_out;
  int i = 0, x;
  int indx;

  my_text = text_Input;
  indx = 0;
  mx.clear();
  while (i < text_Input.length()) //ไล่ไปทีละอักษรจนหมด
  {
    char1 = my_text[i];
    if (char1 == 0xE0) // ตรวจสอบไบต์แรกถ้าเป็นภาษาไทย ถ้าไม่ใช่ให้ไป else ภาษาอังกฤษ
    {
      char1 = my_text[i + 1]; //ให้ char1 เป็นไบต์ที่สองแสดงอักษรไทยหรือสระ
      char2 = my_text[i + 2]; //ให้ char2 เป็นไบต์ที่สามซึ่งเป็นตำแหน่งอักษรไทย
      if (char1 == 0xB8 && (char2 + 32) >= 161 && (char2 + 32) <= 218) //เช็คว่าเป็นหมวด1 ก-พินธุ
      {
        if ((char2 + 32) >= 212 && (char2 + 32) <= 218) //เช็คว่าเป็นสระ อิ-อี-อึ-อื-อุ-อู-อฺ
        {
          char_out = char2 + 32;
          i = i + 3; //ข้ามภาษาไทยที่มี 3 ไบต์ไปอักษรต่อไป
          x = Check_Char_Width(char_out);
          indx = indx - x - 1;
        }
        else //อักษรไทยปรกติ
        {
          char_out = char2 + 32;
          if (char2 + 32 == 209) //เช็คว่าเป็นไม้หันอากาศ
          {
            x = Check_Char_Width(char_out);
            indx = indx - x - 1;
          }
          if (char_out == 211) //ถ้าเป็นสระ อำ
          {
            x = Check_Char_Width(char_out);
            indx = indx - x + 5;
          }
          i = i + 3; //ข้ามภาษาไทยที่มี 3 ไบต์ไปอักษรต่อไป
        }
      }
      if (char1 == 0xB9 && (char2 + 96) >= 224 && (char2 + 96) <= 251) //เช็คว่าเป็นหมวด2 สระเอ-ขอหมุด
      {
        if ((char2 + 96) >= 231 && (char2 + 96) <= 237) //เช็คว่าเป็นวรรยุกต์ ็-่-้-๊-๋-์-ํ
        {
          char_out = char2 + 96;
          i = i + 3; //ข้ามภาษาไทยที่มี 3 ไบต์ไปอักษรต่อไป
          x = Check_Char_Width(char_out);
          indx = indx - x - 1;
        }
        else //สระปรกติ เ แ โ
        {
          char_out = char2 + 96;
          i = i + 3; //ข้ามภาษาไทยที่มี 3 ไบต์ไปอักษรต่อไป

          if (char_out == 226 || char_out == 227 || char_out == 228) //ถ้าเป็นสระ โ ใ ไ
          {
            x = Check_Char_Width(char_out);
            indx = indx - x + 3;
          }
        }
      }
    }
    else  //อักษรภาษาอังกฤษ
    {
      char_out = my_text[i];
      i++; // ไปอักษรต่อไป
      if (char_out == 32)indx = indx + 8; //ถ้าเป็นเว้นวรรค
    }


    DrawChar(my_row + indx, my_column, char_out, state); //พิมพ์อักษรที่แปลง
    x = Check_Char_Width(char_out);
    indx = indx + x + gap_pixel;
  }
  //mp.update();
  mx.update();
}

int CheckTextWidth(String text_Input) //ตรวจขนาดของจุดแนวแกน x ทั้งหมดของข้อความ
{
  String my_text;
  unsigned char char1, char2, char_out;
  int i = 0, x;
  int indx;

  my_text = text_Input;
  indx = 0;
  mx.clear();
  while (i < text_Input.length()) //ไล่ไปทีละอักษรจนหมด
  {
    char1 = my_text[i];
    if (char1 == 0xE0) // ตรวจสอบไบต์แรกถ้าเป็นภาษาไทย ถ้าไม่ใช่ให้ไป else ภาษาอังกฤษ
    {
      char1 = my_text[i + 1]; //ให้ char1 เป็นไบต์ที่สองแสดงอักษรไทยหรือสระ
      char2 = my_text[i + 2]; //ให้ char2 เป็นไบต์ที่สามซึ่งเป็นตำแหน่งอักษรไทย
      if (char1 == 0xB8 && (char2 + 32) >= 161 && (char2 + 32) <= 218) //เช็คว่าเป็นหมวด1 ก-พินธุ
      {
        if ((char2 + 32) >= 212 && (char2 + 32) <= 218) //เช็คว่าเป็นสระ อิ-อี-อึ-อื-อุ-อู-อฺ
        {
          char_out = char2 + 32;
          i = i + 3; //ข้ามภาษาไทยที่มี 3 ไบต์ไปอักษรต่อไป
          x = Check_Char_Width(char_out);
          indx = indx - x - 1;
        }
        else //อักษรไทยปรกติ
        {
          char_out = char2 + 32;
          if (char2 + 32 == 209) //เช็คว่าเป็นไม้หันอากาศ
          {
            x = Check_Char_Width(char_out);
            indx = indx - x - 1;
          }
          if (char_out == 211) //ถ้าเป็นสระ อำ
          {
            x = Check_Char_Width(char_out);
            indx = indx - x + 5;
          }
          i = i + 3; //ข้ามภาษาไทยที่มี 3 ไบต์ไปอักษรต่อไป
        }
      }
      if (char1 == 0xB9 && (char2 + 96) >= 224 && (char2 + 96) <= 251) //เช็คว่าเป็นหมวด2 สระเอ-ขอหมุด
      {
        if ((char2 + 96) >= 231 && (char2 + 96) <= 237) //เช็คว่าเป็นวรรยุกต์ ็-่-้-๊-๋-์-ํ
        {
          char_out = char2 + 96;
          i = i + 3; //ข้ามภาษาไทยที่มี 3 ไบต์ไปอักษรต่อไป
          x = Check_Char_Width(char_out);
          indx = indx - x - 1;
        }
        else //สระปรกติ เ แ โ
        {
          char_out = char2 + 96;
          i = i + 3; //ข้ามภาษาไทยที่มี 3 ไบต์ไปอักษรต่อไป

          if (char_out == 226 || char_out == 227 || char_out == 228) //ถ้าเป็นสระ โ ใ ไ
          {
            x = Check_Char_Width(char_out);
            indx = indx - x + 3;
          }
        }
      }
    }
    else  //อักษรภาษาอังกฤษ
    {
      char_out = my_text[i];
      i++; // ไปอักษรต่อไป
      if (char_out == 32)indx = indx + 8; //ถ้าเป็นเว้นวรรค
    }
    x = Check_Char_Width(char_out);
    indx = indx + x + gap_pixel;
  }
  return indx; //คืนค่าที่ประมวลผลของจุดทั้งหมด
}

void ScrollText(int x, int y, String my_text, int d, char direct) //เลื่อนข้อความ
{
  int j;
  if (direct > 7)direct = 0;
  j = CheckTextWidth(my_text);

  if (direct == 0) //เดินหน้า
  {
    for (int i = Max_col; i > -j; i--)
    {
      DrawText(x + i, y, my_text);
      delay(d);
    }
  }
  if (direct == 1) //ถอยหลัง
  {
    for (int i = -j; i <= Max_col; i++)
    {
      DrawText(x + i, y, my_text);
      delay(d);
    }
  }
  if (direct == 2) //เลื่อนข้อความจากบนลงล่าง
  {
    for (int i = Max_row; i >= -Max_row; i--)
    {
      DrawText(x, y + i, my_text);
      delay(d);
      if (i == 0)delay(1000);
    }
    state = true;
  }
  if (direct == 3) //เลื่อนข้อความจากล่างขึ้นบน
  {
    for (int i = -Max_row; i <= Max_row; i++)
    {
      DrawText(x, y + i, my_text);
      delay(d);
      if (i == 0)delay(1000);
    }
    state = true;
  }
  if (direct == 4) //เฉียงขึ้นขวา
  {
    for (int i = -Max_row; i <= Max_row; i++)
    {
      DrawText(x + i, y + i, my_text);
      delay(d);
      if (i == 0)delay(1000);
    }
    state = true;
  }
  if (direct == 5) //เฉียงลงซ้าย
  {
    for (int i = Max_row; i >= -Max_row; i--)
    {
      DrawText(x + i, y + i, my_text);
      delay(d);
      if (i == 0)delay(1000);
    }
    state = true;
  }
  if (direct == 6) //เฉียงลงขวา
  {
    for (int i = Max_row; i >= -Max_row; i--)
    {
      DrawText(x - i, y + i, my_text);
      delay(d);
      if (i == 0)delay(1000);
    }
    state = true;
  }
  if (direct == 7) //เฉียงขึ้นซ้าย
  {
    for (int i = -Max_row; i <= Max_row; i++)
    {
      DrawText(x - i, y + i, my_text);
      delay(d);
      if (i == 0)delay(1000);
    }
    state = true;
  }
}

void setup()
{

  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 0);

  Serial.begin(9600);
  Serial.println("เริ่มต้นการทดสอบ");

  pinMode(SW, INPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(PIR, INPUT);
  pinMode(SW_Secure, INPUT);


  if (! clockk.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (clockk.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    clockk.adjust(DateTime(F(__DATE__), F(__TIME__)));

  }

  WiFi.begin(ssid, pass); //เชื่อมต่อ WiFi
  Blynk.config(auth, server, port); //กำหนด Token key , ชื่อ Server และ port
  Blynk.connect(); //เชื่อมต่อไปยัง Blynk
  

}

void loop()
{
  while (!insetupmode) {
    Blynk.run();
    runTime();
  }
  while (insetupmode) {
    if (!warningState)
    {
      Blynk.run();
      runText();
    }
    else {
      Blynk.run();
      runSecure();
    }
  }


}

void runTime() {
  DateTime now = clockk.now();

  Serial.println("Hour: " + String(text_Hour));
  Serial.println("Min: " + String(text_Min));
  Serial.println("Text: " + String(text_In));
  String timee ;

  if (now.hour() < 10 && now.minute() < 10 && now.second() < 10)
  {
    timee = "0" + (String)now.hour() + ":0" + (String)now.minute() + ":0" + (String)now.second();
  }
  else if (now.hour() < 10 && now.minute() < 10 && now.second() > 9)
  {
    timee = "0" + (String)now.hour() + ":0" + (String)now.minute() + ":" + (String)now.second();
  }
  else if (now.hour() < 10 && now.minute() > 9 && now.second() < 10)
  {
    timee = "0" + (String)now.hour() + ":" + (String)now.minute() + ":0" + (String)now.second();
  }
  else if (now.hour() > 9 && now.minute() < 10 && now.second() < 10)
  {
    timee = (String)now.hour() + ":0" + (String)now.minute() + ":0" + (String)now.second();
  }
  else if (now.hour() < 10 && now.minute() > 9 && now.second() > 9)
  {
    timee = "0" + (String)now.hour() + ":" + (String)now.minute() + ":" + (String)now.second();
  }
  else if (now.hour() > 9 && now.minute() < 10 && now.second() > 9)
  {
    timee = (String)now.hour() + ":0" + (String)now.minute() + ":" + (String)now.second();
  }
  else if (now.hour() > 9 && now.minute() > 9 && now.second() < 10)
  {
    timee = (String)now.hour() + ":" + (String)now.minute() + ":0" + (String)now.second();
  }
  else if (now.hour() > 9 && now.minute() > 9 && now.second() > 9)
  {
    timee = (String)now.hour() + ":" + (String)now.minute() + ":" + (String)now.second();
  }
  DrawText(9, 0, timee); delay(1000);

  if (now.hour() == text_Hour && now.minute() == text_Min && now.second() >= 3 && now.second() <= 23)
  {
    insetupmode = true;
    digitalWrite(Buzzer, HIGH);
  }
  else {
    insetupmode = false;
    digitalWrite(Buzzer, LOW);
  }

  if (digitalRead(SW) == LOW)
  {
    text_Min = -1;
    text_Hour = -1;
  }

  else if (digitalRead(SW) == HIGH)
  {
    text_Min = temp_Min;
    text_Hour = temp_Hour;
  }

  if (digitalRead(SW_Secure) == HIGH)
  {
    warningState = true;
    insetupmode = true;
  }
  else if (digitalRead(SW_Secure) == LOW)
  {
    warningState = false;
  }




  //  ScrollText(0,0,tx,20,0);delay(1000); //เลื่อนซ้าย
}

void runText()
{
  ScrollText(0, 0, text_In, 20, 0);
  DateTime now = clockk.now();
  if (now.hour() == text_Hour && now.minute() == text_Min && now.second() >= 3 && now.second() <= 23)
  {
    insetupmode = true;
    digitalWrite(Buzzer, HIGH);
  }
  else {
    insetupmode = false;
    digitalWrite(Buzzer, LOW);
  }
}

void runSecure()
{
  if (digitalRead(PIR) == HIGH)
  {
    countPIR += 1;
    timeInSc == false;
    digitalWrite(Buzzer, HIGH);
    ScrollText(0, 0, warningMessage, 20, 0);
  }
  else if (digitalRead(SW_Secure) == LOW) {
    countPIR = 0;
    digitalWrite(Buzzer, LOW);
    insetupmode = false;
    warningState = false;
  }
  else if (digitalRead(SW_Secure) == HIGH) {
    if (timeInSc == true && countPIR < 1) {
      runTime();
    }
    else if (countPIR >= 1) {
      digitalWrite(Buzzer, HIGH);
      ScrollText(0, 0, warningMessage, 20, 0);
    }
  }
}

BLYNK_CONNECTED() {  // ฟังก์ชันนี้ทำงานเมื่อต่อ Blynk ได้
  Serial.println("App Blynk ทำงาน!");
}

BLYNK_WRITE(V0) {
  text_Min = param.asInt();  // Text Input Widget - Int
  temp_Min = text_Min;
  Blynk.virtualWrite(V1, text_Min);
}

BLYNK_WRITE(V2) {
  text_Hour = param.asInt();  // Text Input Widget - Int
  temp_Hour = text_Hour;
  Blynk.virtualWrite(V3, text_Hour);
}

BLYNK_WRITE(V4) {
  text_In = param.asStr();  // Text Input Widget - Strings
  Blynk.virtualWrite(V5, text_In);
}
