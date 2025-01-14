#ifndef restserver_h
#define restserver_h

// Include Arduino header
#include <Arduino.h>
#include <Ethernet.h>


#ifndef ROUTES_TOTAL
#define ROUTES_TOTAL 50
#endif

#ifndef ROUTES_LENGHT
#define ROUTES_LENGHT 32
#endif

#ifndef QUERY_LENGTH
#define QUERY_LENGTH 128
#endif

#ifndef HTTP_COMMON_HEADER
#define HTTP_COMMON_HEADER "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: POST, GET, PUT, OPTIONS\r\nContent-Type: application/json\r\n\n"
#endif

#define HTTP_OK "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n"
#define HTTP_TEXT_TYPE "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: POST, GET, PUT, OPTIONS\r\nContent-Type: text\r\nConnection: close\r\n"
#define HTTP_HTML "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: POST, GET, PUT, OPTIONS\r\nContent-Type: text/html\r\n\n"
#define HTTP_ERROR "HTTP/1.1 400\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: POST, GET, PUT, OPTIONS\r\nContent-Type: text/html\r\nConnection: close\r\n"
#define HTTP_ERROR_BODY "HTTP/1.1 400\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n\n"


#ifndef OUTPUT_BUFFER_SIZE
#define OUTPUT_BUFFER_SIZE 2048
#endif

#define JSON_START() addToBuffer((char *) "{ ");
#define JSON_CLOSE() addToBuffer((char *) " }");


#define METHODS_LENGTH 7
#define ANY (char *) "*"
#define GET (char *) "GET"
#define PUT (char *) "PUT"
#define HEAD (char *) "HEAD"
#define POST (char *) "POST"
#define TRACE (char *) "TRACE"
#define DELETE (char *) "DELETE"
#define OPTIONS (char *) "OPTIONS"

#define MAX_LENGTH 10240

struct Routes {
  char * method;
  const char * name;
  void (*callback)(char * params);
};

class RestServer {
public:
  int buffLen = 0;
  RestServer(EthernetServer& client);
  
  void run();
  void loop();
  String getBody();
  int getBody(char* buffer, int length);
  String getBodyArray();
  
  void addRoute(char * method, const char * route, void (*f)(char *));
  String getParam(char * text);  
  void addData(const char* name, String& value);
  void addData(const char* name, uint16_t value);
  void addData(const char* name, int value);
  void addData(const char* name, float value);
  void addData(const char* name, char* value);
  void sendRaw(String mensagem);
  void sendBuffer(const char* mensagem, uint16_t index);
  void send(uint16_t chunkSize, uint16_t delayTime);
  void reset();
  char read();
  int available();
  bool connect();
  void stop();
  void clear();
  char* getBodyBuffer();
  EthernetClient client_;

private:
  char json[MAX_LENGTH];
  Routes routes_[ROUTES_TOTAL];
  uint16_t routesIndex_;
  char buffer_[OUTPUT_BUFFER_SIZE];
  uint16_t bufferIndex_;  
  
  EthernetServer& server_;
  
  void check();  
  void addToBuffer(char * value);
 
  
  
};

#endif
