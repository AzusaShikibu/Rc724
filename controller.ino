#include <WiFi.h>
#include <esp_now.h>
#include <PubSubClient.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <EEPROM.h>
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0,19,18,U8X8_PIN_NONE);
#define CH1 34//左X，👉0-4095
#define CH2 35//左Y，👆0-4095
#define CH3 32//右X，👉0-4095
#define CH4 33//右Y，👆0-4095
#define CH5 4//左摇杆按键
#define CH6 13//右摇杆按键
#define CH7 15//左边旋钮
#define CH8 22//右边钮子开关
#define CH9 23//龟仔开关
//Mqtt配置区👇👇👇
const char* ssid = "010";
const char* password = "20031201";
String topicString = "724UGCAR";//输入自己的特定主题名，能保证别人不和你重复就行
const char* mqttServer1 = "test.ranye-iot.net";
const char* mqttServer2 = "8.142.157.58";//这是我个人服务器，还有10天到期，大家也能将就用用
int port=1883;//MQTT服务器端口
int isFxfz=0;//如果舵机方向反了，就把它改成1
//Now配置区👇👇👇
uint8_t broadcastAddress[] = {0xA8, 0x48, 0xFA, 0xE2, 0x29, 0xA7};//此为8266接收机的Mac地址


//以下代码无需改动👇👇👇
int address0=0;
int Ym,Fx,Ym0,Fx0,Fire,isFire,FireFx,FireReady,isFirstMedian,isFirstMode,isRun,FxMedian,YmMedian;
int YmTrim,FxTrim;//方向微调
const char* Mode="Now"; 
const char* Server=mqttServer1;

int zoyvb
int zoxvb
//typedef struct struct_message {
//  int a;//油门
//  int b;//方向
//  char c;
//  int d;
//  int e;
//  int f=70;//发射器方向
//} struct_message;
//struct_message LeCar;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

 esp_now_peer_info_t peerInfo;



void setup() {
  Serial.begin(115200);
  pinMode(CH1, INPUT); 
  pinMode(CH2, INPUT); 
  pinMode(CH3, INPUT); 
  pinMode(CH4, INPUT); 
  pinMode(CH5, INPUT); 
  pinMode(CH6, INPUT); 
  pinMode(CH9, INPUT); 
  WiFi.mode(WIFI_STA);
     Now0();
  u8g2.begin();
  u8g2.enableUTF8Print(); 
  EEPROM.begin(4096);
  EEPROM.get(address0, YmMedian);
  EEPROM.get(address0+10, FxMedian);
  EEPROM.get(address0+20, YmTrim);
  EEPROM.get(address0+30, FxTrim);
  EEPROM.end();
  if(YmTrim<=0)
  {YmTrim=90;}
  if(FxTrim<=0)
  {FxTrim=90;}
  if(YmMedian<10)
  { int i=5;
    Mode="Median";
    while(i>=0)
    {u8g2.clearBuffer();
     u8g2.setFont(u8g2_font_wqy12_t_gb2312); 
     u8g2.drawUTF8(0, 15, "警告!");  
     u8g2.drawUTF8(5, 30, "摇杆中位数据缺失!");  
     u8g2.drawUTF8(5, 45, "即将进入中位校准模式");  
     u8g2.drawFrame(0, 17, 128, 47);
     u8g2.setCursor(50,60);
     u8g2.print(i);
     u8g2.drawUTF8(68, 60, "s");
     u8g2.sendBuffer();
     delay(1000);
     i--;
    }
    GetMedian();
  }
  beginMode();
}
void connectWifi(){
  WiFi.begin(ssid, password);
  int i=0;
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_unifont_t_symbols);
  u8g2.setFont(u8g2_font_wqy12_t_gb2312b); 
  u8g2.drawStr(0, 11, "Mode:");
  u8g2.drawStr(30, 11,Mode);
  u8g2.drawStr(10, 40, "ConnectWifi:");
  u8g2.setCursor(80,40);
  u8g2.print(i);
  u8g2.drawStr(100, 40, "s");
  u8g2.sendBuffer();
  Serial.print(".");
  i+=1;
  }
  Serial.println("");
  Serial.println("WiFi Connected!");  
  Serial.println(""); 
}


