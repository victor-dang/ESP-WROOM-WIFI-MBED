// ESP-WROOM-02 Static page WEB server to control Mbed
 #include "mbed.h"

 
Serial pc(USBTX, USBRX);
Serial wroom(p28, p27); // tx, rx
 

 
// Standard Mbed LED definitions
DigitalOut  led1(LED1);      
DigitalOut  led2(LED2);    
DigitalOut  led3(LED3);     
 

DigitalOut  reset(p26);
 
Timer t1;
Timer t2;
////////

char ssid[32] = "ssid";     // enter WiFi router ssid inside the quotes
char pwd [32] = "password"; // enter WiFi router password inside the quotes
char str[255];
void WROOMconfig();
///////
struct tm t;
 
int bufflen, DataRX, counter, getcount, replycount, servreq, timeout;
int bufl, ipdLen, linkID, weberror;

char temp[10];
//char webcount[8];
char type[16];
char type1[16];
char channel[2];
char cmdbuff[32];
char replybuff[1024];
char webdata[1024]; // This may need to be bigger depending on WEB browser used
char webbuff[4096];     // Currently using 1986 characters, Increase this if more web page data added
 
void SendCMD(),getreply(),ReadWebData(),startserver(),sendpage(),SendWEB(),sendcheck();

 

 
int port        =80;  // set server port
int SERVtimeout =5;    // set server timeout in seconds in case link breaks.
 
// Serial Interrupt read ESP data
void callback()
{
    //led3=1;
    while (wroom.readable()) {
        webbuff[counter] = wroom.getc();
        counter++;
    }
    if(strlen(webbuff)>bufflen) {
        DataRX=1;
        //led3=0;
    }
}
 
int main()
{
    reset=0;
    pc.baud(115200);
    pc.printf("\f\n\r------------ ESP-WROOM-02 Hardware Reset --------------\n\r");
    wait(0.5);
    reset=1;
    led1=0,led2=0,led3=0;
    timeout=6000;
    getcount=6000;
    getreply();
    wroom.baud(115200);   // ESP-WROOM-02 baudrate. Maximum on KLxx' is 115200, 230400 works on K20 and K22F
    WROOMconfig(); 
    
    
    startserver();
 
    while(1) {
        if(DataRX==1) {
            ReadWebData();
            
            if (servreq == 1 && weberror == 0) {
                sendpage();
            }
            wroom.attach(&callback);
            pc.printf(" IPD Data:\r\n\n Link ID = %d,\r\n IPD Header Length = %d \r\n IPD Type = %s\r\n", linkID, ipdLen, type);
            pc.printf("\n\n  HTTP Packet: \n\n%s\n", webdata);
            pc.printf("  Web Characters sent : %d\n\n", bufl);
            pc.printf("  -------------------------------------\n\n");
            //strcpy(lasthit, timebuf);
            servreq=0;
        }
    }
}
// Static WEB page
void sendpage()
{
 
// WEB page data
    strcpy(webbuff, "<!DOCTYPE html>");
    strcat(webbuff, "<html><head><title>IISEC - Matsui Lab</title><meta charset=\"UTF-8\"></head>");
    strcat(webbuff, "<body>");
    //strcat(webbuff, "<div style=\"text-align:center; background-color:#F4F4F4; color:#00AEDB;\"><h1>ESP-WROOM-02 and LPC7168 Web Server</h1>");
    strcat(webbuff, "<div style=\"text-align:center; background-color:#F4F4F4; color:#00AEDB;\"><h1>情報セキュリティ大学院 - 松井研 - 2019</h1>");
    strcat(webbuff, "<br><h1>ESP-WROOM-02 and LPC7168 Web Server</h1>");
    strcat(webbuff, "</div><br /><hr>");

    strcat(webbuff, "<form method=\"POST\"> <strong> &nbsp&nbsp");
    if(led1==0) {
        strcat(webbuff, "<p><input type=\"radio\" name=\"led1\" value=\"0\" checked>  LED 1 off");
        strcat(webbuff, "<br><input type=\"radio\" name=\"led1\" value=\"1\" >  LED 1 on");
    } else {
        strcat(webbuff, "<p><input type=\"radio\" name=\"led1\" value=\"0\" >  LED 1 off");
        strcat(webbuff, "<br><input type=\"radio\" name=\"led1\" value=\"1\" checked>  LED 1 on");
    }
    if(led2==0) {
        strcat(webbuff, "<p><input type=\"radio\" name=\"led2\" value=\"0\" checked>  LED 2 off");
        strcat(webbuff, "<br><input type=\"radio\" name=\"led2\" value=\"1\" >  LED 2 on");
    } else {
        strcat(webbuff, "<p><input type=\"radio\" name=\"led2\" value=\"0\" >  LED 2 off");
        strcat(webbuff, "<br><input type=\"radio\" name=\"led2\" value=\"1\" checked>  LED 2 on");
    }
    if(led3==0) {
        strcat(webbuff, "<p><input type=\"radio\" name=\"led3\" value=\"0\" checked>  LED 3 off");
        strcat(webbuff, "<br><input type=\"radio\" name=\"led3\" value=\"1\" >  LED 3 on");
    } else {
        strcat(webbuff, "<p><input type=\"radio\" name=\"led3\" value=\"0\" >  LED 3 off");
        strcat(webbuff, "<br><input type=\"radio\" name=\"led3\" value=\"1\" checked>  LED 3 on");
    }

    strcat(webbuff, "</strong><p><input type=\"submit\" value=\"send\" style=\"background: #3498db;");
    strcat(webbuff, "background-image:-webkit-linear-gradient(top, #3498db, #2980b9);");
    strcat(webbuff, "background-image:linear-gradient(to bottom, #3498db, #2980b9);");
    strcat(webbuff, "-webkit-border-radius:12;border-radius: 12px;font-family: Arial;color:#ffffff;font-size:20px;padding:");
    strcat(webbuff, "10px 20px 10px 20px; border:solid #103c57 3px;text-decoration: none;");
    strcat(webbuff, "background: #3cb0fd;");
    strcat(webbuff, "background-image:-webkit-linear-gradient(top,#3cb0fd,#1a5f8a);");
    strcat(webbuff, "background-image:linear-gradient(to bottom,#3cb0fd,#1a5f8a);");
    strcat(webbuff, "text-decoration:none;\"></form></span>");
    strcat(webbuff, "</body></html>");
// end of WEB page data
    bufl = strlen(webbuff); // get total page buffer length
    sprintf(cmdbuff,"AT+CIPSEND=%d,%d\r\n", linkID, bufl); // send IPD link channel and buffer character length and store all into cmdbuff
    timeout=200;
    getcount=7;
    SendCMD();
    getreply();
    SendWEB();  // send web page
    memset(webbuff, '\0', sizeof(webbuff));
    sendcheck();
}
 
