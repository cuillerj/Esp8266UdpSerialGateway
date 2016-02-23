// can transfer any 2 bytes binary data 
// forbidenc 3 consecutive bytes "0x7f7f7f & 0x7f7e7f"
#define debug 0
String ver = "UdpSerialGateway";
uint8_t vers = 0x02;
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <Time.h>
#include <stdio.h>
#include <string.h>
#include <C:\Users\jean\Documents\Arduino\libraries\Esp8266JC\Esp8266ReadEeprom.h>
#define nbUdpHost 4
#define nbUdpPort 4
#define routePort  1830; // server robot
#define tracePort  1831;  // server trace
#define MAX_SRV_CLIENTS 1
#define power_PIN 5  // relay off if wifi connection not established
#define debug_PIN 14 // switch of udp trace when grounded 
#define led_PIN 15  // off if wifi connection not established
char hostUdp[nbUdpHost][100];
char defaultHostUdp[100] = "jean-PC";
unsigned int defaultudpPort0 = 1830; // server robot
unsigned int defaultudpPort1 = 1831;  // server trace
unsigned int defaultudpPort2 = 1832; // reserved
unsigned int defaultudpPort3 = 1833;  // reserved
unsigned int udpPort[nbUdpPort];
String services[] = {"Route", "Trace"};
byte robotIP[4] = {0xc0, 0xa8, 0x01, 0x05};  //
unsigned int udpListenPort = 8888;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
//how many clients should be able to telnet to this ESP8266
byte bufUdp[255];
// wifi
const char* Pssid ;
const char* Ppassword ;
char ssid[20] ;
char password[50] ;
char stationId[5];

//const int UDP_TX_PACKET_MAX_SIZE = 1024; // NTP time stamp is in the first 48 bytes of the message

// serial fields
#define sizeDataBin 255
byte dataBin[sizeDataBin];
#define maxUDPResp  100
char resp[maxUDPResp] = "";
int frameLen = 0;
int frameCount = 0;
uint8_t frameFlag = 0x00;
int udpFlag = 0;
//String lenS[100];
byte bufParam[5];

byte bufSerialOut[maxUDPResp + 10];

//
// timers
unsigned long timeSerial = 0;
unsigned long timeMemory = 0;
unsigned long timeUdp = 0;
unsigned long timeLastSentUdp = 0;
unsigned long timeCheckWifi;
// internal data
uint8_t Diag = 0x93; // (Wifi,NA,NA,TimeUpTodate,NA,NA,schedulUpload,Memory)
uint8_t SavDiag = Diag;
unsigned int freeMemory = 0;
int id;
//
WiFiUDP Udp;
//WiFiServer server(888);
//WiFiClient serverClients[MAX_SRV_CLIENTS];
//WiFiClient client;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  EEPROM.begin(200);
  //  strcpy (hostUdp[0] , defaultHostUdp); // default server could be updated by services
  //  IPAddress ipRobot=(192,168,1,5);
  //  strcpy (hostUdp[1] , defaultHostUdp); // default server could be updated by services
  udpPort[0] = routePort; //
  udpPort[1] = tracePort; //
  //  Serial.print(ver);
  //  Serial.println(vers);
  // printf("%p\n", (void*) ssid);
  Pssid = &ssid[0];
  char* ad = GetParam(0);
  //  Serial.print("address:");
  for (int i = 1; i < ad[0] + 1; i++)
  {
    //    Serial.print(ad[i], HEX);
    //    Serial.print(":");
  }

  id = ad[3] * 256 + ad[4];

  itoa(id, stationId, 10); // here 10 means base 10
  //  Serial.println(stationId);

  //  Serial.print("type:");

  char* ty = GetParam(1);
  for (int i = 1; i < ty[0] + 1; i++)
  {
    //    Serial.print(ty[i], HEX);
    //    Serial.print(":");
  }
  //  Serial.println();
  //  Serial.print("ssid:");
  char* ss = GetParam(2);
  for (int i = 1; i < ss[0] + 1; i++)
  {
    //   Serial.print(ss[i]);
    ssid[i - 1] = ss[i];
  }
  //  Serial.println();
  //  Serial.print("password:");
  char* ps = GetParam(3);
  for (int i = 1; i < ps[0] + 1; i++)
  {
    //   Serial.print(ps[i]);
    password[i - 1] = ps[i];
  }

  //  Serial.println();
  Pssid = &ssid[0];
  Ppassword = &password[0];
  ConnectWifi();
  WiFi.mode(WIFI_STA);
  //  server.begin();
  //  server.setNoDelay(true);
  //  Serial.print("Ready! Use ");
  //  Serial.print(WiFi.localIP());
  //  Serial.println(" 888' to connect");
  // PrintUdpConfig();

  Udp.begin(udpListenPort);
  delay (5000);
  //  WiFi.printDiag(Serial);
  TraceToUdp(Pssid, 0x01);
  TraceToUdp("Ready! Use ", 0x02);
  TraceToUdp(" 888' to connect", 01);
  pinMode(power_PIN, OUTPUT);
  digitalWrite(power_PIN, true);
  pinMode(led_PIN, OUTPUT);
  digitalWrite(led_PIN, true);
  pinMode(debug_PIN, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:

  // delay(20);
  if (millis() - timeSerial >= 20)
  {
    int lenInput = Serial_have_message();
    {
      if (lenInput != 0)
      {
        //        TraceToUdp("len:",0x01);
        String   lenS = "serial len:" + String(lenInput);
        TraceToUdp(lenS, 0x01);
        RouteToUdp(lenInput);
      }
    }
    timeSerial = millis();
  }


  if (millis() - timeMemory >= 5000)
  {
    freeMemory = ESP.getFreeHeap();
    //    TraceToUdp("free memory:",0x02);
    String  strS = "free memory:" + String(freeMemory);
    TraceToUdp(strS, 0x01);
    //    Serial.print("free memory:");
    //   Serial.println(lenS);
    timeMemory = millis();
  }
  if (millis() - timeUdp >= 100)
  {
    InputUDP();
    timeUdp;
  }
}

