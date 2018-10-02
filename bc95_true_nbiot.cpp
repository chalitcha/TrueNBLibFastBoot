#include "bc95_true_nbiot.h"

/************
 * Constructure
 ************/

/*True_NB_bc95::True_NB_bc95()
{
  debug_flag = false;
}*/
True_NB_bc95::True_NB_bc95(Stream &serial)
{
  this->init_serial(serial);
  debug_flag = false;
}

IoTtweetNBIoT::IoTtweetNBIoT(True_NB_bc95 *nb)
{
  nb_bc95 = nb;
}

ubidotsNBIoT::ubidotsNBIoT(True_NB_bc95 *nb)
{
  nb_bc95 = nb;
}

/***********
 * Common
 ***********/
void True_NB_bc95::pc_debug_enable(Stream &serial, bool flag1)
{
  DEBUG_SERIAL = &serial;
  debug_flag = flag1;
}

void True_NB_bc95::debug_print(String str_out)
{
  if ( debug_flag == true ) { // debug enable
    DEBUG_SERIAL->print(str_out);
  }
}

void True_NB_bc95::debug_println(String str_out)
{
  if ( debug_flag == true ) { // debug enable
    DEBUG_SERIAL->println(str_out);
  }
}

void True_NB_bc95::clear_buff()
{
  char c;
  while (MODEM_SERIAL->available()) {
    c = MODEM_SERIAL->read();
  }
}

void True_NB_bc95::init_serial(Stream &serial)
{
  MODEM_SERIAL = &serial;
}


bool True_NB_bc95::init_modem()
{
  debug_println("## True_NB_BC95 Library by TrueIoT V1.01 ##");
  debug_print( "initial modem" );

  // wait modem
  while ( !this->get_modem_status() ) {
    clear_buff();
    debug_print(".");
  }

  this->set_auto_connect();
  this->set_phon_function();
  debug_println( "[done]" );
}

bool True_NB_bc95::reboot_modem()
{
  debug_print( "reboot modem" );
  MODEM_SERIAL->println(F("AT+NRB"));
  delay(300);

  while ( !this->get_modem_status() ) {
    clear_buff();
    debug_print(".");
  }
  debug_println("[done]");

  init_modem();
}


String True_NB_bc95::expect_rx_str( unsigned long period, char exp_str[], int len_check)
{
  unsigned long cur_t = millis();
  unsigned long start_t = millis();
  bool str_found = 0;
  bool time_out = 0;
  bool loop_out = 0;
  int i = 0;
  int found_index = 0, end_index = 0;
  int modem_said_len = 0;
  char c;
  char modem_said[MODEM_RESP];
  char str[BUF_MAX_SIZE];
  char *x;
  String re_str;

  while ( !loop_out )
  {
    if (MODEM_SERIAL->available()) {
      c = MODEM_SERIAL->read();
      modem_said[i++] = c;
    }
    else {
    }
    cur_t = millis();
    if ( cur_t - start_t > period ) {
      time_out = true;
      start_t = cur_t;
      loop_out = true;
    }
  } //while

  modem_said[i] = '\0';
  end_index = i;
  x = strstr(modem_said, exp_str) ;

  found_index = x ? x - modem_said : -1;

  if ( found_index >= 0  )
  {
    i = 0;
    while ( modem_said[found_index + i + len_check] != 0x0D | i == 0) {
      str[i] = modem_said[found_index + i + len_check];
      re_str += String(str[i]);
      i++;
    }
    str[i] = '\0';

    if (re_str == "\r\nERROR") return "999";
    return re_str;
  }
  return "";
}

/***
 * find_rx_bc()
 * return: >=0: string found, index of string
 *          -1: string not found
 *          -2: Error
 ***/
int True_NB_bc95::find_rx_bc(long tout, String str_wait)
{
  unsigned long start_t = millis();
  unsigned long cur_t = millis();
  String input = "";
  char bc_rx[MODEM_RESP];
  bool flag_end = false;
  int index_str = -1;
  int index_tmp = -1;
  int re_len = 0;

  while (!flag_end)
  {
    // find string
    re_len = this->bc95_response(&bc_rx[0]);

    if (re_len > 0)
    {
      input = String(bc_rx);
      if (index_tmp = input.indexOf(str_wait) >= 0) { // found string
        index_str = index_tmp; // Index of string
        return index_str;
      }
      else if (input.indexOf(F("ERROR")) >= 0) { // error
        index_str = -2; // Error
        return index_str;
      }
    }

    // check timeout
    cur_t = millis();
    if ( flag_end == false)
    {
      if (cur_t - start_t >= tout) {
        flag_end = true;
        index_str = -1; // string not found
        start_t = cur_t;
      }
    }

  }// while

  return index_str;
}


/*********
 * General function
 *********/
