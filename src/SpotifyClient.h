#include "secrets.h"
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include "ApiClient.h"
#include "base64.h"
#include <ArduinoJson.h>
#include <EEPROM.h>


class SpotifyClient{
  private:
  	WebServer server;
	String* callbackLink;
	WiFiClientSecure sClient;
	ApiClient* apiClient;
	String* refresh_token;
	String* accessToken;
	int expires_in;
    
  public:
  	SpotifyClient(const char*);
	String startAuthentication();
	void getAccessAndRefreshToken(String);
	String* getRefreshToken();
	void setRefreshToken(String);


	~SpotifyClient();
};