//  wait for ESP-WROOM "SEND OK" reply, then close IP to load web page
void sendcheck()
{
    weberror=1;
    timeout=500;
    getcount=24;
    t2.reset();
    t2.start();
    while(weberror==1 && t2.read() <5) {
        getreply();
        if (strstr(replybuff, "SEND OK") != NULL) {
            weberror=0;   // wait for valid SEND OK
        }
    }
    if(weberror==1) { // restart connection
        strcpy(cmdbuff, "AT+CIPMUX=1\r\n");
        timeout=500;
        getcount=10;
        SendCMD();
        getreply();
        pc.printf(replybuff);
        sprintf(cmdbuff,"AT+CIPSERVER=1,%d\r\n", port);
        timeout=500;
        getcount=10;
        SendCMD();
        getreply();
        pc.printf(replybuff);
    } else {
        sprintf(cmdbuff, "AT+CIPCLOSE=%s\r\n",channel); // close current connection
        SendCMD();
        getreply();
        pc.printf(replybuff);
    }
    t2.reset();
}
 
// Reads and processes GET and POST web data
void ReadWebData()
{
    wait_ms(200);
    wroom.attach(NULL);
    counter=0;
    DataRX=0;
    weberror=0;
    memset(webdata, '\0', sizeof(webdata));
    int x = strcspn (webbuff,"+");
    if(x) {
        strcpy(webdata, webbuff + x);
        weberror=0;
        //int numMatched = sscanf(webdata,"+IPD,%d,%d:%s", &linkID, &ipdLen, type); // read linkID, ipdLen, type from webdata
        sscanf(webdata,"+IPD,%d,%d:%s", &linkID, &ipdLen, type); // read linkID, ipdLen, type from webdata
        if( strstr(webdata, "led1=1") != NULL ) {
            led1=1;
        }
        if( strstr(webdata, "led1=0") != NULL ) {
            led1=0;
        }
        if( strstr(webdata, "led2=1") != NULL ) {
            led2=1;
        }
        if( strstr(webdata, "led2=0") != NULL ) {
            led2=0;
        }
        if( strstr(webdata, "led3=1") != NULL ) {
            led3=1;
        }
        if( strstr(webdata, "led3=0") != NULL ) {
            led3=0;
        }
        
        sprintf(channel, "%d",linkID);
        if (strstr(webdata, "GET") != NULL) {
            servreq=1;
        }
        if (strstr(webdata, "POST") != NULL) {
            servreq=1;
        }
        //webcounter++;
        //sprintf(webcount, "%d",webcounter);
    } else {
        memset(webbuff, '\0', sizeof(webbuff));
        wroom.attach(&callback);
        weberror=1;
    }
}
// Starts and restarts webserver if errors detected.
void startserver()
{

    pc.printf("++++++++++ Resetting ESP-WROOM-02 ++++++++++\r\n");
    strcpy(cmdbuff,"AT+RST\r\n");
    timeout=8000;
    getcount=1000;
    SendCMD();
    getreply();
    pc.printf(replybuff);
    pc.printf("%d",counter);
    if (strstr(replybuff, "OK") != NULL) {
        pc.printf("\n++++++++++ Starting Server ++++++++++\r\n");
        strcpy(cmdbuff, "AT+CIPMUX=1\r\n");  // set multiple connections.
        timeout=500;
        getcount=20;
        SendCMD();
        getreply();
        pc.printf(replybuff);
        sprintf(cmdbuff,"AT+CIPSERVER=1,%d\r\n", port);
        timeout=500;
        getcount=20;
        SendCMD();
        getreply();
        pc.printf(replybuff);
        wait(1);
        sprintf(cmdbuff,"AT+CIPSTO=%d\r\n",SERVtimeout);
        timeout=500;
        getcount=50;
        SendCMD();
        getreply();
        pc.printf(replybuff);
        wait(5);
        pc.printf("\n Getting Server IP \r\n");
        strcpy(cmdbuff, "AT+CIFSR\r\n");
        timeout=2500;
        getcount=200;
        while(weberror==0) {
            SendCMD();
            getreply();
            if (strstr(replybuff, "0.0.0.0") == NULL) {
                weberror=1;   // wait for valid IP
            }
        }
        pc.printf("\n Enter WEB address (IP) found below in your browser \r\n\n");
        pc.printf("\n The MAC address is also shown below,if it is needed \r\n\n");
        replybuff[strlen(replybuff)-1] = '\0';
        sprintf(webdata,"%s", replybuff);
        pc.printf(webdata);
        led2=0;
        bufflen=200;
        counter=0;
        pc.printf("\n\n++++++++++ Ready ++++++++++\r\n\n");
        wroom.attach(&callback);
    } else {
        pc.printf("\n++++++++++ ESP-WROOM-02 error, check power/connections ++++++++++\r\n");
        while(1) {}
    }
    t2.reset();
    t2.start();
   
}
// ESP-WROOM-02 Command data send
void SendCMD()
{
    wroom.printf("%s", cmdbuff);
}
// Large WEB buffer data send
void SendWEB()
{
    int i=0;
    if(wroom.writeable()) {
        while(webbuff[i]!='\0') {
            wroom.putc(webbuff[i]);
            i++;
        }
    }
}
// Get Command and ESP-WROOM-02 status replies
void getreply()
{
    memset(replybuff, '\0', sizeof(replybuff));
    t1.reset();
    t1.start();
    replycount=0;
    while(t1.read_ms()< timeout && replycount < getcount) {
        if(wroom.readable()) {
            replybuff[replycount] = wroom.getc();
            replycount++;
        }
    }
    t1.stop();
}

