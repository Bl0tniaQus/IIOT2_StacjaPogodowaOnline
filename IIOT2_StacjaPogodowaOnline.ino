#include "M5Core2.h"
#include "M5_ENV.h"
#include "math.h"
#include "WiFi.h"
#include "time.h"
#include "PubSubClient.h"
SHT3X sht30;
QMP6988 qmp6988;
WiFiClient espClient;
PubSubClient client(espClient);
RTC_TimeTypeDef RTCTime;
RTC_DateTypeDef RTCDate;
const char* ssid = "tajne";
const char* password = "tajne";
const char* mqtt_server = "broker.mqttdashboard.com";
const char* owmkey = "";
const char* ntp_server = "pool.ntp.org";
const long  gmtOffset_sec = 3600; //gmt+1 -> 3600
const int   daylightOffset_sec = 3600;
int menu_stan=1; // Display page
int pomiary_stan=0;
char disp_refresh=1; // Display refresh
int drawScreen = 1;
long timer1,timer2,timer3 = 0;
int timerLimit1 = 5000,timerLimit2 = 30000;
int temp_UB,temp_LB;
int hum_UB,hum_LB,pres_UB,pres_LB;
int brightness;
int prevBound;
int focus;
bool error;
bool reset_confirm;
bool is_connected;
bool hour_passed = false;
bool fifteen_minutes_passed = false;
bool connection_tested = false;
bool measurements_written = false;
//tablice z danymi historycznymi
int n_hours = 0;
int n_temps = 0;
int n_pressures = 0;
int n_humidities = 0;
int pres1=0,humD1=0,humN1=0;
int pres2=0,humD2=0,humN2=0;
float tempD1 = 0, tempD2 = 0, tempN1 = 0, tempN2 = 0;
float atempD1=0,atempD2=0,atempD3=0,atempN1=0,atempN2=0,atempN3=0;
int ahumD1=0,ahumD2=0,ahumD3=0,ahumN1=0,ahumN2=0,ahumN3=0;
int apresDN1=0,apresDN2=0,apresDN3=0;
float* temps; //temperatury z trzech ostatnich dni
int* pressures; //ciśnienia z trzech ostatnich dni
int* humidities; //wilgotności z trzech ostatnich dni
int* hours; //godziny pomiarów
char buf [100];
void setupWifi();
void mqttConnect();
void publishMeasurements();
int getPressure();
float getTemperature();
int getHumidity();
void readSystemDate();
void readSystemTime();
bool checkDate(int y, int m, int d);
void setDate(int y, int m, int d);
void setTime(int h, int m, int s);
int getYear();
int getMonth();
int getDay();
int getHours();
int getMinutes();
int getSeconds();
void showBatteryLevelAndNetworkStatus();
void readHours();
void readTemps();
void readPressures();
void readHumidities();
bool checkFiles();
void writeMeasurements();
void writeBrightness(int val);
void writeFocus(char val);
void readBrightness();
void readFocus();
void writeTLB(int val);
void writeTUB(int val);
void writePLB(int val);
void writePUB(int val);
void writeHLB(int val);
void writeHUB(int val);
void readBounds();
void factoryReset();
void forecast();
void syncTime();
int getY(int val, int minval, int unit, int ymin);
//ekrany główne
void screen1(); //data i czas
void screen2(); //temperatura
void screen3(); //ciśnienie
void screen4(); //wilgotność
//ekrany menu głównego
void screen5(); //menu główne powrót
void screen6(); //menu główne statystyki
void screen7(); //menu główne ustawienia
void screen8(); //menu główne prognoza
void screen52();//menu główne pomoc
//ekrany menu ustawień
void screen9(); //menu ustawien powrót
void screen10(); //menu ustawien data
void screen11(); //menu ustawien czas
void screen12(); //menu ustawien progi
void screen45(); //menu ustawien jasnosc
void screen51(); //menu ustawien tryb skupienia
void screen49(); //menu ustawien reset
//ekran ustawienia jasnosci
void screen50();
//ekrany ustawień progów
void screen13(); //menu ustawien progów powrót
void screen14(); //temp lb
void screen15(); //temp ub
void screen16(); //pres lb
void screen17(); //pres ub
void screen18(); //hum lb
void screen19(); //hum ub
//ekrany edycji progów
void screen20(); //temp lb
void screen21(); //temp ub
void screen22(); //pres lb
void screen23(); //pres ub
void screen24(); //hum lb
void screen25(); //hum ub
//ekrany ustawień daty
void screen26(); //menu ustawień daty powrót
void screen27(); //dzien
void screen28(); //miesiac
void screen29(); //rok
//ekrany edycji daty
void screen30(); //ustaw dzien
void screen31(); //ustaw miesiac
void screen32(); //ustaw rok
//ekrany ustawień czasu
void screen33(); //menu ustawien czasu powrót
void screen34(); //godziny
void screen35(); //minuty
//ekrany edycji czasu
void screen36(); //ustaw godziny
void screen37(); //ustaw minuty
//ekrany menu statystyk
void screen38(); //menu statystyk powrót
void screen39(); //temperatura stat 
void screen40(); //temperatura wykres
void screen41(); //ciśnienie stat 
void screen42(); //ciśnienie wykres
void screen43(); //ciśnienie wykres
void screen44(); //wilgotność wykres
//prognoza
void screen46(); //prognoza return
void screen47(); //prognoza dziś
void screen48(); //prognoza jutro

void setup() {
  // put your setup code here, to run once:
  Wire.begin(); // Wire init, adding the I2C bus.
  M5.begin();
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.setTextSize(3);
  M5.Lcd.drawString("Loading...",160,120,2);
  M5.Axp.SetSpkEnable(0);
  client.setServer(mqtt_server, 1883);
  is_connected = false;
  setupWifi();
  if (is_connected) {syncTime();}
  SD.begin();
  qmp6988.init();
  readBrightness();
  readFocus();
  M5.Axp.SetLcdVoltage(2500 + 100*brightness);
  M5.Axp.SetSpkEnable(0);
  readBounds();
  temps = new float[0];
  pressures = new int[0];
  humidities = new int[0];
  hours = new int[0];
  timer1 = millis();
  timer3 = millis();
}
//prognoza, komentarze
void loop() {
  M5.update();
  
 if (menu_stan!=20&&menu_stan!=21&&menu_stan!=22&&menu_stan!=23&&menu_stan!=24&&menu_stan!=25
 &&menu_stan!=30&&menu_stan!=31&&menu_stan!=32&&menu_stan!=36&&menu_stan!=37&&menu_stan!=49&&menu_stan!=50) {error = false;}
 if (menu_stan!=49) {reset_confirm = false;}
  if (getMinutes()==0&&!measurements_written) {hour_passed=true; measurements_written = false;}
  else {measurements_written = true;}
  if (getMinutes()+1%15==0&&!connection_tested) {fifteen_minutes_passed=true; connection_tested = false;}
  else {connection_tested = true;}
  if (fifteen_minutes_passed)
  {
    if (!is_connected) {setupWifi();}
    if (!client.connected()) { mqttConnect(); }
    fifteen_minutes_passed=false;
    connection_tested = true;
    drawScreen++;
  }
  if (hour_passed)
  {
    writeMeasurements();
    hour_passed=false;
    measurements_written = true;
    if (is_connected)
    {
      publishMeasurements();
      syncTime();
    }
    drawScreen++;
  }

  
  
  switch(menu_stan)
  {
  case 1:screen1();break;
  case 2:screen2();break;
  case 3:screen3();break;
  case 4:screen4();break;
  case 5:screen5();break;
  case 6:screen6();break;
  case 7:screen7();break;
  case 8:screen8();break;
  case 9:screen9();break;
  case 10:screen10();break;
  case 11:screen11();break;
  case 12:screen12();break;
  case 13:screen13();break;
  case 14:screen14();break;
  case 15:screen15();break;
  case 16:screen16();break;
  case 17:screen17();break;
  case 18:screen18();break;
  case 19:screen19();break;
  case 20:screen20();break;
  case 21:screen21();break;
  case 22:screen22();break;
  case 23:screen23();break;
  case 24:screen24();break;
  case 25:screen25();break;
  case 26:screen26();break;
  case 27:screen27();break;
  case 28:screen28();break;
  case 29:screen29();break;
  case 30:screen30();break;
  case 31:screen31();break;
  case 32:screen32();break;
  case 33:screen33();break;
  case 34:screen34();break;
  case 35:screen35();break;
  case 36:screen36();break;
  case 37:screen37();break;
  case 38:screen38();break;
  case 39:screen39();break;
  case 40:screen40();break;
  case 41:screen41();break;
  case 42:screen42();break;
  case 43:screen43();break;
  case 44:screen44();break;
  case 45:screen45();break;
  case 46:screen46();break;
  case 47:screen47();break;
  case 48:screen48();break;
  case 49:screen49();break;
  case 50:screen50();break;
  case 51:screen51();break;
  case 52:screen52();break;
  }
}
void setupWifi() {
  
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
   M5.Lcd.drawString("Trying to connect...",160,0,2);
  WiFi.mode(WIFI_STA); // Set the mode to WiFi station mode.
  WiFi.begin(ssid, password); // Start Wifi connection.
  short tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries<1000) {
    delay(10);
    tries++;    
  }
  is_connected = WiFi.status() == WL_CONNECTED;
}
void mqttConnect()
{
  
  int tries = 0;
  while (!client.connected() && tries<1000) {
      
    // Próba nawiązania połączenia
    if (client.connect("M5PIR")) {
    client.subscribe("pir/test/xd/#"); // ponowna subskrypcja tematu
    client.setBufferSize(4096);
    } else {
      delay(10);
      tries++;  }
  } 
}
void publishMeasurements()
{
  if (!client.connected()) {return;}
  char jsonStr [2048];
  float temp = getTemperature();
  int pres = getPressure();
  int hum = getHumidity();
  readTemps();
  readHours();
  readPressures();
  readHumidities();
  String tempsBuf = "\"temps\":[";
  String hoursBuf = "\"hours\":[";
  String pressuresBuf = "\"pressures\":[";
  String humsBuf = "\"hums\":[";
  if ((n_temps==n_hours)&&(n_hours==n_pressures)&&(n_hours==n_humidities))
    {
     for (int i=0;i<n_temps;i++)
      {
          tempsBuf += String(temps[i],1);
          hoursBuf += String(hours[i]);
          pressuresBuf += String(pressures[i]);
          humsBuf += String(humidities[i]);
          if (i!=n_temps-1)
          {
              tempsBuf += ",";
              hoursBuf += ",";
              pressuresBuf += ",";
              humsBuf += ",";
          }
          
      } 
    }
      tempsBuf += "]";
      hoursBuf += "]";
      pressuresBuf += "]";
      humsBuf += "]";
  sprintf(jsonStr, "{\"temp_LB\":%d,\"temp\":%.1f,\"temp_UB\":%d,\"pres_LB\":%d,\"pres\":%d,\"pres_UB\":%d,\"hum_LB\":%d,\"hum\":%d,\"hum_UB\":%d,",
  temp_LB,temp,temp_UB,pres_LB,pres,pres_UB,hum_LB,hum,hum_UB);
  String message = String(jsonStr) + hoursBuf + "," + tempsBuf + "," + pressuresBuf + "," + humsBuf + "}";
  client.publish("pir/test/xd", message.c_str());

}
float getTemperature()
{
  if(sht30.get()==0){
    float temp = sht30.cTemp;
    char buf[7];
    sprintf(buf,"%.1f",temp);
    temp = atof(buf); return temp;
    }else{return 0;}
}
int getPressure()
{
  int pressure = (int)(round(qmp6988.calcPressure()/100));
  return pressure;
}
int getHumidity()
{
  if(sht30.get()==0){
    int humidity = (int)(round(sht30.humidity));
    return sht30.humidity;
    }else{return 0;}
}
void readSystemDate() { M5.Rtc.GetDate(&RTCDate); }
void readSystemTime() { M5.Rtc.GetTime(&RTCTime); }

