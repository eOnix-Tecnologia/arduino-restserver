#include "RestServer.h"



RestServer::RestServer(EthernetServer& server): server_(server), routesIndex_(0), bufferIndex_(0) {}

String RestServer::getParam(char * text){
	int begin = String(text).indexOf('=') + 1;
	return String(text).substring(begin);
}

bool RestServer::connect(){
	client_ = server_.available();
	if(client_) return true;
	else return false;
}


String RestServer::getBody(){
  	String json = "";
	long timeout = millis();
  	while (millis() - timeout < 300) {
		if (available()){
			timeout = millis();
			json += read();
		}		
	}
 
	if (json.charAt(0) == '\'' && json.charAt(json.length()-1) == '\'') {
		json = json.substring(1, json.length()-1);
	}
	return json;
}


int RestServer::getBody(char* buffer, int length){
	int count = 0;
	long timeout = millis();
	while (count < length && millis() - timeout < 300) {
		if (available()){
			timeout = millis();
			buffer[count] = read();
			count++;
		}
	}
	return count;
}


String RestServer::getBodyArray(){
  String json = "";
  long time = millis();
  while(millis() - time < 1000) {
    if (available()) {
      time = millis();
      char character = read();
      json += character;
      if (character == ']') break;
    }
  }
  if (json.charAt(0) == '\'') {
    json = json.substring(1);
  }
  return json;
}

void RestServer::stop(){
  client_.stop();
}

char RestServer::read(){
  return client_.read();
}

int RestServer::available(){
  return client_.available();
}

void RestServer::run() {
  if (server_.available()){
     client_ = server_.available();
    if (client_) {
      JSON_START();
      // Check the received request and process it
      check();
      bufferIndex_--;
      JSON_CLOSE();

      // Send data for the client
      send(8, 0);

      // Stop the client connection
      client_.stop();

      // Necessary resets
      reset();
    }
  }
}

void RestServer::loop() {
  if (server_.available()) {
    client_ = server_.available();
    if (client_) {
      check();
    }
  }  
}

void RestServer::reset() {
  // Reset buffer
  memset(&buffer_[0], 0, sizeof(buffer_));

  // Reset buffer index
  bufferIndex_ = 0;
}

void RestServer::addRoute(char * method, const char * route, void (*f)(char * params) ) {
  // memcpy(routes_[routesIndex_].name, route, strlen(route)+1);
  routes_[routesIndex_].method   = method;
  routes_[routesIndex_].name     = route;
  routes_[routesIndex_].callback = f;
  
  // DLOG( "Route added:" );
  // DLOG( routes_[routesIndex_].name );
  routesIndex_++;
}

void RestServer::addToBuffer(char * value) {
  for (int i = 0; i < strlen(value); i++){
    buffer_[bufferIndex_+i] = value[i];  
  }
  bufferIndex_ = bufferIndex_ + strlen(value);
}

void RestServer::addData(const char* name, char * value) {
  char bufferAux[5120] = {0};
  uint16_t idx = 0;

  // Format the data as:
  // "name":"value",
  bufferAux[idx++] = '"';
  for (int i = 0; i < strlen(name); i++){
    bufferAux[idx++] = name[i];
  }
  bufferAux[idx++] = '"';

  bufferAux[idx++] = ':';
  bufferAux[idx++] = '"';
  for (int i = 0; i < strlen(value); i++){
    bufferAux[idx++] = value[i];  
  }
  bufferAux[idx++] = '"';
  bufferAux[idx++] = ',';

  addToBuffer(bufferAux);
}

// Add to output buffer_
void RestServer::addData(const char* name, String& value){

  char bufferAux[5120] = {0};
  uint16_t idx = 0;

  bufferAux[idx++] = '"';
  for (int i = 0; i < strlen(name); i++){
    bufferAux[idx++] = name[i];
  }
  bufferAux[idx++] = '"';

  bufferAux[idx++] = ':';
  bufferAux[idx++] = '"';
  for (int i = 0; i < value.length(); i++){
    bufferAux[idx++] = value.charAt(i);  
  }
  bufferAux[idx++] = '"';
  bufferAux[idx++] = ',';

  addToBuffer(bufferAux);

}


// Add to output buffer_
void RestServer::addData(const char* name, uint16_t value){
  char number[10];
  itoa(value,number,10);
  
  addData(name, number);
}