void ConnectWifi()
{
  timeCheckWifi = millis();
  WiFi.begin(Pssid, Ppassword);
  //  Serial.print("\nConnecting to "); Serial.println(Pssid);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 5) delay(2000);
  bitWrite(Diag, 7, 0);       // position bit diag
  if (i == 5) {
    //    Serial.print("Could not connect to"); Serial.println(Pssid);
    while (1) delay(500);
    bitWrite(Diag, 7, 1);       // position bit diag
    digitalWrite(power_PIN, false);
    digitalWrite(led_PIN, false);
  }
}
void PrintUdpConfig() {
  for (int i = 0; i < (nbUdpPort); i++)
  {
    Serial.print(services[i]);
    Serial.print(" Udp");
    Serial.print(i);
    Serial.print(" ");
    //    Serial.print(hostUdp[i]);
    Serial.print(" port:");
    Serial.println(udpPort[i]);

  }
}

void SendToUdp( int mlen, int serviceId) {
  if ( millis() - timeLastSentUdp  > 200)
  {
    Udp.beginPacket(robotIP, udpPort[serviceId]);
    Udp.write(dataBin, mlen);
    Udp.endPacket();
  }
  else
  {
    delay (200);
    Udp.beginPacket(robotIP, udpPort[serviceId]);
    Udp.write(dataBin, mlen);
    Udp.endPacket();
  }
  timeLastSentUdp = millis();
}
void TraceToUdp(String req, uint8_t code)
{
  if ( digitalRead(debug_PIN) != 0)
  {

    int len = req.length() + 5;
    dataBin[0] = uint8_t(id / 256); // addrSSerial
    dataBin[1] = uint8_t(id);  // addrMSerial
    dataBin[2] = code;  // Print
    dataBin[3] = 0x00;

    if (len > sizeDataBin)
    {
      len = sizeDataBin;
    }
    dataBin[4] = uint8(len - 5); //len
    for (int i = 0; i < len - 5; i++)
    {

      dataBin[i + 5] = req[i];

    }


    SendToUdp(len, 1);
  }
}
void RouteToUdp(int len)
{
  //byte dataBin[100];
  dataBin[0] = bufParam[0];
  dataBin[1] = 0x3B;
  dataBin[2] = 0x64;
  dataBin[3] = 0x3B;
  dataBin[4] = uint8(len);  //len
  dataBin[5] = 0x3B;
  // Serial.println(len);
  if (len > sizeDataBin - 6)
  {
    len = sizeDataBin - 6;

  }
  for (int i = 0; i < len; i++)
  {
    dataBin[i + 6] =   bufUdp[i];


  }
  SendToUdp( len + 6 , 0);
}
int Serial_have_message() {
  if (Serial.available() )
  {
    //  delay(20);
    //    TraceToUdp("serial2");
    while (Serial.available())
    {
      byte In1 = (Serial.read());
      //     TraceToUdp("in1");
      //   Serial.print(In1, HEX);
      //   Serial.print("-");
      switch (In1)
      {
        case 0x7f:  // looking for start of frame "0x7f-0x7e-0x7f-0x7e"
          if (frameFlag == 0)  // first 0x7f in the frame
          {
            frameFlag = 1;
            break;
          }
          if (frameFlag == 2)
          {
            frameFlag = 3;  // 0x7f after 0x7f-0x7e
            break;
          }
          if (frameFlag == 4)
          {
            frameFlag = 5;
            break;
          }
          if (frameFlag == 6)
          {
            frameFlag = 7;
            break;
          }

        //  }
        case 0x7e:
          if (frameFlag == 3)   // last 0x7e start frame completed
          {
            frameFlag = 8;
            //           TraceToUdp(" +");
            frameLen = 0;
            frameCount = 0;
            udpFlag = 0;
            break;
          }
          if (frameFlag == 1)  // 0x7e after 0x7f
          {
            frameFlag = 2;
            break;
          }
          if (frameFlag == 5)
          {

            frameFlag = 6;
            break;
          }
          if (frameFlag == 7)
          {
            frameFlag = 8;
            //           TraceToUdp(" +");
            frameLen = 0;
            frameCount = 0;
            udpFlag = 0;
            break;
          }

        default:
          //                                printf(" flag:%d",frameFlag);

          if (frameFlag == 5 || frameFlag == 6 || frameFlag == 7 || frameFlag == 8)
          {
            frameFlag = 4;
            break;
          }
      }

      switch (frameFlag)
      {

        case 4:  // useful data
        case 5:  // useful data  added 23/02/2016
        case 6:  // useful data  added 23/02/2016
          frameCount = frameCount + 1;
          switch (frameCount)
          {
            case 1:
              //             TraceToUdp(" addrS:%d");
              bufParam[0] = In1;   // get the serial source address
              break;
            case 2:
              //              TraceToUdp(" addrM:%d");
              bufParam[1] = In1; // get the serial master  address - not currently used
              break;
            case 3:

              //             TraceToUdp(" req:%d");
              bufParam[2] = In1;  // get the request type
              break;
            case 4:              // get the ack flag
              bufParam[3] = In1;
              //             TraceToUdp(" acq:%d");
              break;
            case 5:              // get the frame len
              frameLen = In1;
              bufParam[4] = In1;
              //              TraceToUdp(" len:%d");
              break;
            default:
              bufUdp[frameCount - 6] = In1; //
              if (frameCount == (frameLen + 5) && udpFlag == 0 && bufParam[2] != 0xff)
              {
                //            RouteToUdp(frameLen);
                udpFlag = 0xff;
                //                TraceToUdp("sendUdp");
              }
              if (bufParam[2] == 0xff)
              {
                udpFlag = 1;
              }

              break;


          }

        default:

          break;
      }
    }
    //end of for all chars in string
  }

  if (udpFlag == 0xff)  // end of frame reached

  {
    udpFlag = 0x00;
    int retLen = frameLen;
    frameLen = 0;
    frameCount = 0;
    frameFlag = 0x00;
    return (retLen);

  }
  else
  {
    return (0);
  }
}
void InputUDP() {
  int packetSize = Udp.parsePacket();  // message from UDP

  if (packetSize)
  {
    //  TraceToUdp("udp input", 0x02);
    String lenS = "udp input" + String(packetSize);
    TraceToUdp(lenS, 0x01);
    bufSerialOut[1] = 0x7f;
    bufSerialOut[2] = 0x7e;
    bufSerialOut[3] = 0x7f;
    bufSerialOut[4] = 0x7e;

    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    //   int packetBufferLen=sizeof(packetBuffer);
    int packetBufferLen = packetSize;
    bufSerialOut[5] = packetSize;
    for (int i = 0; i < maxUDPResp; i++)
    {
      bufSerialOut[i + 6] = 0x00;
    }

    for (int i = 0; i < packetSize; i++)
    {

      bufSerialOut[i + 6] = packetBuffer[i];
      //     Serial.print(packetBuffer[i],HEX);
      //     Serial.print(":");
    }
    if (packetSize < 6)
    {
      packetSize = 6;
    }

    Serial.write(bufSerialOut, packetSize + 6);

  }
}
