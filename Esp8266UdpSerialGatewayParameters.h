#define nbUdpHost 4
#define nbUdpPort 4
#define routePort  1830; // server robot
#define tracePort  1831;  // server trace
#define power_PIN 5  // relay off if wifi connection not established
#define debug_PIN 14 // switch of udp trace when grounded 
#define led_PIN 15  // off if wifi connection not established
byte robotIP[4] = {0xc0, 0xa8, 0x01, 0x05};  //
unsigned int udpListenPort = 8888;
char ssid1[] = "yourfirstSSID";       // first SSID  will be used at first
char pass1[] = "xxxxxxx";      // first pass
char ssid2[] = "yoursecondSSID";        // second SSID will be used in case on connection failure with the first ssid
char pass2[] = "yyyyyyyyyyyyyyyyy";  // second pass