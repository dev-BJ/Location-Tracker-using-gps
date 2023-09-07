#include <Thread.h>
#include <AltSoftSerial.h>
#include <TinyGPS++.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

SoftwareSerial Gsm(6, 7);
AltSoftSerial gps; //RX, TX
Thread myThread = Thread();

TinyGPSPlus gps_;

char phone_no[] = "+234xxxxxxxx";

double lat_v, lng_v;

int state;
String sname = "John Doe";
String loc = "https://maps.google.com/maps?q=loc:" + String((float)lat_v, 6) + "," + String((float)lng_v, 6);

void setup() {
  String textMessage;
  pinMode(10, INPUT);
  pinMode(11, INPUT);

  Serial.begin(9600);

  gps.begin(9600);
  delay(2000);
  Gsm.begin(9600);
  delay(2000);


  Serial.println("SIM800 ready...");
  Gsm.print("AT+CMGF=1\r");

  delay(1000);
  Gsm.print("AT+CNMI=2,2,0,0,0\r");
  if (Gsm.available() > 0) {
    Serial.println("Setup GSM data available");
    textMessage = Gsm.readString();
    Serial.println(textMessage);

  }

  gprs_setup();

  delay(5000);
  //myThread.onRun([](){
  myThread.onRun(sendData);
  // Serial.println("Thread Working");

  // });
  //     myThread.onRun(sed);
  myThread.setInterval(300000);
  Serial.println("Ready");
}

void loop() {
  String textMessage;

  if (myThread.shouldRun())
    myThread.run();

  while (gps.available() > 0) {
    //    Serial.println("GPS data");
    //    Serial.write(gps.read());

    if (gps_.encode(gps.read())) {
      gpsInfo(&lat_v, &lng_v);
    }

  }

  //  Serial.print(lat_v, 6);
  //  Serial.print(F(","));
  //  Serial.println(lng_v, 6);

  while (Gsm.available()) {
    Serial.println("GSM data available");
    textMessage = Gsm.readString();
    textMessage.trim();
    Serial.println(textMessage);
  }

  if (textMessage.indexOf("getcall") >= 0) {
    callm();
  } else if (textMessage.indexOf("getsms") >= 0) {
    sms();
  }

  state = digitalRead(10);
  if (state == 0)
  {
    sms();
    delay(200);
    Gsm.println();
    Serial.println("SMS Sent");

  } else {
    delay(10);
  }
  state = digitalRead(11);
  if (state == 0)
  {
    callm();

    delay(200);
    Gsm.println();
    Serial.println("Calling");

  } else {
    delay(10);
  }
  //  Serial.println(failed);

}

void sms() {

  // AT command to set SIM800 to SMS mode
//    Gsm.print("AT+CMGF=1\r");
  //  delay(100);
  Gsm.print("AT+CMGS=\"");
  Gsm.print(phone_no);
  Gsm.println("\"");
  // Send the SMS
  Gsm.println("Alert Please I need help.............");

  Serial.println(loc);
  //  Gsm.print("https://maps.google.com/maps?q=loc:");
  //  Gsm.print(lat_v, 6);
  //  Gsm.print(",");
  //  Gsm.println(lng_v, 6);
  Gsm.println(loc);

  delay(200);
  Gsm.println((char)26);
  Gsm.println();
  // Give module time to send SMS
  delay(1000);

}

void callm() {
  Serial.println("Calling");
  delay(10000);
  Gsm.println("ATD+2347038557674;");
  delay(20000);
  Gsm.println("ATH");
  delay(1000);
}

void gpsInfo(double* lat_v, double* lng_v)
{

  if (gps_.location.isValid())
  {
    *lat_v = ( gps_.location.lat());
    *lng_v = ( gps_.location.lng());
  }
  //  else
  //  {
  //    Serial.println(F("INVALID"));
  //  }
}
void sendData() {
  Serial.println("SendData called");

  gprs_cmd("AT+SAPBR=1,1");

  gprs_cmd("AT+SAPBR=2,1");

  gprs_cmd("AT+HTTPINIT");

  gprs_cmd("AT+HTTPPARA=CID,1");
  String url = "AT+HTTPPARA=\"URL\",\"http://feeltechinstitute.com/updatelocation.php?sname=" + sname + "&location=" + loc + "\"";
  Serial.println(url);
  gprs_cmd(url);

  //  gprs_cmd("AT+HTTPPARA=CONTENT,application/x-www-form-urlencoded");
  //
  //  gprs_cmd("AT+HTTPDATA=602,10000");

  //  gprs_cmd();

  gprs_cmd("AT+HTTPACTION=0");

  gprs_cmd("AT+HTTPREAD");
  delay(2000);
  gprs_cmd("AT+HTTPTERM");

  gprs_cmd("AT+SAPBR=0,1");

}

void gprs_setup() {
  gprs_cmd("AT+CSQ");

  gprs_cmd("AT+CGATT?");

  gprs_cmd("AT+SAPBR=3,1,CONTYPE,GPRS");

  gprs_cmd("AT+SAPBR=3,1,\"APN\",\"gloflat\"");

  gprs_cmd("AT+SAPBR=3,1,\"USER\",\"flat\"");

  gprs_cmd("AT+SAPBR=3,1,\"PWD\",\"flat\"");

}

void gprs_cmd(String cmd) {
  Gsm.println(cmd);
  delay(2000);
  while (Gsm.available()) {
    Serial.write(Gsm.read());
  }
}