void connectMQTTServer(){
  // 根据ESP32的MAC地址生成客户端ID（避免与其它ESP32的客户端ID重名）
  String clientId = "esp32-" + 50014EE0ABD613C4;
  // 连接MQTT服务器
  if (mqttClient.connect(clientId.c_str())) { 
    Serial.println("MQTT Server Connected.");
    Serial.println("Server Address: ");
    Serial.println(Server);
    Serial.println("ClientId:");
    Serial.println(clientId);
  } else {
    Serial.print("MQTT Server Connect Failed. Client State:");
    Serial.println(mqttClient.state());
    ShowFail();
    u8g2.sendBuffer();
    delay(3000);
  }   
}
 void pubMQTTmsg(){
  static int value; // 客户端发布信息用数字
  char publishTopic[topicString.length() + 1];   // 这么做是为确保不同用户进行MQTT信息发布时，ESP8266客户端名称各不相同，
  strcpy(publishTopic, topicString.c_str());
  Ym0 = analogRead(CH2); //读取油门信息
  Fx0 = analogRead(CH3); //读取方向信息
if (Ym0 > YmMedian + 100)
   {Ym = map(Ym0,YmMedian+100, 4095, YmTrim, 180);}
   else if(Ym0 < YmMedian - 100)
   {Ym =map(Ym0,0, YmMedian-100,0,YmTrim);}
   else if (YmMedian-100<Ym0<YmMedian+100)
    Ym=YmTrim;
 if(Fx0 > FxMedian + 100)
   {if(isFxfz)
   Fx = map(Fx0,FxMedian+100, 4095, FxTrim, 180);
   else Fx = map(Fx0,FxMedian+100, 4095,FxTrim,0);}
   else if(Fx0 < FxMedian - 100)
   {if(isFxfz)
   Fx =map(Fx0,0,FxMedian-100,0,FxTrim);
   else Fx = map(Fx0,0,FxMedian-100,180,FxTrim);
    }
   else if (FxMedian-100<Fx0<FxMedian+100)
    {Fx=FxTrim;} 
  // 建立发布信息。信息内容以Hello World为起始，后面添加发布次数。
  String messageString = "A"+String(Ym)+"B"+String(Fx)+"C"; 
  char publishMsg[messageString.length() + 1];   
  strcpy(publishMsg, messageString.c_str());
  // 实现ESP32向主题发布信息
  if(mqttClient.publish(publishTopic, publishMsg)){
  //  Serial.println(publishMsg); 
  ShowOk();
  } else {
    ShowFail();
  }
    u8g2.sendBuffer();
}

void beginMode(){
  Mode="Now";
  delay(500);
 while(1)
 {ShowMode();
  u8g2.drawUTF8(10, 25, "N/A");
  u8g2.drawUTF8(10, 38, "MQTT");
  u8g2.drawUTF8(10, 51, "摇杆调整");
  u8g2.setFont(u8g2_font_adventurer_t_all);
  if(Mode=="Now")
    {u8g2.drawGlyph(0,26,0x25ba);
     if(analogRead(CH2)<1000)
       {Mode="MQTT";}
    }
  else if(Mode=="MQTT")
  { u8g2.drawGlyph(0,39,0x25ba);
    if(analogRead(CH2)>3000)
      {Mode="Now";}
    else if(analogRead(CH2)<1000)
      {Mode="Median";}
  }
  else if(Mode=="Median")
  {u8g2.drawGlyph(0,52,0x25ba);
   if(analogRead(CH2)>3000)
      {Mode="MQTT";}
  }
  delay(30);
  u8g2.sendBuffer();
  if(digitalRead(CH5)==0)
    {pdMode();break;}
 // u8g2.sendBuffer();
 }
}

void pdMode()
{
  if(Mode=="MQTT")
  {  
  
  isRun=1;
  ShowSetServer();
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &LeCar, sizeof(LeCar));
  connectWifi();
  mqttClient.setServer(Server, port);
  connectMQTTServer();}
else if(Mode=="Median")
{ShowMedian();}}