bool checkDate(int y, int m, int d)
{
  if (!(m>=1&&m<=12) || !(y>=1&&y<=9999) || !(d>=1&&d<=31)) return false;
  bool leap = (y % 400 == 0) || (y % 4 == 0 && y % 100 !=0);

  if (m==2)
  {
    if (leap) { if (!(d<=29)) return false; }
    else {if (!(d<=28)) return false;}
  }

  if (m==4 || m==6 || m==9 || m==1) {if (!(d<=30)) return false;}
  return true;
}
  

void setDate(int y, int m, int d) {
  RTCDate.Year = y;  //Set the date.  设置日期
  RTCDate.Month = m;
  RTCDate.Date = d;
  M5.Rtc.SetDate(&RTCDate);}
void setTime(int h, int m, int s)
{
  RTCTime.Hours = h; //Set the time.  设置时间
  RTCTime.Minutes = m;
  RTCTime.Seconds = s;
  M5.Rtc.SetTime(&RTCTime); //and writes the set time to the real time clock.  并将设置的时间写入实时时钟
}
int getYear() {readSystemDate();return RTCDate.Year;}
int getMonth() {readSystemDate();return RTCDate.Month;}
int getDay() {readSystemDate();return RTCDate.Date;}
int getHours() {readSystemTime();return RTCTime.Hours;}
int getMinutes() {readSystemTime();return RTCTime.Minutes;}
int getSeconds() {readSystemTime();return RTCTime.Seconds;}
void showBatteryLevelAndNetworkStatus() 
{  
  double voltage = M5.Axp.GetBatVoltage();
  double percentage;
  if (voltage < 3.2) {percentage = 0;}
  else {
    percentage = (voltage - 3.2) * 100;
    }
  char battery_buf[10];
  if (is_connected) {sprintf(battery_buf,"C|%d%%",(int)round(percentage));}
  else {sprintf(battery_buf,"DC|%d%%",(int)round(percentage));}
  
  M5.Lcd.setTextColor(WHITE,BLUE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextDatum(TR_DATUM);
  M5.Lcd.drawString(battery_buf,320,0,1);
}
void readHours()
{
    char number[2];
    int digit = 0;
    n_hours = 0;
  File f = SD.open("/hours.txt", FILE_READ);
  if (f) {
      char znak;
      while (f.available()){
      znak = f.read();
      if (znak!='\n'&&znak!='\0')
      {
        number[digit] = (char)znak;
        digit++;
      }
      else 
        {
           digit=0;
           n_hours++;
           int *hours_new = new int[n_hours];
           int hour = atoi(number);
           if (n_hours==1) {hours_new[0] = hour;}
            else
            {
            for (int i=0;i<n_hours-1;i++)
            {
                hours_new[i] = hours[i];
            }
            hours_new[n_hours-1] = hour;
            }
            delete [] hours;
            hours = hours_new;
            memset(number,0,sizeof(number));
        }    
      }
    f.close();
  } else {
    return;
  }
}
void readTemps()
{
    char number[7];
    int digit = 0;
    n_temps = 0;
  File f = SD.open("/temperature.txt", FILE_READ);
  if (f) {
      char znak;
      while (f.available()){
      znak = f.read();
      if (znak!='\n'&&znak!='\0')
      {
        number[digit] = (char)znak;
        digit++;
      }
      else 
        {
           digit=0;
           n_temps++;
           float *temps_new = new float[n_temps];
           float temp = atof(number);
           if (n_temps==1) {temps_new[0] = temp;}
            else
            {
            for (int i=0;i<n_temps-1;i++)
            {
                temps_new[i] = temps[i];
            }
            temps_new[n_temps-1] = temp;
            }
            delete [] temps;
            temps = temps_new;
            memset(number,0,sizeof(number));
        }    
      }
    f.close();
  } else {
    return;
  }
}
void readPressures()
{
    char number[4];
    int digit = 0;
    n_pressures = 0;
  File f = SD.open("/pressure.txt", FILE_READ);
  if (f) {
      char znak;
      while (f.available()){
      znak = f.read();
      if (znak!='\n'&&znak!='\0')
      {
        number[digit] = (char)znak;
        digit++;
      }
      else 
        {
           digit=0;
           n_pressures++;
           int *pressures_new = new int[n_pressures];
           int pressure = atoi(number);
           if (n_pressures==1) {pressures_new[0] = pressure;}
            else
            {
            for (int i=0;i<n_pressures-1;i++)
            {
                pressures_new[i] = pressures[i];
            }
            pressures_new[n_pressures-1] = pressure;
            }
            delete [] pressures;
            pressures = pressures_new;
            memset(number,0,sizeof(number));
        }    
      }
    f.close();
  } else {
    return;
  }
}
void readHumidities()
{
    char number[4];
    int digit = 0;
    n_humidities = 0;
  File f = SD.open("/humidity.txt", FILE_READ);
  if (f) {
      char znak;
      while (f.available()){
      znak = f.read();
      if (znak!='\n'&&znak!='\0')
      {
        number[digit] = (char)znak;
        digit++;
      }
      else 
        {
           digit=0;
           n_humidities++;
           int *humidities_new = new int[n_humidities];
           int humidity = atoi(number);
           if (n_humidities==1) {humidities_new[0] = humidity;}
            else
            {
            for (int i=0;i<n_humidities-1;i++)
            {
                humidities_new[i] = humidities[i];
            }
            humidities_new[n_humidities-1] = humidity;
            }
            delete [] humidities;
            humidities = humidities_new;
            memset(number,0,sizeof(number));
        }    
      }
    f.close();
  } else {
    return;
  }
}
void writeMeasurements()
{
  readHours(); readHumidities(); readTemps(); readPressures();
    int n = n_hours;
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Writing...",160,0,2);
  if ((n_temps!=n)||(n_humidities!=n)||(n_pressures!=n)||(n>72))
  {
  SD.remove("/hours.txt"); 
  SD.remove("/temperature.txt");
  SD.remove("/pressure.txt");
  SD.remove("/humidity.txt");
  delete [] pressures; delete [] hours; delete [] temps; delete [] humidities;
  n_hours = 0; n_temps = 0; n_pressures = 0; n_humidities = 0; n = 0;
  pressures = new int [0]; hours = new int [0]; temps = new float [0]; humidities = new int [0];
  }
  float temp = getTemperature();
  int hum = getHumidity();
  int pres = getPressure();
  int hour = getHours();
  if (n<72)
  {
    char buf[10];
 
    File hoursFile = SD.open("/hours.txt",FILE_APPEND);
    sprintf(buf,"%d",hour); hoursFile.println(buf); memset(buf,0,sizeof(buf));
    File tempFile = SD.open("/temperature.txt",FILE_APPEND);
    sprintf(buf,"%.1f",temp); tempFile.println(buf); memset(buf,0,sizeof(buf));
    File presFile = SD.open("/pressure.txt",FILE_APPEND);
    sprintf(buf,"%d",pres); presFile.println(buf); memset(buf,0,sizeof(buf));
    File humFile = SD.open("/humidity.txt",FILE_APPEND);
    sprintf(buf,"%d",hum); humFile.println(buf); memset(buf,0,sizeof(buf));
    hoursFile.close();
    tempFile.close();
    presFile.close();
    humFile.close();
  }
  else if (n==72)
  {
    char buf [10];
    SD.remove("/hours.txt"); SD.remove("/temperature.txt");
    SD.remove("/pressure.txt");SD.remove("/humidity.txt");
    File hoursFile = SD.open("/hours.txt",FILE_APPEND);
    File tempFile = SD.open("/temperature.txt",FILE_APPEND);
    File presFile = SD.open("/pressure.txt",FILE_APPEND);
    File humFile = SD.open("/humidity.txt",FILE_APPEND);
    for (int i=1;i<72;i++)
    {
       sprintf(buf,"%d",hours[i]); hoursFile.println(buf); memset(buf,0,sizeof(buf));
       sprintf(buf,"%.1f",temps[i]); tempFile.println(buf); memset(buf,0,sizeof(buf));
       sprintf(buf,"%d",pressures[i]); presFile.println(buf); memset(buf,0,sizeof(buf));
       sprintf(buf,"%d",humidities[i]); humFile.println(buf); memset(buf,0,sizeof(buf));
    }
       sprintf(buf,"%d",hour); hoursFile.println(buf); memset(buf,0,sizeof(buf));
       sprintf(buf,"%.1f",temp); tempFile.println(buf); memset(buf,0,sizeof(buf));
       sprintf(buf,"%d",pres); presFile.println(buf); memset(buf,0,sizeof(buf));
       sprintf(buf,"%d",hum); humFile.println(buf); memset(buf,0,sizeof(buf));
       hoursFile.close();
      tempFile.close();
      presFile.close();
      humFile.close();
  }
}
void writeTLB(int val)
{
  char buf [7];
  File file = SD.open("/temp_LB.txt",FILE_WRITE);
  sprintf(buf, "%d",val);
  file.print(buf);
  file.close();
}
void writeTUB(int val)
{
  char buf [7];
  File file = SD.open("/temp_UB.txt",FILE_WRITE);
  sprintf(buf, "%d",val);
  file.print(buf);
  file.close();
}
void writePLB(int val)
{
  char buf [7];
  File file = SD.open("/pres_LB.txt",FILE_WRITE);
  sprintf(buf, "%d",val);
  file.print(buf);
  file.close();
}
void writePUB(int val)
{
  char buf [7];
  File file = SD.open("/pres_UB.txt",FILE_WRITE);
  sprintf(buf, "%d",val);
  file.print(buf);
  file.close();
}
void writeHLB(int val)
{
  char buf [7];
  File file = SD.open("/hum_LB.txt",FILE_WRITE);
  sprintf(buf, "%d",val);
  file.print(buf);
  file.close();
}
void writeHUB(int val)
{
  char buf [7];
  File file = SD.open("/hum_UB.txt",FILE_WRITE);
  sprintf(buf, "%d",val);
  file.print(buf);
  file.close();
}
void writeBrightness(int val)
{
  char buf [7];
  File file = SD.open("/brightness.txt",FILE_WRITE);
  sprintf(buf, "%d",val);
  file.print(buf);
  file.close();
}
void writeFocus(char val)
{
  char buf [7];
  File file = SD.open("/focus.txt",FILE_WRITE);
  sprintf(buf, "%d",val);
  file.print(buf);
  file.close();
}
void readBrightness()
{
  char digits[7];
  int n = 0;
  int val = 0;
  if (SD.exists("/brightness.txt"))
  {
    File f = SD.open("/brightness.txt");
      char znak;
      while (f.available()){
      znak = f.read();
      if (znak!='\n'&&znak!='\0')
      {
        digits[n] = (char)znak;
        n++;
      }
      else 
      {
        val = atoi(digits);
        break;
      }
      }
      val = atoi(digits);
      f.close();
  }else {writeBrightness(4); val = 4;}
  if (val<0 || val>8) {val = 4; writeBrightness(4);}
  brightness = val;
}
void readFocus()
{
  char digits[7];
  int n = 0;
  int val = 0;
  if (SD.exists("/focus.txt"))
  {
    File f = SD.open("/focus.txt");
      char znak;
      while (f.available()){
      znak = f.read();
      if (znak!='\n'&&znak!='\0')
      {
        digits[n] = (char)znak;
        n++;
      }
      else 
      {
        val = atoi(digits);
        break;
      }
      }
      val = atoi(digits);
      f.close();
  }else {writeFocus(0); val = 0;}
  if (!(val==1 || val==0)) {val = 0; writeFocus(0);}
  focus = val;
}
int readBound(const char* filename)
{
  char digits[7];
  int n = 0;
  int val = 0;
  if (SD.exists(filename))
  {
    File f = SD.open(filename);
      char znak;
      while (f.available()){
      znak = f.read();
      if (znak!='\n'&&znak!='\0')
      {
        digits[n] = (char)znak;
        n++;
      }
      else 
      {
        val = atoi(digits);
        break;
      }
      }
      val = atoi(digits);
      f.close();
  }
  
  return val;
  
}
void readBounds()
{
  int tlb = readBound("/temp_LB.txt");
  int tub = readBound("/temp_UB.txt");
  int plb = readBound("/pres_LB.txt");
  int pub = readBound("/pres_UB.txt");
  int hlb = readBound("/hum_LB.txt");
  int hub = readBound("/hum_UB.txt");
  if (tlb<=tub-2 && tlb>=-40 && tlb<=120 && tub>=-40 && tub <= 120) {temp_LB = tlb; temp_UB = tub;}
  else {temp_LB = 20; temp_UB = 24; writeTLB(temp_LB);writeTUB(temp_UB);}

  if (plb<=pub-10 && plb>=300 && plb<=1100 && pub>=300 && pub <= 1100) {pres_LB = plb; pres_UB = pub;}
  else {pres_LB = 950; pres_UB = 1050; writePLB(pres_LB);writePUB(pres_UB);}

  if (hlb<=hub-2 && hlb>=10 && hlb<=90 && hub>=10 && hub <=90) {hum_LB = hlb; hum_UB = hub;}
  else {hum_LB = 30; hum_UB = 50; writeHLB(hum_LB);writeHUB(hum_UB);}
}
void factoryReset()
{
  SD.remove("/hours.txt"); 
  SD.remove("/temperature.txt");
  SD.remove("/pressure.txt");
  SD.remove("/humidity.txt");
  SD.remove("/temp_lb.txt");
  SD.remove("/temp_ub.txt");
  SD.remove("/pres_lb.txt");
  SD.remove("/pres_ub.txt");
  SD.remove("/pres_lb.txt");
  SD.remove("/pres_ub.txt");
  SD.remove("/brightness.txt");
  SD.remove("/focus.txt");
  delete [] pressures; delete [] hours; delete [] temps; delete [] humidities;
  n_hours = 0; n_temps = 0; n_pressures = 0; n_humidities = 0;
  pressures = new int [0]; hours = new int [0]; temps = new float [0]; humidities = new int [0];
  readBounds();
  readBrightness();
  readFocus();
  setDate(2023,9,15); 
  setTime(10, 49, 0);
  M5.Axp.SetLcdVoltage(2500 + 100*brightness);
}
void forecast()
{
  readHours();
  if (n_hours==0) {return;} 
  readTemps();readHumidities();readPressures();
  int n = n_hours;
  int ho;
  int last14 = 0;
  int last2 = 0;
  float tTD = 0, tTN = 0;
  int tHD = 0, tHN = 0;
  int sP = 0;
  for (int i=0;i<71;i++)
  {
    ho = hours[i];
    if (ho>=21||ho<=8) {
      tTN = temps[i+1]-temps[i];
      tHN = humidities[i+1] - humidities[i];
  }
  else
  {
      tTD = temps[i+1]-temps[i];
      tHD = humidities[i+1] - humidities[i];
    }
    sP += pressures[i];
    if (ho==14) {last14 = i;}
    if (ho==2) {last2 = i;}
  }

  tempD1 = temps[last14] + 24*(tTD/71);
  tempD2 = tempD1 + 24*(tTD/71);
  humD1 = humidities[last14] + 24*(tHD/71);
  humD2 = humD1 + 24*(tHD/71);
  tempN1 = temps[last2] + 24*(tTN/71);
  tempN2 = tempN1 + 24*(tTN/71);
  humN1 = humidities[last2] + 24*(tHN/71);
  humN2 = humN1 + 24*(tHN/71);
  pres1 = (sP+pressures[71]) / 71;
  pres2 = pres1;   
}
void syncTime()
{
  configTime(gmtOffset_sec, daylightOffset_sec, ntp_server);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo))
  {
    timeinfo.tm_hour;
    setDate(timeinfo.tm_year+1900,timeinfo.tm_mon+1,timeinfo.tm_mday);
    setTime(timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
  }
 
}
int getY(int val, int minval, int unit, int ymin)
{
  return ymin - (val - minval) * unit;
}
void screen1()
{
  if (drawScreen) { 
    drawScreen--;
    int hours = getHours(); int minutes = getMinutes();
    int day = getDay(); int month = getMonth(); int year = getYear();
    char time_buf[10]; char hours_buf[5]; char minutes_buf[5];
    char date_buf[15]; char month_buf[5]; char day_buf[5]; char year_buf[5];
    if (minutes<10) {sprintf(minutes_buf,"0%d", minutes);}
    else {sprintf(minutes_buf,"%d", minutes);}
    if (hours<10) {sprintf(hours_buf,"0%d:", hours);}
    else {sprintf(hours_buf,"%d:", hours);}
    if (day<10) {sprintf(day_buf,"0%d.", day);}
    else {sprintf(day_buf,"%d.", day);}
    if (month<10) {sprintf(month_buf,"0%d.", month);}
    else {sprintf(month_buf,"%d.", month);}
    sprintf(year_buf,"%d",year);
    strcpy(time_buf,hours_buf);strcat(time_buf,minutes_buf);
    strcpy(date_buf,day_buf);strcat(date_buf,month_buf);strcat(date_buf,year_buf);
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(4);
    M5.Lcd.drawString(time_buf,160,95,4);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString(date_buf,160,175,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("...",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
  if (((millis()-timer1>=timerLimit1)||(millis()-timer1<0))&&focus) {drawScreen = 1; timer1 = millis();}
  else if ((((millis()-timer1>=timerLimit1)||(millis()-timer1<0))&&!focus)||M5.BtnC.wasPressed()) {menu_stan = 2; drawScreen=1; timer1=millis();}
  else if (M5.BtnB.wasReleased()) {menu_stan = 6; drawScreen=1; timer2 = millis();}
  else if (M5.BtnA.wasPressed()) {menu_stan = 4; drawScreen=1; timer1 = millis();}
  
}
void screen2()
{
  if (drawScreen) {
    drawScreen--;
    showBatteryLevelAndNetworkStatus();
    float temp = getTemperature();
    char temp_buf[7];
    sprintf(temp_buf,"%.1f", temp);
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setTextDatum(MC_DATUM);
    if ((temp > temp_UB && temp < temp_UB+2) || (temp <= temp_LB && temp >= temp_LB - 2)) {
      M5.Lcd.fillRect(0, 70, 320, 25, MAROON);
      M5.Lcd.fillRect(0, 170, 320, 25, MAROON);
      }
    else if (temp > temp_UB + 2 || temp < temp_LB - 2) {M5.Lcd.fillRect(0, 70, 320, 140, MAROON);}
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("T [C]",160,40,4);
    M5.Lcd.setTextSize(4);
    M5.Lcd.drawString(temp_buf,160,135,4);
    M5.Lcd.setTextSize(2);
    char bounds_buf[20]; char lbbuf[10]; char ubbuf[10];
    sprintf(lbbuf,"[%d -",temp_LB);
    sprintf(ubbuf, " %d]",temp_UB);
    strcpy(bounds_buf,lbbuf);strcat(bounds_buf,ubbuf);
    M5.Lcd.drawString(bounds_buf,160,185,2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("...",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
  if (((millis()-timer1>=timerLimit1)||(millis()-timer1<0))&&focus) {drawScreen = 1; timer1 = millis();}
  else if ((((millis()-timer1>=timerLimit1)||(millis()-timer1<0))&&!focus)||M5.BtnC.wasPressed()) {menu_stan = 3; drawScreen=1; timer1=millis();}
  else if (M5.BtnB.wasReleased()) {menu_stan = 6; drawScreen=1; timer2 = millis();}
  else if (M5.BtnA.wasPressed()) {menu_stan = 1; drawScreen=1; timer1 = millis();}
}
void screen3()
{
  if (drawScreen) {
    drawScreen--;
    int pres = getPressure();
    char pres_buf[5];
    sprintf(pres_buf,"%d", pres);
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(GREEN);
    if ((pres > pres_UB && pres < pres_UB+5) || (pres <= pres_LB && pres >= pres_LB - 5)) {
      M5.Lcd.fillRect(0, 70, 320, 25, MAROON);
      M5.Lcd.fillRect(0, 170, 320, 25, MAROON);
      }
    else if (pres > pres_UB + 5 || pres < pres_LB - 5) {M5.Lcd.fillRect(0, 70, 320, 140, MAROON);}
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("p [hPa]",160,40,4);
    M5.Lcd.setTextSize(4);
    M5.Lcd.drawString(pres_buf,160,135,4);
    M5.Lcd.setTextSize(2);
    char bounds_buf[20]; char lbbuf[10]; char ubbuf[10];
    sprintf(lbbuf,"[%d -",pres_LB);
    sprintf(ubbuf, " %d]",pres_UB);
    strcpy(bounds_buf,lbbuf);strcat(bounds_buf,ubbuf);
    M5.Lcd.drawString(bounds_buf,160,185,2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("...",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
  if (((millis()-timer1>=timerLimit1)||(millis()-timer1<0))&&focus) {drawScreen = 1; timer1 = millis();}
  else if ((((millis()-timer1>=timerLimit1)||(millis()-timer1<0))&&!focus)||M5.BtnC.wasPressed()) {menu_stan = 4; drawScreen=1; timer1=millis();}
  else if (M5.BtnB.wasReleased()) {menu_stan = 6; drawScreen=1; timer2 = millis();}
  else if (M5.BtnA.wasPressed()) {menu_stan = 2; drawScreen=1; timer1 = millis();}
}
void screen4()
{
  if (drawScreen) {
    drawScreen--;
    int hum = getHumidity();
    char hum_buf[4];
    sprintf(hum_buf,"%d", hum);
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(CYAN);
    if ((hum > hum_UB && hum < hum_UB+2) || (hum <= hum_LB && hum >= hum_LB - 2)) {
      M5.Lcd.fillRect(0, 70, 320, 25, MAROON);
      M5.Lcd.fillRect(0, 170, 320, 25, MAROON);
      }
    else if (hum > hum_UB + 2 || hum < hum_LB - 2) {M5.Lcd.fillRect(0, 70, 320, 140, MAROON);}
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Hum. [%]",160,40,4);
    M5.Lcd.setTextSize(4);
    M5.Lcd.drawString(hum_buf,160,135,4);
    M5.Lcd.setTextSize(2);
    char bounds_buf[20]; char lbbuf[10]; char ubbuf[10];
    sprintf(lbbuf,"[%d -",hum_LB);
    sprintf(ubbuf, " %d]",hum_UB);
    strcpy(bounds_buf,lbbuf);strcat(bounds_buf,ubbuf);
    M5.Lcd.drawString(bounds_buf,160,185,2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("...",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
  if (((millis()-timer1>=timerLimit1)||(millis()-timer1<0))&&focus) {drawScreen = 1; timer1 = millis();}
  else if ((((millis()-timer1>=timerLimit1)||(millis()-timer1<0))&&!focus)||M5.BtnC.wasPressed()) {menu_stan = 1; drawScreen=1; timer1=millis();}
  else if (M5.BtnB.wasReleased()) {menu_stan = 6; drawScreen=1; timer2 = millis();}
  else if (M5.BtnA.wasPressed()) {menu_stan = 3; drawScreen=1; timer1 = millis();}
}
void screen5()
{
  if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Return",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 52; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 1; drawScreen=1; timer1 = millis(); timer2=0;}
    else if (M5.BtnC.wasPressed()) {menu_stan = 6; drawScreen=1; timer2 = millis();}
}
void screen6()
{
  if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Stats",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 5; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 39; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 7; drawScreen=1; timer2 = millis();}
}
void screen7()
{
  if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Options",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 6; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 10; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 8; drawScreen=1; timer2 = millis();}
}
void screen8()
{
  if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Forecast",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 7; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 47; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 52; drawScreen=1; timer2 = millis();}
}
void screen52()
{
  if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString("Help",160,40,4);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0,60);
    M5.Lcd.print("-Use three buttons under the screen to navigate\n-SD card is required to use certain functions and to preserve settings\n-Turn on \"Focus mode\" to stop screen changes without input");
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString(".",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 7; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 5; drawScreen=1; timer2 = millis();}
}
void screen9()
{
   if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Return",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 45; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 7; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 10; drawScreen=1; timer2 = millis();}
}
void screen10()
{
  if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Set date",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 9; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 27; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 11; drawScreen=1; timer2 = millis();}
}
void screen11()
{
  if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Set time",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 10; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 34; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 12; drawScreen=1; timer2 = millis();}
}
void screen12()
{
  if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Set",160,60,4);
    M5.Lcd.drawString("bounds",160,120,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 11; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 14; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 45; drawScreen=1; timer2 = millis();}
}
void screen45()
{
  if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Set",160,60,4);
    M5.Lcd.drawString("brightn.",160,120,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 12; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 50; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 51; drawScreen=1; timer2 = millis();}
}
void screen51()
{
  if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Focus",160,60,4);
    M5.Lcd.drawString("mode",160,120,4);
    M5.Lcd.setTextSize(1);
    if (focus) {M5.Lcd.drawString("[ON]",160,175,4);}
    else if (!focus) {M5.Lcd.drawString("[OFF]",160,175,4);}
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 45; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {
      focus = !focus; writeFocus(focus); drawScreen=1; timer2 = millis();
      }
    else if (M5.BtnC.wasPressed()) {menu_stan = 49; drawScreen=1; timer2 = millis();}
}
void screen49()
{
  if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    if (error) {M5.Lcd.setTextColor(RED);M5.Lcd.drawString("Reset complete",160,175,4);M5.Lcd.setTextColor(YELLOW);}
    if (reset_confirm) {M5.Lcd.setTextColor(RED);M5.Lcd.drawString("OK to reset",160,175,4);M5.Lcd.setTextColor(YELLOW);}
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Factory",160,60,4);
    M5.Lcd.drawString("reset",160,120,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 52; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {

      if (!reset_confirm) {reset_confirm = true;}
      else
      {
       M5.Lcd.setTextDatum(MC_DATUM);
      M5.Lcd.fillRect(0,155,320,40,BLACK);
      M5.Lcd.drawString("resetting...",160,175,4);
      factoryReset(); error = true; reset_confirm = false;    
      }
      drawScreen=1; timer2 = millis();
      }
    else if (M5.BtnC.wasPressed()) {menu_stan = 9; drawScreen=1; timer2 = millis();}
}
void screen50()
{
  if (drawScreen) {
    drawScreen--;
    char buf[10];
    sprintf(buf,"%d", brightness);
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Brightness",160,40,4);
    if (error) {M5.Lcd.setTextColor(RED);M5.Lcd.drawString("invalid value",160,70,4);M5.Lcd.setTextColor(WHITE);}
    M5.Lcd.setTextSize(4);
    M5.Lcd.drawString(buf,160,135,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("-",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString("+",275,240,4);
    }
  if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {
      if (brightness-1 >= 0) {brightness--; writeBrightness(brightness); M5.Axp.SetLcdVoltage(2500 + 100*brightness); error = false;}
      else {error = true;}
      drawScreen=1; timer2 = millis();
      }
    else if (M5.BtnB.wasReleased()) {menu_stan = 45; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {
       if (brightness+1 < 9) {brightness++; writeBrightness(brightness); M5.Axp.SetLcdVoltage(2500 + 100*brightness); error = false;}
      else {error = true;}
      drawScreen=1; timer2 = millis();
      }
}
void screen13()
{
   if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Return",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 19; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 12; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 14; drawScreen=1; timer2 = millis();}
}
void screen14()
{
   if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Min. T",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 13; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 20; drawScreen=1; prevBound = temp_LB; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 15; drawScreen=1; timer2 = millis();}
}
void screen15()
{
   if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Max. T",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 14; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 21; drawScreen=1; prevBound = temp_UB; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 16; drawScreen=1; timer2 = millis();}
}
void screen16()
{
   if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Min. p",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 15; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 22; drawScreen=1; prevBound = pres_LB; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 17; drawScreen=1; timer2 = millis();}
}
void screen17()
{
   if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Max. p",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 16; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 23; drawScreen=1; prevBound = pres_UB; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 18; drawScreen=1; timer2 = millis();}
}
void screen18()
{
   if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Min.",160,60,4);
    M5.Lcd.drawString("hum.",160,120,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 17; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 24; drawScreen=1; prevBound = hum_LB; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 19; drawScreen=1; timer2 = millis();}
}
void screen19()
{
   if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Max.",160,60,4);
    M5.Lcd.drawString("hum.",160,120,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 18; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 25; drawScreen=1; prevBound = hum_UB; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 13; drawScreen=1; timer2 = millis();}
}
void screen20()
{
  if (drawScreen) {
    drawScreen--;
    char temp_buf[10];
    sprintf(temp_buf,"%d", temp_LB);
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Min. T [C]",160,40,4);
    char pbuf[10];
    sprintf(pbuf,"[%d]",prevBound);
    M5.Lcd.drawString("prev.:",0,0,4);
    M5.Lcd.drawString(pbuf,0,40,4);
    if (error) {M5.Lcd.setTextColor(RED);M5.Lcd.drawString("invalid value",160,70,4);M5.Lcd.setTextColor(YELLOW);}
    M5.Lcd.setTextSize(4);
    M5.Lcd.drawString(temp_buf,160,135,4);
    M5.Lcd.setTextSize(2);
    char bounds_buf[20]; char buf[10];
    sprintf(temp_buf,"[%d -",temp_LB);
    sprintf(buf, " %d]",temp_UB);
    strcpy(bounds_buf,temp_buf);strcat(bounds_buf,buf);
    M5.Lcd.drawString(bounds_buf,160,185,2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("-",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString("+",275,240,4);
    }
  if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {
      if (temp_LB-1 <= temp_UB-2 && temp_LB-1>=-40 && temp_LB-1<=120) {temp_LB--; writeTLB(temp_LB);error = false;}
      else {error = true;}
      drawScreen=1; timer2 = millis();
      }
    else if (M5.BtnB.wasReleased()) {menu_stan = 14; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {
      if (temp_LB+1 <= temp_UB-2 && temp_LB+1>=-40 && temp_LB+1<=120) {temp_LB++; writeTLB(temp_LB);error = false;}
      else {error = true;}
      drawScreen=1; timer2 = millis();
      }
}
void screen21()
{
  if (drawScreen) {
    drawScreen--;
    char temp_buf[10];
    sprintf(temp_buf,"%d", temp_UB);
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Max. T [C]",160,40,4);
    char pbuf[10];
    sprintf(pbuf,"[%d]",prevBound);
    M5.Lcd.drawString("prev.:",0,0,4);
    M5.Lcd.drawString(pbuf,0,40,4);
    if (error) {M5.Lcd.setTextColor(RED);M5.Lcd.drawString("invalid value",160,70,4);M5.Lcd.setTextColor(YELLOW);}
    M5.Lcd.setTextSize(4);
    M5.Lcd.drawString(temp_buf,160,135,4);
    M5.Lcd.setTextSize(2);
    char bounds_buf[20]; char buf[10];
    sprintf(temp_buf,"[%d -",temp_LB);
    sprintf(buf, " %d]",temp_UB);
    strcpy(bounds_buf,temp_buf);strcat(bounds_buf,buf);
    M5.Lcd.drawString(bounds_buf,160,185,2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("-",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString("+",275,240,4);
    }
  if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {
      if (temp_UB-1 >= temp_LB+2 && temp_UB-1>=-40 && temp_UB-1<=120) {temp_UB--; writeTUB(temp_UB);error = false;}
      else {error = true;}
      drawScreen=1; timer2 = millis();
      }
    else if (M5.BtnB.wasReleased()) {menu_stan = 15; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {
      if (temp_UB+1 >= temp_LB+2 && temp_UB+1>=-40 && temp_UB+1<=120) {temp_UB++; writeTUB(temp_UB);error = false;}
      else {error = true;}
      drawScreen=1; timer2 = millis();
      }
}
void screen22()
{
  if (drawScreen) {
    drawScreen--;
    char pres_buf[10];
    sprintf(pres_buf,"%d", pres_LB);
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Min. p [hPa]",160,40,4);
    char pbuf[10];
    sprintf(pbuf,"[%d]",prevBound);
    M5.Lcd.drawString("prev.:",0,0,4);
    M5.Lcd.drawString(pbuf,0,40,4);
    if (error) {M5.Lcd.setTextColor(RED);M5.Lcd.drawString("invalid value",160,70,4);M5.Lcd.setTextColor(GREEN);}
    M5.Lcd.setTextSize(4);
    M5.Lcd.drawString(pres_buf,160,135,4);
    M5.Lcd.setTextSize(2);
    char bounds_buf[20]; char buf[10];
    sprintf(pres_buf,"[%d -",pres_LB);
    sprintf(buf, " %d]",pres_UB);
    strcpy(bounds_buf,pres_buf);strcat(bounds_buf,buf);
    M5.Lcd.drawString(bounds_buf,160,185,2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("-",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString("+",275,240,4);
    }
  if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {
      if (pres_LB-1 <= pres_UB-10 && pres_LB-1>=300 && pres_LB-1<=1100) {pres_LB--; writePLB(pres_LB);error = false;}
      else {error = true;}
      drawScreen=1; timer2 = millis();
      }
    else if (M5.BtnB.wasReleased()) {menu_stan = 16; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {
      if (pres_LB+1 <= pres_UB-10 && pres_LB+1>=300 && pres_LB+1<=1100) {pres_LB++; writePLB(pres_LB);error = false;}
      else {error = true;}
      drawScreen=1; timer2 = millis();
      }
}
void screen23()
{
  if (drawScreen) {
    drawScreen--;
    char pres_buf[10];
    sprintf(pres_buf,"%d", pres_UB);
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Max. p [hPa]",160,40,4);
    char pbuf[10];
    sprintf(pbuf,"[%d]",prevBound);
    M5.Lcd.drawString("prev.:",0,0,4);
    M5.Lcd.drawString(pbuf,0,40,4);
    if (error) {M5.Lcd.setTextColor(RED);M5.Lcd.drawString("invalid value",160,70,4);M5.Lcd.setTextColor(GREEN);}
    M5.Lcd.setTextSize(4);
    M5.Lcd.drawString(pres_buf,160,135,4);
    M5.Lcd.setTextSize(2);
    char bounds_buf[20]; char buf[10];
    sprintf(pres_buf,"[%d -",pres_LB);
    sprintf(buf, " %d]",pres_UB);
    strcpy(bounds_buf,pres_buf);strcat(bounds_buf,buf);
    M5.Lcd.drawString(bounds_buf,160,185,2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("-",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString("+",275,240,4);
    }
  if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {
      if (pres_UB-1 >= pres_LB+10 && pres_UB-1>=300 && pres_UB-1<=1100) {pres_UB--; writePUB(pres_UB);error = false;}
      else {error = true;}
      drawScreen=1; timer2 = millis();
      }
    else if (M5.BtnB.wasReleased()) {menu_stan = 17; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {
      if (pres_UB-1 >= pres_LB+10 && pres_UB+1>=300 && pres_UB+1<=1100) {pres_UB++; writePUB(pres_UB);error = false;}
      else {error = true;}
      drawScreen=1; timer2 = millis();
      }
}
void screen24()
{
  if (drawScreen) {
    drawScreen--;
    char hum_buf[10];
    sprintf(hum_buf,"%d", hum_LB);
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Min. hum. [%]",160,40,4);
    char pbuf[10];
    sprintf(pbuf,"[%d]",prevBound);
    M5.Lcd.drawString("prev.:",0,0,4);
    M5.Lcd.drawString(pbuf,0,40,4);
    if (error) {M5.Lcd.setTextColor(RED);M5.Lcd.drawString("invalid value",160,70,4);M5.Lcd.setTextColor(CYAN);}
    M5.Lcd.setTextSize(4);
    M5.Lcd.drawString(hum_buf,160,135,4);
    M5.Lcd.setTextSize(2);
    char bounds_buf[20]; char buf[10];
    sprintf(hum_buf,"[%d -",hum_LB);
    sprintf(buf, " %d]",hum_UB);
    strcpy(bounds_buf,hum_buf);strcat(bounds_buf,buf);
    M5.Lcd.drawString(bounds_buf,160,185,2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("-",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString("+",275,240,4);
    }
  if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {
      if (hum_LB-1 <= hum_UB-2 && hum_LB-1>=10 && hum_LB-1<=90) {hum_LB--; writeHLB(hum_LB);error = false;}
      else {error = true;}
      drawScreen=1; timer2 = millis();
      }
    else if (M5.BtnB.wasReleased()) {menu_stan = 18; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {
      if (hum_LB+1 <= hum_UB-2 && hum_LB+1>=10 && hum_LB+1<=90) {hum_LB++; writeHLB(hum_LB);error = false;}
      else {error = true;}
      drawScreen=1; timer2 = millis();
      }
}
void screen25()
{
  if (drawScreen) {
    drawScreen--;
    char hum_buf[10];
    sprintf(hum_buf,"%d", hum_UB);
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Max. hum. [%]",160,40,4);
    char pbuf[10];
    sprintf(pbuf,"[%d]",prevBound);
    M5.Lcd.drawString("prev.:",0,0,4);
    M5.Lcd.drawString(pbuf,0,40,4);
    if (error) {M5.Lcd.setTextColor(RED);M5.Lcd.drawString("invalid value",160,70,4);M5.Lcd.setTextColor(CYAN);}
    M5.Lcd.setTextSize(4);
    M5.Lcd.drawString(hum_buf,160,135,4);
    M5.Lcd.setTextSize(2);
    char bounds_buf[20]; char buf[10];
    sprintf(hum_buf,"[%d -",hum_LB);
    sprintf(buf, " %d]",hum_UB);
    strcpy(bounds_buf,hum_buf);strcat(bounds_buf,buf);
    M5.Lcd.drawString(bounds_buf,160,185,2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("-",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString("+",275,240,4);
    }
  if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {
      if (hum_UB-1 >= hum_LB+2 && hum_UB-1>=10 && hum_UB-1<=90) {hum_UB--; writeHUB(hum_UB);error = false;}
      else {error = true;}
      drawScreen=1; timer2 = millis();
      }
    else if (M5.BtnB.wasReleased()) {menu_stan = 19; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {
      if (hum_UB+1 >= hum_LB+2 && hum_UB+1>=10 && hum_UB+1<=90) {hum_UB++; writeHUB(hum_UB);error = false;}
      else {error = true;}
      drawScreen=1; timer2 = millis();
      }
}
void screen26()
{
   if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Return",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 29; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 10; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 27; drawScreen=1; timer2 = millis();}
}
void screen27()
{
    if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Day",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 26; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 30; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 28; drawScreen=1; timer2 = millis();}

  
   
}
void screen28()
{
   if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Month",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 27; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 31; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 29; drawScreen=1; timer2 = millis();}
}
void screen29()
{
   if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Year",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 28; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 32; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 26; drawScreen=1; timer2 = millis();}
}
void screen30()
{
  if (drawScreen) {
    drawScreen--;
    char day_buf[6];
    int day = getDay();
    if (day<10) {sprintf(day_buf,"0%d", day);}
    else {sprintf(day_buf,"%d", day);}
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Day",160,40,4);
    if (error) {M5.Lcd.setTextColor(RED);M5.Lcd.drawString("invalid date",160,70,4);M5.Lcd.setTextColor(WHITE);}
    M5.Lcd.setTextSize(4);
    M5.Lcd.drawString(day_buf,160,135,4);
    M5.Lcd.setTextSize(2);
    int month = getMonth(); int year = getYear();
    char date_buf[18]; char month_buf[6]; char year_buf[6];
    if (day<10) {sprintf(day_buf,"[0%d.", day);}
    else {sprintf(day_buf,"[%d.", day);}
    if (month<10) {sprintf(month_buf,"0%d.", month);}
    else {sprintf(month_buf,"%d.", month);}
    sprintf(year_buf,"%d]",year);
    strcpy(date_buf,day_buf);strcat(date_buf,month_buf);strcat(date_buf,year_buf);
    M5.Lcd.drawString(date_buf,160,185,2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("-",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString("+",275,240,4);
    }
  if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {
      int day = getDay(); day--; int month = getMonth(); int year=getYear();
      if (checkDate(year,month,day)) {setDate(year,month,day); error=false;}
      else {error=true;}
      drawScreen=1; timer2 = millis();
      }
    else if (M5.BtnB.wasReleased()) {menu_stan = 27; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {
      int day = getDay(); day++; int month = getMonth(); int year=getYear();
      if (checkDate(year,month,day)) {setDate(year,month,day); error=false;}
      else {error=true;}
      drawScreen=1; timer2 = millis();
      }
}
void screen31()
{
  if (drawScreen) {
    drawScreen--;
    char month_buf[6];
    int month = getMonth();
    if (month<10) {sprintf(month_buf,"0%d", month);}
    else {sprintf(month_buf,"%d", month);}
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Month",160,40,4);
    if (error) {M5.Lcd.setTextColor(RED);M5.Lcd.drawString("invalid date",160,70,4);M5.Lcd.setTextColor(WHITE);}
    M5.Lcd.setTextSize(4);
    M5.Lcd.drawString(month_buf,160,135,4);
    M5.Lcd.setTextSize(2);
    int day = getDay(); int year = getYear();
    char date_buf[18]; char day_buf[6]; char year_buf[6];
    if (day<10) {sprintf(day_buf,"[0%d.", day);}
    else {sprintf(day_buf,"[%d.", day);}
    if (month<10) {sprintf(month_buf,"0%d.", month);}
    else {sprintf(month_buf,"%d.", month);}
    sprintf(year_buf,"%d]",year);
    strcpy(date_buf,day_buf);strcat(date_buf,month_buf);strcat(date_buf,year_buf);
    M5.Lcd.drawString(date_buf,160,185,2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("-",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString("+",275,240,4);
    }
  if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {
      int day = getDay(); int month = getMonth(); month--; int year=getYear();
      if (checkDate(year,month,day)) {setDate(year,month,day); error=false;}
      else {error=true;}
      drawScreen=1; timer2 = millis();
      }
    else if (M5.BtnB.wasReleased()) {menu_stan = 28; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {
      int day = getDay(); int month = getMonth(); month++; int year=getYear();
      if (checkDate(year,month,day)) {setDate(year,month,day); error=false;}
      else {error=true;}
      drawScreen=1; timer2 = millis();
      }
}
void screen32()
{
  if (drawScreen) {
    drawScreen--;
    char year_buf[6];
    int year = getYear();
    sprintf(year_buf,"%d", year);
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Year",160,40,4);
    if (error) {M5.Lcd.setTextColor(RED);M5.Lcd.drawString("invalid date",160,70,4);M5.Lcd.setTextColor(WHITE);}
    M5.Lcd.setTextSize(4);
    M5.Lcd.drawString(year_buf,160,135,4);
    M5.Lcd.setTextSize(2);
    int month = getMonth(); int day = getDay();
    char date_buf[18]; char month_buf[6]; char day_buf[6];
    if (day<10) {sprintf(day_buf,"[0%d.", day);}
    else {sprintf(day_buf,"[%d.", day);}
    if (month<10) {sprintf(month_buf,"0%d.", month);}
    else {sprintf(month_buf,"%d.", month);}
    sprintf(year_buf,"%d]",year);
    strcpy(date_buf,day_buf);strcat(date_buf,month_buf);strcat(date_buf,year_buf);
    M5.Lcd.drawString(date_buf,160,185,2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("-",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString("+",275,240,4);
    }
  if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {
      int day = getDay(); int month = getMonth(); int year=getYear(); year--;
      if (checkDate(year,month,day)) {setDate(year,month,day); error=false;}
      else {error=true;}
      drawScreen=1; timer2 = millis();
      }
    else if (M5.BtnB.wasReleased()) {menu_stan = 29; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {
      int day = getDay(); int month = getMonth(); int year=getYear(); year++;
      if (checkDate(year,month,day)) {setDate(year,month,day); error=false;}
      else {error=true;}
      drawScreen=1; timer2 = millis();
      }
}
void screen33()
{
   if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Return",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 35; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 11; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 34; drawScreen=1; timer2 = millis();}
}
void screen34()
{
   if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Hours",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 33; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 36; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 35; drawScreen=1; timer2 = millis();}
}
void screen35()
{
   if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Minutes",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 34; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 37; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 33; drawScreen=1; timer2 = millis();}
}
void screen36()
{
  if (drawScreen) {
    drawScreen--;
    char hours_buf[5];
    int hours = getHours();
    if (hours<10) {sprintf(hours_buf,"0%d", hours);}
    else {sprintf(hours_buf,"%d", hours);}
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Hours",160,40,4);
    if (error) {M5.Lcd.setTextColor(RED);M5.Lcd.drawString("invalid time",160,70,4);M5.Lcd.setTextColor(WHITE);}
    M5.Lcd.setTextSize(4);
    M5.Lcd.drawString(hours_buf,160,135,4);
    M5.Lcd.setTextSize(2);
    int minutes = getMinutes();
    char time_buf[10]; char minutes_buf[5];
    if (minutes<10) {sprintf(minutes_buf,"0%d]", minutes);}
    else {sprintf(minutes_buf,"%d]", minutes);}
    if (hours<10) {sprintf(hours_buf,"[0%d:", hours);}
    else {sprintf(hours_buf,"[%d:", hours);}
    strcpy(time_buf,hours_buf);strcat(time_buf,minutes_buf);
    M5.Lcd.drawString(time_buf,160,185,2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("-",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString("+",275,240,4);
    }
  if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {
      int hours = getHours(); hours--;
      if (hours>=0&&hours<=23) {setTime(hours,getMinutes(),getSeconds());error=false;}
      else {error=true;}
      drawScreen=1; timer2 = millis();
      }
    else if (M5.BtnB.wasReleased()) {menu_stan = 34; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {
      int hours = getHours(); hours++;
      if (hours>=0&&hours<=23) {setTime(hours,getMinutes(),getSeconds());error=false;}
      else {error=true;}
      drawScreen=1; timer2 = millis();
      }
}
void screen37()
{
  if (drawScreen) {
    drawScreen--;
    char minutes_buf[5];
    int minutes = getMinutes();
    if (minutes<10) {sprintf(minutes_buf,"0%d", minutes);}
    else {sprintf(minutes_buf,"%d", minutes);}
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Minutes",160,40,4);
    if (error) {M5.Lcd.setTextColor(RED);M5.Lcd.drawString("invalid time",160,70,4);M5.Lcd.setTextColor(WHITE);}
    M5.Lcd.setTextSize(4);
    M5.Lcd.drawString(minutes_buf,160,135,4);
    M5.Lcd.setTextSize(2);
    int hours = getHours();
    char time_buf[10]; char hours_buf[5];
    if (minutes<10) {sprintf(minutes_buf,"0%d]", minutes);}
    else {sprintf(minutes_buf,"%d]", minutes);}
    if (hours<10) {sprintf(hours_buf,"[0%d:", hours);}
    else {sprintf(hours_buf,"[%d:", hours);}
    strcpy(time_buf,hours_buf);strcat(time_buf,minutes_buf);
    M5.Lcd.drawString(time_buf,160,185,2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("-",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString("+",275,240,4);
    }
  if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {
      int minutes = getMinutes(); minutes--;
      if (minutes>=0&&minutes<=59) {setTime(getHours(),minutes,getSeconds());error=false;}
      else {error=true;}
      drawScreen=1; timer2 = millis();
      }
    else if (M5.BtnB.wasReleased()) {menu_stan = 35; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {
       int minutes = getMinutes(); minutes++;
      if (minutes>=0&&minutes<=59) {setTime(getHours(),minutes,getSeconds());error=false;}
      else {error=true;}
      drawScreen=1; timer2 = millis();
      }
}
void screen38()
{
   if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Return",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 44; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 6; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 39; drawScreen=1; timer2 = millis();}
}
void screen39()
{
  float day_min = 0,day_max = 0,night_min = 0,night_max = 0, sum_night=0,sum_day=0, avg_night = 0,avg_day = 0;
  int n_day = 0, n_night = 0;
  if (drawScreen) {
    drawScreen--;
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Reading...",160,0,2);
    readTemps();
    readHours();
  if (n_temps == n_hours)
  {
      
      
      for (int i=0;i<n_temps;i++)
      {
          int hour = hours[i];
          float temp = temps[i];
          if (hour>=21||hour<=8)
          {
            if (n_night==0) {night_min = temp; night_max = temp;}
            else 
              {
                if (temp>night_max) {night_max = temp;}
                if (temp<night_min) {night_min = temp;}
              }
            sum_night = sum_night + temp;
            n_night++;
            }
          else
          {
            if (n_day==0) {day_min = temp; day_max = temp;}
            else 
              {
                if (temp>day_max) {day_max = temp;}
                if (temp<day_min) {day_min = temp;}
              }
            sum_day = sum_day + temp;
            n_day++;
           }
            
      }
      if (n_night!=0) {avg_night = sum_night / n_night;}
      if (n_day!=0) {avg_day = sum_day / n_day;}
  }

    char temp_buf[15];
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("T stats [C]",160,40,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(ML_DATUM);
    M5.Lcd.drawString("Day",60,75,4);
    sprintf(temp_buf, "max. %.1f", day_max);
    M5.Lcd.drawString(temp_buf,30,115,4);
    sprintf(temp_buf, "min. %.1f", day_min);
    M5.Lcd.drawString(temp_buf,30,145,4);
    sprintf(temp_buf, "avg. %.1f", avg_day);
    M5.Lcd.drawString(temp_buf,30,175,4);
    M5.Lcd.setTextDatum(MR_DATUM);
    M5.Lcd.drawString("Night",260,75,4);
    M5.Lcd.setTextDatum(ML_DATUM);
    sprintf(temp_buf, "max. %.1f", night_max);
    M5.Lcd.drawString(temp_buf,200,115,4);
    sprintf(temp_buf, "min. %.1f", night_min);
    M5.Lcd.drawString(temp_buf,200,145,4);
    sprintf(temp_buf, "avg. %.1f", avg_night);
    M5.Lcd.drawString(temp_buf,200,175,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString(".",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 38; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 40; drawScreen=1; timer2 = millis();}
}
void screen40()
{
  float tmin = 0,tmax = 0;
  int nt = 0;
  if (drawScreen) {
    drawScreen--;
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Reading...",160,0,2);
    readTemps();
    readHours(); 
      for (int i=0;i<n_temps;i++)
      {
          float temp = temps[i];
            if (nt==0) {tmin = temp; tmax = temp;}
            else 
              {
                if (temp>tmax) {tmax = temp;}
                if (temp<tmin) {tmin = temp;}
              }
              nt++;         
      }
    
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setTextDatum(MC_DATUM);
    char buf[30];
    sprintf(buf, "T [C]");
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString(buf,10,0,2);
    tmax = round(tmax);
    tmin = round(tmin);
    if (tmin>temp_LB) {tmin = temp_LB;}
    if (tmax<temp_UB) {tmax = temp_UB;}
    int unit;
    if (tmax!=tmin) {unit = 160/(tmax-tmin);}
    else {unit = 1;}
    int t1 = tmin + (tmax-tmin)/3;
    int t2 = tmax - (tmax-tmin)/3;
    M5.Lcd.drawLine(60, 20, 60, 189, YELLOW); // y
    M5.Lcd.drawLine(51, 180, 300, 180, YELLOW); //x
    int ylb = getY(temp_LB,tmin,unit,180);
    int yub = getY(temp_UB,tmin,unit,180);
    M5.Lcd.drawLine(51, ylb, 300, ylb, RED); //x
    M5.Lcd.drawLine(51, yub, 300, yub, RED); //x
    if (n_hours>0) {
     sprintf(buf, "%d",hours[0]);
    M5.Lcd.drawString(buf,60,198,2);
    }
    if (tmin<0||0>tmax) 
    {
      int yt0 = getY(0,tmin,unit,180);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.drawString("0",49,yt0,2);
    M5.Lcd.drawLine(51, yt0, 300, yt0, WHITE);
    }
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setTextDatum(MR_DATUM);
    sprintf(buf, "%.0f",tmin);
    M5.Lcd.drawString(buf,49,180,2);
    sprintf(buf, "%.0f",tmax);
    int ymax = getY(round(tmax),tmin,unit,180);
    M5.Lcd.drawString(buf,49,ymax,2);
    M5.Lcd.drawLine(51, ymax, 60, ymax, YELLOW);
    
    int yt1 = getY(t1,tmin,unit,180);
    int yt2 = getY(t2,tmin,unit,180);
    sprintf(buf, "%d",t1);
    M5.Lcd.drawString(buf,49,yt1,2);
    M5.Lcd.drawLine(51, yt1, 60, yt1, YELLOW);
    sprintf(buf, "%d",t2);
    M5.Lcd.drawString(buf,49,yt2,2);
    M5.Lcd.drawLine(51, yt2, 60, yt2, YELLOW);
    M5.Lcd.setTextColor(RED);
    sprintf(buf, "%d",temp_UB);
    M5.Lcd.drawString(buf,49,yub,2);
    M5.Lcd.drawLine(51, yub, 60, yub, RED);
    sprintf(buf, "%d",temp_LB);
    M5.Lcd.drawString(buf,49,ylb,2);
    M5.Lcd.drawLine(51, ylb, 60, ylb, RED);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setTextDatum(MC_DATUM);
    int x = 60, y = getY(round(temps[0]),tmin,unit,180);
    for (int i = 1;i<nt;i++)
    {
      int xx = x + 3;
      int yy = getY(round(temps[i]),tmin,unit,180);
      M5.Lcd.drawLine(x, y, xx, yy, YELLOW);
      if (i%8==0||(i==nt-1&&i>66))
      {
        sprintf(buf, "%d",hours[i]);
        M5.Lcd.drawLine(xx, 180, xx, 189, YELLOW);
        M5.Lcd.drawString(buf,xx,198,2);
      }
      x = xx;
      y = yy;
    }
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString(".",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 39; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 41; drawScreen=1; timer2 = millis();}
}
void screen41()
{
  int day_min = 0,day_max = 0,night_min = 0,night_max = 0, sum_night=0,sum_day=0;
  float avg_night = 0,avg_day = 0;
  int n_day = 0, n_night = 0;
  if (drawScreen) {
    drawScreen--;
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Reading...",160,0,2);
    readPressures();
    readHours();
  if (n_pressures == n_hours)
  {
      for (int i=0;i<n_pressures;i++)
      {
          int hour = hours[i];
          int pres = pressures[i];
          if (hour>=21||hour<=8)
          {
            if (n_night==0) {night_min = pres; night_max = pres;}
            else 
              {
                if (pres>night_max) {night_max = pres;}
                if (pres<night_min) {night_min = pres;}
              }
            sum_night = sum_night + pres;
            n_night++;
            }
          else
          {
            if (n_day==0) {day_min = pres; day_max = pres;}
            else 
              {
                if (pres>day_max) {day_max = pres;}
                if (pres<day_min) {day_min = pres;}
              }
            sum_day = sum_day + pres;
            n_day++;
           }
            
      }
      if (n_night!=0) {avg_night = sum_night / n_night;}
      if (n_day!=0) {avg_day = sum_day / n_day;}
  }

    char temp_buf[15];
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("p stats [hPa]",160,40,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(ML_DATUM);
    M5.Lcd.drawString("Day",60,75,4);
    sprintf(temp_buf, "max. %d", day_max);
    M5.Lcd.drawString(temp_buf,30,115,4);
    sprintf(temp_buf, "min. %d", day_min);
    M5.Lcd.drawString(temp_buf,30,145,4);
    sprintf(temp_buf, "avg. %.1f", avg_day);
    M5.Lcd.drawString(temp_buf,30,175,4);
    M5.Lcd.setTextDatum(MR_DATUM);
    M5.Lcd.drawString("Night",260,75,4);
    M5.Lcd.setTextDatum(ML_DATUM);
    sprintf(temp_buf, "max. %d", night_max);
    M5.Lcd.drawString(temp_buf,200,115,4);
    sprintf(temp_buf, "min. %d", night_min);
    M5.Lcd.drawString(temp_buf,200,145,4);
    sprintf(temp_buf, "avg. %.1f", avg_night);
    M5.Lcd.drawString(temp_buf,200,175,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString(".",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 40; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 42; drawScreen=1; timer2 = millis();}
}
void screen42()
{
  int pmin = 0,pmax = 0;
  int np = 0;
  if (drawScreen) {
    drawScreen--;
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Reading...",160,0,2);
    readPressures();
    readHours();   
      for (int i=0;i<n_pressures;i++)
      {
          int pres = pressures[i];
            if (np==0) {pmin = pres; pmax = pres;}
            else 
              {
                if (pres>pmax) {pmax = pres;}
                if (pres<pmin) {pmin = pres;}
              }
              np++;         
      }
    
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setTextDatum(MC_DATUM);
    char buf[30];
    sprintf(buf, "p [hPa]");
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString(buf,10,0,2);
    pmax = round(pmax);
    pmin = round(pmin);
    if (pmin>pres_LB) {pmin = pres_LB;}
    if (pmax<pres_UB) {pmax = pres_UB;}
    int unit;
    if (pmax!=pmin) {unit = 160/(pmax-pmin);}
    else {unit = 1;}
    int p1 = pmin + (pmax-pmin)/3;
    int p2 = pmax - (pmax-pmin)/3;
    M5.Lcd.drawLine(60, 20, 60, 189, GREEN); // y
    M5.Lcd.drawLine(51, 180, 300, 180, GREEN); //x
    int ylb = getY(pres_LB,pmin,unit,180);
    int yub = getY(pres_UB,pmin,unit,180);
    M5.Lcd.drawLine(51, ylb, 300, ylb, RED); //x
    M5.Lcd.drawLine(51, yub, 300, yub, RED); //x
    if (n_hours>0) {
     sprintf(buf, "%d",hours[0]);
    M5.Lcd.drawString(buf,60,198,2);
    }
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setTextDatum(MR_DATUM);
    sprintf(buf, "%d",pmin);
    M5.Lcd.drawString(buf,49,180,2);
    sprintf(buf, "%d",pmax);
    int ymax = getY(round(pmax),pmin,unit,180);
    M5.Lcd.drawString(buf,49,ymax,2);
    M5.Lcd.drawLine(51, ymax, 60, ymax, GREEN);  
    int yp1 = getY(p1,pmin,unit,180);
    int yp2 = getY(p2,pmin,unit,180);
    sprintf(buf, "%d",p1);
    M5.Lcd.drawString(buf,49,yp1,2);
    M5.Lcd.drawLine(51, yp1, 60, yp1, GREEN);
    sprintf(buf, "%d",p2);
    M5.Lcd.drawString(buf,49,yp2,2);
    M5.Lcd.drawLine(51, yp2, 60, yp2, GREEN);
    M5.Lcd.setTextColor(RED);
    sprintf(buf, "%d",pres_UB);
    M5.Lcd.drawString(buf,49,yub,2);
    M5.Lcd.drawLine(51, yub, 60, yub, RED);
    sprintf(buf, "%d",pres_LB);
    M5.Lcd.drawString(buf,49,ylb,2);
    M5.Lcd.drawLine(51, ylb, 60, ylb, RED);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setTextDatum(MC_DATUM);
    int x = 60, y = getY(pressures[0],pmin,unit,180);
    for (int i = 1;i<np;i++)
    {
      int xx = x + 3;
      int yy = getY(pressures[i],pmin,unit,180);
      M5.Lcd.drawLine(x, y, xx, yy, GREEN);
      if (i%8==0||(i==np-1&&i>66))
      {
        sprintf(buf, "%d",hours[i]);
        M5.Lcd.drawLine(xx, 180, xx, 189, GREEN);
        M5.Lcd.drawString(buf,xx,198,2);
      }
      x = xx;
      y = yy;
    }
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString(".",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 41; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 43; drawScreen=1; timer2 = millis();}
}
void screen43()
{
  int day_min = 0,day_max = 0,night_min = 0,night_max = 0, sum_night=0,sum_day=0;
  float avg_night = 0,avg_day = 0;
  int n_day = 0, n_night = 0;
  if (drawScreen) {
    drawScreen--;
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Reading...",160,0,2);
    readHumidities();
    readHours();
  if (n_humidities == n_hours)
  {
      for (int i=0;i<n_humidities;i++)
      {
          int hour = hours[i];
          int hum = humidities[i];
          if (hour>=21||hour<=8)
          {
            if (n_night==0) {night_min = hum; night_max = hum;}
            else 
              {
                if (hum>night_max) {night_max = hum;}
                if (hum<night_min) {night_min = hum;}
              }
            sum_night = sum_night + hum;
            n_night++;
            }
          else
          {
            if (n_day==0) {day_min = hum; day_max = hum;}
            else 
              {
                if (hum>day_max) {day_max = hum;}
                if (hum<day_min) {day_min = hum;}
              }
            sum_day = sum_day + hum;
            n_day++;
           }
            
      }
      if (n_night!=0) {avg_night = sum_night / n_night;}
      if (n_day!=0) {avg_day = sum_day / n_day;}
  }

    char temp_buf[15];
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("hum. stats [%]",160,40,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(ML_DATUM);
    M5.Lcd.drawString("Day",60,75,4);
    sprintf(temp_buf, "max. %d", day_max);
    M5.Lcd.drawString(temp_buf,30,115,4);
    sprintf(temp_buf, "min. %d", day_min);
    M5.Lcd.drawString(temp_buf,30,145,4);
    sprintf(temp_buf, "avg. %.1f", avg_day);
    M5.Lcd.drawString(temp_buf,30,175,4);
    M5.Lcd.setTextDatum(MR_DATUM);
    M5.Lcd.drawString("Night",260,75,4);
    M5.Lcd.setTextDatum(ML_DATUM);
    sprintf(temp_buf, "max. %d", night_max);
    M5.Lcd.drawString(temp_buf,200,115,4);
    sprintf(temp_buf, "min. %d", night_min);
    M5.Lcd.drawString(temp_buf,200,145,4);
    sprintf(temp_buf, "avg. %.1f", avg_night);
    M5.Lcd.drawString(temp_buf,200,175,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString(".",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 42; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 44; drawScreen=1; timer2 = millis();}
}
void screen44()
{
  int hmin = 0,hmax = 0;
  int nh = 0;
  if (drawScreen) {
    drawScreen--;
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Reading...",160,0,2);
    readHumidities();
    readHours();   
      for (int i=0;i<n_humidities;i++)
      {
          int hum = humidities[i];
            if (nh==0) {hmin = hum; hmax = hum;}
            else 
              {
                if (hum>hmax) {hmax = hum;}
                if (hum<hmin) {hmin = hum;}
              }
              nh++;         
      }
    
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.setTextDatum(MC_DATUM);
   char buf[30];
    sprintf(buf, "hum. [%%]");
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString(buf,10,0,2);
    hmax = round(hmax);
    hmin = round(hmin);
    if (hmin>hum_LB) {hmin = hum_LB;}
    if (hmax<hum_UB) {hmax = hum_UB;}
    int unit;
    if (hmax!=hmin) {unit = 160/(hmax-hmin);}
    else {unit = 1;}
    int h1 = hmin + (hmax-hmin)/3;
    int h2 = hmax - (hmax-hmin)/3;
    M5.Lcd.drawLine(60, 20, 60, 189, CYAN); // y
    M5.Lcd.drawLine(51, 180, 300, 180, CYAN); //x
    int ylb = getY(hum_LB,hmin,unit,180);
    int yub = getY(hum_UB,hmin,unit,180);
    M5.Lcd.drawLine(51, ylb, 300, ylb, RED); 
    M5.Lcd.drawLine(51, yub, 300, yub, RED); 
    if (n_hours>0) {
     sprintf(buf, "%d",hours[0]);
    M5.Lcd.drawString(buf,60,198,2);
    }
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.setTextDatum(MR_DATUM);
    sprintf(buf, "%d",hmin);
    M5.Lcd.drawString(buf,49,180,2);
    sprintf(buf, "%d",hmax);
    int ymax = getY(round(hmax),hmin,unit,180);
    M5.Lcd.drawString(buf,49,ymax,2);
    M5.Lcd.drawLine(51, ymax, 60, ymax, CYAN);  
    int yh1 = getY(h1,hmin,unit,180);
    int yh2 = getY(h2,hmin,unit,180);
    sprintf(buf, "%d",h1);
    M5.Lcd.drawString(buf,49,yh1,2);
    M5.Lcd.drawLine(51, yh1, 60, yh1, CYAN);
    sprintf(buf, "%d",h2);
    M5.Lcd.drawString(buf,49,yh2,2);
    M5.Lcd.drawLine(51, yh2, 60, yh2, CYAN);
    M5.Lcd.setTextColor(RED);
    sprintf(buf, "%d",hum_UB);
    M5.Lcd.drawString(buf,49,yub,2);
    M5.Lcd.drawLine(51, yub, 60, yub, RED);
    sprintf(buf, "%d",hum_LB);
    M5.Lcd.drawString(buf,49,ylb,2);
    M5.Lcd.drawLine(51, ylb, 60, ylb, RED);
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.setTextDatum(MC_DATUM);
    int x = 60, y = getY(humidities[0],hmin,unit,180);
    for (int i = 1;i<nh;i++)
    {
      int xx = x + 3;
      int yy = getY(humidities[i],hmin,unit,180);
      M5.Lcd.drawLine(x, y, xx, yy, CYAN);
      if (i%8==0||(i==nh-1&&i>66))
      {
        sprintf(buf, "%d",hours[i]);
        M5.Lcd.drawLine(xx, 180, xx, 189, CYAN);
        M5.Lcd.drawString(buf,xx,198,2);
      }
      x = xx;
      y = yy;
    }
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString(".",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 43; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 38; drawScreen=1; timer2 = millis();}
}
void screen46()
{
   if (drawScreen) {
    drawScreen--;
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(3);
    M5.Lcd.drawString("Return",160,90,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString("OK",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 48; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {menu_stan = 8; drawScreen=1; timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 47; drawScreen=1; timer2 = millis();}
}
void screen47()
{
    if (drawScreen)
    {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Reading...",160,0,2);
    forecast();
    if (n_hours==72)
    {
      drawScreen--;
    char buf[15];
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Forecast(today)",160,40,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(ML_DATUM);
    M5.Lcd.drawString("Day",120,75,4);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.drawString("T[C]",30,115,4);
    sprintf(buf,"%.1f",tempD1);
    M5.Lcd.drawString(buf,125,115,4);
    M5.Lcd.setTextColor(CYAN);
    sprintf(buf,"%d",humD1);
    M5.Lcd.drawString(buf,125,145,4);
    M5.Lcd.drawString("h[%]",30,145,4);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.drawString("p[hPa]",30,175,4);
    M5.Lcd.setTextDatum(MR_DATUM);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.drawString("Night",280,75,4);
    M5.Lcd.setTextColor(YELLOW);
    sprintf(buf,"%.1f",tempN1);
    M5.Lcd.drawString(buf,270,115,4);
    M5.Lcd.setTextColor(CYAN);
    sprintf(buf,"%d",humN1);
    M5.Lcd.drawString(buf,270,145,4);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextColor(GREEN);
    sprintf(buf,"%d",pres1);
    M5.Lcd.drawString(buf,195,175,4);
    }
    else
    {
      drawScreen--;
      M5.Lcd.fillScreen(BLACK);
      showBatteryLevelAndNetworkStatus();
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setTextDatum(MC_DATUM);
      M5.Lcd.setTextSize(1);
      M5.Lcd.drawString("Forecast(today)",160,40,4);
      M5.Lcd.setTextSize(2);
      M5.Lcd.drawString("Not enough",160,100,4);
      M5.Lcd.drawString("data",160,155,4);
    }
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString(".",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 46; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 48; drawScreen=1; timer2 = millis();}
}
void screen48()
{
    if (drawScreen)
    {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Reading...",160,0,2);
    forecast();
    if (n_hours==72)
    {
      drawScreen--;
    char buf[15];
    M5.Lcd.fillScreen(BLACK);
    showBatteryLevelAndNetworkStatus();
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(1);
    M5.Lcd.drawString("Forecast(tommorow)",160,40,4);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(ML_DATUM);
    M5.Lcd.drawString("Day",120,75,4);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.drawString("T[C]",30,115,4);
    sprintf(buf,"%.1f",tempD2);
    M5.Lcd.drawString(buf,125,115,4);
    M5.Lcd.setTextColor(CYAN);
    sprintf(buf,"%d",humD2);
    M5.Lcd.drawString(buf,125,145,4);
    M5.Lcd.drawString("h[%]",30,145,4);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.drawString("p[hPa]",30,175,4);
    M5.Lcd.setTextDatum(MR_DATUM);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.drawString("Night",280,75,4);
    M5.Lcd.setTextColor(YELLOW);
    sprintf(buf,"%.1f",tempN2);
    M5.Lcd.drawString(buf,270,115,4);
    M5.Lcd.setTextColor(CYAN);
    sprintf(buf,"%d",humN2);
    M5.Lcd.drawString(buf,270,145,4);
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextColor(GREEN);
    sprintf(buf,"%d",pres2);
    M5.Lcd.drawString(buf,195,175,4);
    }
    else
    {
      drawScreen--;
      M5.Lcd.fillScreen(BLACK);
      showBatteryLevelAndNetworkStatus();
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setTextDatum(MC_DATUM);
      M5.Lcd.setTextSize(1);
      M5.Lcd.drawString("Forecast(tomorrow)",160,40,4);
      M5.Lcd.setTextSize(2);
      M5.Lcd.drawString("Not enough",160,100,4);
      M5.Lcd.drawString("data",160,155,4);
    }
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextDatum(BL_DATUM);
    M5.Lcd.drawString("<",45,240,4);
    M5.Lcd.setTextDatum(BC_DATUM);
    M5.Lcd.drawString(".",160,240,4);
    M5.Lcd.setTextDatum(BR_DATUM);
    M5.Lcd.drawString(">",275,240,4);
    }
    if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&focus) {drawScreen=1; timer2 = millis();}
    else if (((millis()-timer2>=timerLimit2)||(millis()-timer2<0))&&!focus) {menu_stan = 1; drawScreen=1; timer1=millis();timer2=0;}
    else if (M5.BtnA.wasPressed()) {menu_stan = 47; drawScreen=1; timer2 = millis();}
    else if (M5.BtnB.wasReleased()) {timer2 = millis();}
    else if (M5.BtnC.wasPressed()) {menu_stan = 46; drawScreen=1; timer2 = millis();}
}