String True_NB_bc95::get_imei()
{
  MODEM_SERIAL->println(F("AT+CGSN=1"));
  return expect_rx_str(500, "+CGSN:", 6);
}


String True_NB_bc95::get_imsi()
{
  MODEM_SERIAL->println(F("AT+CIMI"));
  return expect_rx_str(500, "\r\n", 2);
}


String True_NB_bc95::get_ip_address()
{
  MODEM_SERIAL->println(F("AT+CGPADDR=0"));
  delay(500);
  return expect_rx_str(500, "+CGPADDR:0,", 11 );
}


/****  AT Command direct ****
 * bc95_send_AT_cmd() : for send direct and get response with BC95
 * input : AT Command set, response string
 * Output : String response from BC95 (res_str) and length
 ****/
int True_NB_bc95::bc95_send_AT_cmd(String at_cmd, char *res_str /*out put*/)
{
  int re_len = 0;

  MODEM_SERIAL->println(at_cmd);
  delay(1300);
  re_len = bc95_response(res_str);
  return re_len;
}


int True_NB_bc95::bc95_response(char *modem_said /*out put*/)
{
  int end_index = 0;

  while ( MODEM_SERIAL->available() )
  {
    modem_said[end_index++] = MODEM_SERIAL->read();
  }

  modem_said[end_index] = '\0';
  return end_index;
}


/************************
 * Network connection
 ************************/

/***
 *  register_network2()
 *  retur: true - network register done, else - unsuccess
 ***/
bool True_NB_bc95::register_network2()
{
  bool res_stts = false;
  char waitForCGATT[] = "OK";
  int num_recon = 0;

  MODEM_SERIAL->println(F("AT+CGATT=1")); // connecting
  //MODEM_SERIAL->println(F("AT+COPS=1,2,\"52004\"")); // connecting
  delay(1100);

  if (find_rx_bc(500, waitForCGATT) >= 0 )
  {
    do { // Limit reconnect 10 time
      debug_print( "." );
      num_recon++;
      res_stts = this->get_network_status();
    }
    while ( !res_stts && num_recon < 10 );

    if ( num_recon > 10) {
      return false; // Failed - reconnect more than 10 time.
    }
    return true; // Attach success
  }
  return false; // Failed, can't set network parameter
}

/***
 * get_network_status()
 * return: true - Attached network, else - not register
 */
bool True_NB_bc95::get_network_status()
{
  int net_stts = -1;
  char waitForCGATT[] = "+CGATT:1";

  MODEM_SERIAL->println(F("AT+CGATT?")); // check status
  delay(1000);

  net_stts = find_rx_bc(500, waitForCGATT);
  //debug_println(String(net_stts));
  if (net_stts >= 0 ) {
    return true; // Attached (Connected)
  }
  return false; // Not register network
}


bool True_NB_bc95::register_network()
{
  bool regist = 0;
  char waitForCGATT[] = "+CGATT:1";

  MODEM_SERIAL->println(F("AT+CGATT=1")); // connectint
  delay(1100);
  MODEM_SERIAL->println(F("AT+CGATT?")); // check status
  delay(1100);

  if (expect_rx_str(500, waitForCGATT, 8 ) == "" ) {
    return false;
  }
  return true;
}

// Set Auto Connection
bool True_NB_bc95:: set_auto_connect()
{
  MODEM_SERIAL->println(F("AT+NCONFIG=AUTOCONNECT,TRUE"));
  delay(300);
  if ( find_rx_bc(500, "OK") >= 0) {
    return true;
  }
  return false;
}

// Set Phone Functionality
bool True_NB_bc95:: set_phon_function()
{
  MODEM_SERIAL->println(F("AT+CFUN=1"));
  delay(2000);
  if ( find_rx_bc(1000, "OK") >= 0) {
    return true;
  }
  return false;
}


/******************
 * Get board info
 ******************/

/***
 * get_modem_signal()
 * return: Signal strength, -200: no signal
 ***/
int True_NB_bc95::get_modem_signal()
{
  int ssi;
  char ssi_str[3];
  char waitForCSQ[] = "+CSQ:";
  String re_str;

  MODEM_SERIAL->println("AT+CSQ");
  delay(400);
  re_str = expect_rx_str( 500, waitForCSQ, 5);

  if ( re_str != "" )
  {
    ssi_str[0] = re_str[0];
    // check the next char is not "," It is not single digit
    if ( re_str[1] != 0x2c) {
      ssi_str[1] = re_str[1];
      ssi_str[2] = '\0';
      ssi = atoi(ssi_str);
      ssi = -1 * (113 - ssi * 2);
      return ssi;
    }
    // it is single digit
    ssi_str[1] = '\0';
    ssi = atoi(ssi_str);
    ssi = -1 * (113 - ssi * 2);
    return ssi;

  }

  return -200;

}

