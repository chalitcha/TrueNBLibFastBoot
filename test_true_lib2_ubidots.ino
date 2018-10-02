#include "bc95_true_nbiot.h"
#include <AltSoftSerial.h>

AltSoftSerial Serial2;
True_NB_bc95 modem(Serial2);    // Modem connection
IoTtweetNBIoT iotweet(&modem);  // Platform connection
ubidotsNBIoT ubidots(&modem);

/** IoTtweet platform **/
String tweet_serv_ip = "35.185.177.33";
int tweet_serv_port = 5683;
int sock_0 = 0; 
int port_0 = 4700; // local port

/** ubidots platform **/
String ubidots_serv_ip = "50.23.124.66"; /* translate.ubidots.com */
int ubidots_serv_port = 9012;
int sock_1 = 1;
int port_1 = 4701; // local port

String end_line = "\r\n";
long start = 0;

void setup()
{
  bool reg_success = false;

  Serial.begin(9600);   // Connect PC
  Serial2.begin(9600);  // connect BC95

  delay(1000);
  // Test class True_NB_bc95
  modem.pc_debug_enable(Serial, true); // Open debug from lib on Hardware serial
  modem.init_modem();

  delay(300);
  Serial.println( "IMSI: " + modem.get_imsi() );
  Serial.println( "IMEI: " + modem.get_imei() );

  Serial.print("register network");
  reg_success = modem.register_network2();
  if (reg_success == true) {
    Serial.println("[done]");
  }
  else {
    Serial.println("[failed]");
  }

  Serial.print("waiting IP");
  while ( !modem.get_network_status() ) {
    Serial.print(".");
  }
  Serial.println("\r\nIP: " + modem.get_ip_address() );

  Serial.println("RSSI:" + String(modem.get_modem_signal()) );
  Serial.println("initial success\r\n");

  iotweet.init(tweet_serv_ip, tweet_serv_port);
  ubidots.init(ubidots_serv_ip, ubidots_serv_port);
  
  Serial.println( "start udp socket id: " + String(modem.create_udp_socket(port_0, sock_0)) + ", port: " + String(port_0) ); // local port
  Serial.println( "start udp socket id: " + String(modem.create_udp_socket(port_1, sock_1)) + ", port: " + String(port_1) ); // local port
  Serial.println("ready to send data\r\n");
}

void loop()
{
  /** data iottweet **/
  float data0, data1, data2, data3;   /* Your sending data variable. */
  String userid = "002459";           /* IoTtweet account user ID (6 digits, included zero pre-fix) ex.002xxx */
  String key = "xxxxxxxx";        /* IoTtweet registered device key in "MY IOT Garage" */
  String private_tweet = "TTT";      /* Your private tweet meassage to dashboard */
  String public_tweet = "Hi02";     /* Your public tweet message to dashboard */

  /** data ubidot **/
  String useragent = "nbiot2018/1.1";
  String token = "xxxxxxxxxxx"; // Assign your Ubidots TOKEN
  String device_name = "nb001"; // Assign the unique device label (device name)  
  String val_name1 = "temperature:";
  String val_name2 = "humidity:";
  String data_u = "";

  if (millis() - start > 15000)
  {
    start = millis();

    /* Example data generating */
    data0 = random(20, 80);
    data1 = random(30, 70);
    data2 = random(40, 60);
    data3 = random(50, 55);
    data_u = val_name1 + String(data0) + "," + val_name2 + String(data1);
    
    /* Send data to IoTtweet dashboard */
    iotweet.write_dashboard_IoTtweet(userid, key, data0, data1, data2, data3, private_tweet, public_tweet, sock_0); // socket id 0
    delay(500);
    ubidots.write_dashboard_ubidotsNBIoT(useragent, token, device_name, data_u, sock_1);
  }
}