// Add to output buffer_
void RestServer::addData(const char* name, int value){
  char number[10];
  itoa(value,number,10);
  
  addData(name, number);
}

// Add to output buffer_ (Mega & ESP only)
void RestServer::addData(const char* name, float value){
  char number[10];
  // dtostrf(value, 5, 2, number);  
  char fmt[20];
  sprintf(fmt, "%%%d.%df", 5, 2);
  sprintf(number, fmt, value);  

  addData(name, number);
}

// Send the HTTP response for the client
void RestServer::send(uint16_t chunkSize, uint16_t delayTime) {
  // First, send the HTTP Common Header
  client_.println(HTTP_COMMON_HEADER);

  // Send all of it
  if (chunkSize == 0)
    client_.print(buffer_);

  // Send chunk by chunk #####################################

  // Max iterations
  uint16_t max = (int)(bufferIndex_/chunkSize) + 1;

  // Send data
  for (uint16_t i = 0; i < max; i++) {
    char bufferAux[chunkSize+1];
    memcpy(bufferAux, buffer_ + i*chunkSize, chunkSize);
    bufferAux[chunkSize] = '\0';

    // DLOGChar(bufferAux);
    client_.print(bufferAux);

    // Wait for client_ to get data
    delay(delayTime);
  }
}


void RestServer::sendRaw(String mensagem){
	// Serial.println("[sendRaw] "+mensagem);
	for (int i=0; i<mensagem.length(); i+=1024){
		client_.print(mensagem.substring(i, i+1024));
	}	
	// client_.flush();
}

void RestServer::sendBuffer(const char* mensagem, uint16_t lenght){
	client_.write(mensagem, lenght);
}

// Extract information about the HTTP Header
void RestServer::check() {
  char route[ROUTES_LENGHT] = {0};
  bool routePrepare = false;
  bool routeCatchFinished = false;
  uint16_t r = 0;

  char query[QUERY_LENGTH] = {0};
  bool queryPrepare = false;
  bool queryCatchFinished = false;
  uint16_t q = 0;
  
  char method[METHODS_LENGTH] = {0};
  bool methodCatchFinished = false;
  uint16_t m = 0;

  bool currentLineIsBlank = true;
  char c;
  while ( client_.connected() && client_.available() ) {
    c = client_.read();

    // DLOGChar(c);

    // Start end of line process /
    // if you've gotten to the end of the line (received a newline
    // character) and the line is blank, the http request header has ended,
    // so you can send a reply or check the body of the http header
    if (c == '\n' && currentLineIsBlank) {
      // Here is where the parameters of other HTTP Methods will be.
      break;
    }

    if (c == '\n')
      currentLineIsBlank = true; // you're starting a new line
    else if (c != '\r')
      currentLineIsBlank = false; // you've gotten a character on the current line
    // End end of line process ///

    // Start route catch process /
    if(c == '/' && !routePrepare)
      routePrepare = true;

    if((c == ' ' || c == '?') && routePrepare)
      routeCatchFinished = true;

    if(routePrepare && !routeCatchFinished)
      route[r++] = c;
    // End route catch process 

    // Start query catch process 
    if(c == ' ' && queryPrepare)
      queryCatchFinished = true;

    if(queryPrepare && !queryCatchFinished)
      query[q++] = c;

    if(c == '?' && !queryPrepare)
      queryPrepare = true;
    // End query catch process 

    // Start method catch process 
    if(c == ' ' && !methodCatchFinished)
      methodCatchFinished = true;

    if(!methodCatchFinished)
      method[m++] = c;
    // End method catch process 

  }

  for(int i = 0; i < routesIndex_; i++) {
      // Check if the routes names matches
      // if(strncmp( route, routes_[i].name, sizeof(routes_[i].name) ) != 0)
      if (String(route) != String(routes_[i].name))
        continue;

      // Check if the HTTP METHOD matters for this route
      // if(strncmp( routes_[i].method, "*", sizeof(routes_[i].method) ) != 0) {
      if (String(routes_[i].method) != "*") { 
        // If it matters, check if the methods matches
        // if(strncmp( method, routes_[i].method, sizeof(routes_[i].method) ) != 0)
        if (String(method) != String(routes_[i].method))
          continue;
      }

      // Route callback (function)
      routes_[i].callback(query);
  }
}