void loop() {
  if(Mode=="MQTT" && isRun==1)
  {//tickerCount(); 
 if (mqttClient.connected()) { // 如果开发板成功连接服务器    
      pubMQTTmsg();
    mqttClient.loop();          // 保持客户端心跳
  } else {                  // 如果开发板未能成功连接服务器
    connectMQTTServer();    // 则尝试连接服务器
  }
  }
 else if(Mode=="Now" && isRun==1)
 {
     Now1();
  }
  delay(100);
}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if(status == ESP_NOW_SEND_SUCCESS && Mode=="Now")
  ShowOk();
  else if(status != ESP_NOW_SEND_SUCCESS && Mode=="Now")
  {ShowFail();    }
  u8g2.sendBuffer();
}

unsigned long currentTime0,currentTime1;
void Now1(){
  Ym0 = analogRead(CH2); //读取油门信息
  Fx0 = analogRead(CH3); //读取方向信息
  Fire = digitalRead(CH6);
  FireFx = analogRead(CH4);
  FireReady=digitalRead(CH5);
   if (Ym0 > YmMedian + 100)
   { zoyvb = map(Ym0,YmMedian+100, 4095, YmTrim, 180);}
   else if(Ym0 < YmMedian - 100)
   {zoyvb =map(Ym0,0, YmMedian-100,0,YmTrim);}
   else if (YmMedian-100<Ym0<YmMedian+100)
   zoyvb=YmTrim;
   if(Fx0 > FxMedian + 100)
   {if(isFxfz)
   zoxvb = map(Fx0,FxMedian+100, 4095, FxTrim, 180);
   else zoxvb = map(Fx0,FxMedian+100, 4095,FxTrim,0);}
   else if(Fx0 < FxMedian - 100)
   {if(isFxfz)
   zoxvb =map(Fx0,0,FxMedian-100,0,FxTrim);
   else zoxvb = map(Fx0,0,FxMedian-100,180,FxTrim);
    }
   else if (FxMedian-100<Fx0<FxMedian+100)
    {zoxvb=FxTrim;} 
    if(FireFx>2500)
    {LeCar.f+=1;}
    else if(FireFx<1200)
    {LeCar.f-=1;}
    if(FireReady==0)
    {LeCar.e=2;}
    else {currentTime1=millis();
    if(currentTime1-currentTime0>=2000)
    {isFire=0;
    //Serial.println("Fire is Ready");
    }
   if(Fire==0 && isFire==0)
   {LeCar.e=1;
    currentTime0=millis();
    isFire=1;
    Serial.println("Fire is Ok");}
   else {LeCar.e=0;}
   }
    Ym=zoyvb;Fx=zoxvb;
  //  Serial.println(LeCar.e);
esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &LeCar, sizeof(LeCar));
}