void WROOMconfig()
{
    wait(5);
    strcpy(cmdbuff,"AT\r\n");
    SendCMD();
    wait(1);
    strcpy(cmdbuff,"AT\r\n");
    SendCMD();
    wait(1);
    strcpy(cmdbuff,"AT\r\n");
    SendCMD();
    timeout=1;
    getreply();
    wait(1);
    pc.printf("\f---------- Starting ESP-WROOM-02 Config ----------\r\n\n");
 

    wait(3);
 
    // set CWMODE to 1=Station,2=AP,3=BOTH, default mode 1 (Station)
    pc.printf("\n---------- Setting Mode ----------\r\n");
    strcpy(cmdbuff, "AT+CWMODE=1\r\n");
    SendCMD();
    timeout=100;
    getreply();
    pc.printf(replybuff);
 
    wait(2);
 
    // set CIPMUX to 0=Single,1=Multi
    pc.printf("\n---------- Setting Connection Mode ----------\r\n");
    strcpy(cmdbuff, "AT+CIPMUX=1\r\n");
    SendCMD();
    timeout=400;
    getreply();
    pc.printf(replybuff);
 
    wait(2);
 
    pc.printf("\n---------- Listing Access Points ----------\r\n");
    strcpy(cmdbuff, "AT+CWLAP\r\n");
    SendCMD();
    timeout=9000;
    getcount=5000;
    getreply();
    pc.printf(replybuff);
 
    wait(2);
 
    pc.printf("\n---------- Connecting to AP ----------\r\n");
    pc.printf("ssid = %s   pwd = %s\r\n",ssid,pwd);
    strcpy(cmdbuff, "AT+CWJAP=\"");
    strcat(cmdbuff, ssid);
    strcat(cmdbuff, "\",\"");
    strcat(cmdbuff, pwd);
    strcat(cmdbuff, "\"\r\n");
    SendCMD();
    timeout=9000;
    getcount=5000;
    getreply();
    pc.printf(replybuff);
 
    wait(5);
 
    pc.printf("\n---------- Get IP's ----------\r\n");
    strcpy(cmdbuff, "AT+CIFSR\r\n");
    SendCMD();
    timeout=400;
    getreply();
    pc.printf(replybuff);
 
    wait(1);
 
    pc.printf("\n---------- Get Connection Status ----------\r\n");
    strcpy(cmdbuff, "AT+CIPSTATUS\r\n");
    SendCMD();
    timeout=500;
    getreply();
    pc.printf(replybuff);
    pc.printf("ESP-WROOM saves the SSID and password settings internally\r\n");
    wait(10);
}
