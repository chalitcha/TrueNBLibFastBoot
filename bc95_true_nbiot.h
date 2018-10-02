/********************
 * BC95 interface lib
 *********************/

#ifndef _BC95_TRUE_NBIOT_H_
#define _BC95_TRUE_NBIOT_H_

#include <Stream.h>
#include <Arduino.h>

#define BUF_MAX_SIZE 64 // default 64
#define MODEM_RESP 128 // default 128

#define _OPEN_DEBEG_MODE_ false

class True_NB_bc95 {
  private:
    Stream* MODEM_SERIAL;
    Stream* DEBUG_SERIAL;
    bool debug_flag;
    
    String expect_rx_str( unsigned long period, char exp_str[], int len_check);    

    /** Private Board init **/
    void init_serial(Stream &serial);  

    /** Private modem network init **/
    bool set_auto_connect();
    bool set_phon_function();

  public:
    //True_NB_bc95();
    True_NB_bc95(Stream &serial);

    /** Board init **/      
    bool init_modem();
    bool reboot_modem();

    /** Get board info **/
    String  get_imsi();
    String  get_imei();
    String  get_ip_address();
    int     get_modem_signal();
    bool    get_modem_status();

    /** Network connection **/
    bool register_network();    
    int create_udp_socket(int port, int sock_num = 0);
    int close_udp_socket(int sock_numb = 0 /*defualt socket number is 0 */);
    bool send_upd_data(int socknum, String remote_ip, int remote_port, int data_len, String *data);

    /** AT Command direct **/
    int bc95_send_AT_cmd(String at_cmd, char *res_str);
    int bc95_response(char *res_str);

    /** Common **/
    void debug_print(String str_out);
    void debug_println(String str_out);
    void pc_debug_enable(Stream &serial, bool flag1);
    int find_rx_bc(long tout,String str_wait);  
    
    // Test    
    bool register_network2();    
    bool get_network_status();
    void clear_buff();
};


/*********************
 * Example: Platform 
 *********************/
class IoTtweetNBIoT 
{
  private:
      True_NB_bc95 *nb_bc95;
  public:      
      IoTtweetNBIoT(True_NB_bc95 *nb);
      
      String server_ip = "35.185.177.33";
      int server_port = 5683;

      IoTtweetNBIoT(Stream &serial);
      void init(String serv_ip, int serv_port);
      String write_dashboard_IoTtweet(String userid, String key, float slot0, float slot1, float slot2, float slot3, String tw, String twpb, int sock_num=0);
};

class ubidotsNBIoT 
{
  private:
      True_NB_bc95 *nb_bc95;
  public:      
      ubidotsNBIoT(True_NB_bc95 *nb);
      
      String server_ip = "50.23.124.66"; /* translate.ubidots.com */
      int server_port = 9012;
      
      ubidotsNBIoT(Stream &serial);
      void init(String serv_ip, int serv_port);
      String write_dashboard_ubidotsNBIoT(String useragent, String token, String device_label, String data_val, int sock_num=0);
};

#endif _BC95_TRUE_NBIOT_H_