void ShowMedian()
{ Mode="Median1";
delay(500);
  while(1)
    {  ShowMode();
  u8g2.drawUTF8(10, 26, "中位校准");
  u8g2.drawUTF8(10, 39, "中位微调");
  u8g2.drawUTF8(10, 52, "EEPROM查看");
  u8g2.setFont(u8g2_font_adventurer_t_all);
   if(Mode=="Median1")
  {
    u8g2.drawGlyph(0,27,0x25ba);
    LeCar.c='N';
    if(analogRead(CH2)<1000)
      {Mode="Median2";}
  }
   else if(Mode=="Median2")
  {
   u8g2.drawGlyph(0,40,0x25ba);
    LeCar.c='M';
    if(analogRead(CH2)>3000)
    {Mode="Median1";}
    else if(analogRead(CH2)<1000)
    {Mode="Median3";}
  }
  else if(Mode=="Median3")
  {u8g2.drawGlyph(0,53,0x25ba);
   if(analogRead(CH2)>3000)
     {Mode="Median2";}
   }
  u8g2.sendBuffer();
  delay(30);
if(digitalRead(CH5)==0)
    {
    if(Mode=="Median1")
     {GetMedian();}
    else if(Mode=="Median2")
     {ShowMedian2();}
    else if(Mode=="Median3")
     {ShowMedian3();}
     break;}
    if(digitalRead(CH6)==0)
    {beginMode();break;}
  }
}
void ShowMedian2()
{delay(500);
  EEPROM.begin(4096);
  EEPROM.get(address0+20, YmTrim);
  EEPROM.get(address0+30, FxTrim);
  EEPROM.end();
  while(1)
 {int isTrim=digitalRead(CH9);
  if(isTrim==0)
   { 
    Ym = analogRead(CH2); 
    Fx = analogRead(CH3); 
    if(Ym<YmMedian-500)
      {YmTrim-=1;}
    else if(Ym>YmMedian+500)
    {YmTrim+=1;}
    if(Fx<FxMedian-500)
      {FxTrim-=1;}
    else if(Fx>FxMedian+500)
      {FxTrim+=1;}}
      Ym=YmTrim;
      Fx=FxTrim;
  Show0();
  u8g2.sendBuffer();
   if(digitalRead(CH5)==0)
        {ShowMedian2_1();break;}
   if(digitalRead(CH6)==0)
        {ShowMedian();break;}
 }
 
}
void ShowMedian2_1()
{
  ShowMode();
  u8g2.drawUTF8(5, 30, "End   已写入EEPROM!");  
  u8g2.drawUTF8(10, 45, "->YmTrim =");  
     u8g2.drawFrame(0, 17, 128, 47);
     u8g2.setCursor(85,45);
     u8g2.print(YmTrim); 
     u8g2.drawUTF8(10, 60, "->FxTrim ="); 
     u8g2.setCursor(85,60);
     u8g2.print(FxTrim); 
     u8g2.sendBuffer();
     EEPROM.begin(4096);
     EEPROM.put(address0+20,Ym);  
     EEPROM.put(address0+30,Fx);    
     EEPROM.end();
     while(1)
     {  if(digitalRead(CH6)==0)
        {ShowMedian();break;}}
}
int page=1;
void ShowMedian3()
{
  EEPROM.begin(4096);
  EEPROM.get(address0, YmMedian);
  EEPROM.get(address0+10, FxMedian);
  EEPROM.get(address0+20, YmTrim);
  EEPROM.get(address0+30, FxTrim);
  EEPROM.end();
  ShowMode();
  u8g2.drawFrame(0, 17, 128, 47);
  u8g2.drawUTF8(5, 26, "address");
  u8g2.drawUTF8(65, 26, "data");
  if(page==1)
 { u8g2.setCursor(5, 40);
  u8g2.print(address0);
  u8g2.setCursor(65, 40);
  u8g2.print(YmMedian);
  u8g2.setCursor(5, 53);
  u8g2.print(address0+10);
  u8g2.setCursor(65, 53);
  u8g2.print(FxMedian);
 }else if(page==2)
 { u8g2.setCursor(5, 40);
  u8g2.print(address0+20);
  u8g2.setCursor(65, 40);
  u8g2.print(YmTrim);
  u8g2.setCursor(5, 53);
  u8g2.print(address0+30);
  u8g2.setCursor(65, 53);
  u8g2.print(FxTrim);}
  u8g2.sendBuffer();
  while(1)
    {if(analogRead(CH1)>3000 && page==1)
    {page=2;ShowMedian3();break;}
    if(analogRead(CH1)<1000 && page==2)
    {page=1;ShowMedian3();break;}
     if(digitalRead(CH6)==0)
        {ShowMedian();break;}
    }
}