/*****
 * get_modem_status()
 * return: true - ready to use, else: not ready
 */
bool True_NB_bc95::get_modem_status()
{
  int test = 0;

  MODEM_SERIAL->println(F("AT"));
  delay(300);
  if ( find_rx_bc(500, "OK") >= 0) {
    return true;
  }
  return false;

}

/****
 * create_udp_socket() : to create UDP socket for connect to platform
 * return: -1: Error, else: success - defualt socket number 0
 ****/
int True_NB_bc95::create_udp_socket(int listen_port, int sock_num /* defulat is 0 */)
{
  String at_cmd;
  String res_str;
  int sock = -1;
  int tmp = -1;

  tmp = this->close_udp_socket(sock_num); // close exiting session

  at_cmd = "AT+NSOCR=DGRAM,17," + String(listen_port) + ",1\r\n";
  MODEM_SERIAL->print(at_cmd);
  delay(300);
  res_str = expect_rx_str(300, "\r\n", 2);  
  if ( res_str == "" ) {
    return -1;
  }
  else {
    sock = res_str.toInt();
    return sock;
  }

}


int True_NB_bc95::close_udp_socket(int sock_numb /*defualt socket number is 0 */)
{
  String at_cmd;
  String res_str;

  at_cmd = "AT+NSOCL=" + String(sock_numb) + "\r\n";
  MODEM_SERIAL->print(at_cmd);
  delay(300);

  res_str = expect_rx_str(300, "OK", 2);
  if ( res_str == "") {
    return -1;
  }
  //debug_println("closed exiting socket id:" + String(sock_numb));
  return sock_numb;

}

/****
 * send_upd_data() : to send UDP data to platform
 * require: create_udp_socket() first.
 ****/
bool True_NB_bc95::send_upd_data(int socknum, String remote_ip, int remote_port, int data_len, String *data)
{
  String at_cmd;
  char buff[data_len + 2];

  char *h_buf;
  char fetch[3] = "";
  bool chk = false;
  int i = 0;
  //int data_len = data->length();

  data->toCharArray(buff, data_len + 1);

  at_cmd = "AT+NSOST=" + String(socknum) + "," + remote_ip + "," + String(remote_port) + "," + String(data_len) + ",";
  MODEM_SERIAL->print(at_cmd);

  /* Fetch print data in hex format */
  h_buf = buff;
  while (*h_buf)
  {
    chk = itoa( (int) * h_buf, fetch, 16 );
    if (chk) {
      MODEM_SERIAL->print(fetch);
    }
    h_buf++;
  }
  MODEM_SERIAL->print("\r\n");

}


/************************
 *
 *  Class Platform
 *
 ************************/

 /*** IoTtweet ***/
void IoTtweetNBIoT::init(String serv_ip, int serv_port)
{
  this->server_ip = serv_ip;
  this->server_port = serv_port;
}
 
String IoTtweetNBIoT::write_dashboard_IoTtweet(String userid, String key, float slot0, float slot1, float slot2, float slot3, String tw, String twpb, int sock_num)
{
  String _packet = "";
  int data_len;

  //nb_bc95->debug_println("--- Send to Cloud.IoTtweet ---");
  _packet = userid;
  _packet += ":";
  _packet += key;
  _packet += ":";
  _packet += String(slot0);
  _packet += ":";
  _packet += String(slot1);
  _packet += ":";
  _packet += String(slot2);
  _packet += ":";
  _packet += String(slot3);
  _packet += ":";
  _packet += tw;
  _packet += ":";
  _packet += twpb;

  nb_bc95->debug_print("iottweet: " + String(_packet));
  nb_bc95->debug_print("\r\n");

  data_len = _packet.length();
  nb_bc95->send_upd_data(sock_num, this->server_ip, this->server_port, data_len, &_packet); // defualt sock num is 0

  return "OK";

}


/*** ubidots platform ***/
void ubidotsNBIoT::init(String serv_ip, int serv_port)
{
  this->server_ip = serv_ip;
  this->server_port = serv_port;
}

String ubidotsNBIoT::write_dashboard_ubidotsNBIoT(String useragent, String token, String device_label, String data_val, int sock_num)
{
  String _packet = "";
  int data_len;
  
  /* UDP format: {useragent}|POST|{token}|{device_label}=>{value1:xx1},{value2:xx2}|end */  
  _packet = useragent;
  _packet += "|POST|";
  _packet += token;
  _packet += "|";
  _packet += device_label;
  _packet += "=>";
  _packet += data_val;
  _packet += "|end";
  
  nb_bc95->debug_print("ubitdos: " + String(data_val));
  nb_bc95->debug_print("\r\n");

  data_len = _packet.length();
  nb_bc95->send_upd_data(sock_num, this->server_ip, this->server_port, data_len, &_packet); // defualt sock num is 0

  return "OK";

}
