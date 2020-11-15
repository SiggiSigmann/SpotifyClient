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
	int accessTokenExpiresTime;

	void checkAccessToken();
	
    
  public:
  	SpotifyClient(String);
	String startAuthentication();
	void getAccessAndRefreshToken(String);
	String* getRefreshToken();
	void setRefreshToken(String);
	void renewAccessToken();
	int getPlayerInfo();
	void startControl();


	~SpotifyClient();
};