void Now0()
{
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
  }
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  //通道
  peerInfo.encrypt = false;//是否加密为False
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
  }
}
void ShowVbat()
{
  float V_BAT = 8.2*10; //电池电压
  int val = map(V_BAT, 60, 84, 0, 100); //电压映射
  u8g2.drawRFrame(110, 1, 17, 11, 3); //绘制空心圆角矩形
  u8g2.setCursor(100, 11);
  u8g2.print("%");
  u8g2.setCursor(85, 11);
  u8g2.print(val); //显示电量百分比
 int pixelnum = map(V_BAT, 60, 84, 0, 15); //电压映射
  if (pixelnum < 1)
  {pixelnum = 0;}
  else if (pixelnum > 14)
  {pixelnum = 15;}
  int i;
  for (i = 0; i < pixelnum; i++)
  {u8g2.drawVLine(126 - i, 1, 11);}
}
void ShowMode()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_wqy12_t_gb2312b); 
  u8g2.drawStr(0, 11, "Mode:");
  u8g2.drawStr(30, 11,Mode);
}
void Show0()
{
  ShowMode();
  u8g2.setCursor(0, 35);
  u8g2.print("Ym：");
  u8g2.setCursor(45, 35);
  u8g2.print(Ym); 
  u8g2.setCursor(0, 51);
  u8g2.print("Fx：");
  u8g2.setCursor(45, 51);
  u8g2.print(Fx); 
}
void ShowOk()
{
  Show0();
  ShowVbat();
  u8g2.drawStr(80,42, "OK");
}
void ShowFail()
{
  Show0();
  ShowVbat();
  u8g2.drawStr(80,42, "Fail");
}
void ShowSetServer()
{  const char* Select0="No";
delay(500);
 do{  
  ShowMode();
  ShowVbat();
  u8g2.drawStr(10,33,mqttServer1);
  u8g2.drawStr(10,55,"My Mqtt Server");
  if(Server==mqttServer1)
  {
    u8g2.drawFrame(0, 22, 128, 15);
    if(analogRead(CH2)<1200)
      {Server=mqttServer2;
      LeCar.d=2;
      }
    }
   else if( Server==mqttServer2)
  {
    u8g2.drawFrame(0, 44, 128, 15);
    if(analogRead(CH2)>2800)
    {Server=mqttServer1;
    LeCar.d=1;}
    }
    if(digitalRead(CH5)==0)
    {
     Select0="Ok";}
  u8g2.sendBuffer();
 }
  while(Select0!="Ok");
}

void GetMedian()
{ 
  int jccs=10000,YmMax=0,YmMin=4095,FxMax=0,FxMin=4095,Yml,Fxl;
    u8g2.clearBuffer();
    ShowMode();
    u8g2.drawUTF8(5, 30, "校准次数：10000"); 
    u8g2.drawUTF8(5, 50, "3秒后开始校准,请等待!"); 
    u8g2.sendBuffer();
    delay(3000);
  for(int i=0;i<jccs;i++)
   {Ym0 = analogRead(CH2); //读取油门值
    Fx0 = analogRead(CH3); //读取方向值
    //Serial.print(i);Serial.print("   ");Serial.print(Ym0);Serial.print(";");Serial.println(Fx0);
    if(Ym0>YmMax)
    {YmMax=Ym0;
    Yml=i;}
    else if(Ym0<YmMin)
    {YmMin=Ym0;}
    if(Fx0>FxMax)
    {FxMax=Fx0;
    Fxl=i;}
    else if(Fx0<FxMin)
    {FxMin=Fx0;}
    Ym=Ym+Ym0;
    Fx=Fx+Fx0;
    }
    
    delay(100);
   Serial.print("i=");Serial.print(Yml);Serial.print(";YmMax=");Serial.print(YmMax);Serial.print(";YmMin=");Serial.println(YmMin);
     delay(100);Serial.print("i=");Serial.print(Fxl);Serial.print(";FxMax=");Serial.print(FxMax);Serial.print(";FxMin=");Serial.println(FxMin);
    
    Ym=Ym/jccs;
    Fx=Fx/jccs;
    
     ShowMode();
    // u8g2.setFont(u8g2_font_wqy12_t_gb2312); 
     u8g2.drawUTF8(5, 30, "End   已写入EEPROM!");  
     u8g2.drawUTF8(10, 45, "->YmMedian =");  
     u8g2.drawFrame(0, 17, 128, 47);
     u8g2.setCursor(85,45);
     u8g2.print(Ym); 
     u8g2.drawUTF8(10, 60, "->FxMedian ="); 
     u8g2.setCursor(85,60);
     u8g2.print(Fx); 
     u8g2.sendBuffer();
     EEPROM.begin(4096);
     EEPROM.put(address0,Ym);  
     EEPROM.put(address0+10,Fx);    
     EEPROM.end();
     while(1)
     {
       if(digitalRead(CH6)==0)
        {ShowMedian();break;}
     }
}
