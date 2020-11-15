#include "SpotifyClient.h"

//url will be <name>.local
SpotifyClient::SpotifyClient(String name){
	if (MDNS.begin(name.c_str())) {
		Serial.println("MDNS responder started");
	}
	//http://name.local/authentication/
	callbackLink = new String("http%3A%2F%2F" + String(name) + ".local%2Fauthentication%2F");
	apiClient = new ApiClient(&sClient);
}

//step one
//get code to access token and a refresh token.
//https://developer.spotify.com/documentation/general/guides/authorization-guide/
//code will be send to URI in callbackLink.
//todo: handel non autentification
String SpotifyClient::startAuthentication(){
	Serial.println("\n\nstartAuthentication");
	Serial.println("start Server");
	String requestCode = "";

	//open in Browser to get a logintpromt
	this->server.on ( "/", [this]() {
		//redirect quere to spotify
		server.sendHeader("Location", String("https://accounts.spotify.com/authorize/?client_id=" 
									+ String(CLIENTID)
									+ "&response_type=code&redirect_uri=" 
									+ *this->callbackLink 
									+ "&scope=user-read-private%20"
									+ "user-read-currently-playing%20"
									+ "user-read-playback-state%20"
									+ "user-modify-playback-state"),
						true);
		server.send ( 302, "text/plain", "");
	});

	//catch the code created by the login
	this->server.on ( "/authentication/", [this, &requestCode](){
		//check if request was sucessfull.
		if(!this->server.hasArg("code")) {
			server.send(500, "text/plain", "BAD ARGS");
			return;
		}

		requestCode = this->server.arg("code");
		//Serial.printf("Code: %s\n", requestCode.c_str());

		String message = "<html><head></head><body>Succesfully authentiated This device with Spotify</body></html>";

		this->server.send ( 200, "text/html", message );
	} );

	this->server.begin();
	Serial.println ( "HTTP server started" );

	while(requestCode == "") {
		server.handleClient();
		yield();
	}
	
	Serial.println ( "stop server" );

	this->server.stop();
	return requestCode;
}

//step two
//get authenication und refreshtoken
//todo:errorhandling
void SpotifyClient::getAccessAndRefreshToken(String code){
	Serial.println("\n\ngetAccessAndRefreshToken");
	//https://accounts.spotify.com/api/token
	const char* host = "accounts.spotify.com";
	const int port = 443;

	//connect to spotify
	if (!this->apiClient->connect(host, port)) {
		Serial.println("connection failed");
		return;
	}

	//generate autentification
	String authorizationRaw = String(CLIENTID) + ":" + String(CLIENTSECRET);
	String authorization = base64::encode(authorizationRaw);
	this->apiClient->setAuthentication( "Basic " + authorization);
	
	String url = "/api/token";
	//create body content
	String content = "grant_type=authorization_code&code=" + code 
					+ "&redirect_uri=" + *this->callbackLink;


	//query
	this->apiClient->POST(url.c_str(), content);
	DynamicJsonDocument doc = *this->apiClient->getData();

	const char* accessToken = doc["access_token"];
	const char* expires_in = doc["expires_in"];
	const char* refresh_token = doc["refresh_token"];
	
	this->refresh_token = new String(refresh_token);
	this->accessTokenExpiresTime = String(expires_in).toInt()*1000+millis();
	this->accessToken = new String(accessToken);

	this->apiClient->setAuthentication( "Bearer " + *this->accessToken);

}

String* SpotifyClient::getRefreshToken(){
	return this->refresh_token;
}

void SpotifyClient::setRefreshToken(String refreshToken){
	this->refresh_token = new String(refreshToken);
}

//todo:errorhandling
void SpotifyClient::checkAccessToken(){
	if(millis()>this->accessTokenExpiresTime){
		this->renewAccessToken();
	}
}

void SpotifyClient::renewAccessToken(){
	Serial.println("\n\nrenewAccessToken");
	//https://accounts.spotify.com/api/token
	const char* host = "accounts.spotify.com";
	const int port = 443;

	//connect to spotify
	if (!this->apiClient->connect(host, port)) {
		Serial.println("connection failed");
		return;
	}

	//generate autentification
	String authorizationRaw = String(CLIENTID) + ":" + String(CLIENTSECRET);
	String authorization = base64::encode(authorizationRaw);
	this->apiClient->setAuthentication( "Basic " + authorization);
	
	String url = "/api/token";
	//create body content
	String content = "grant_type=refresh_token&refresh_token=" + *this->refresh_token;


	//query
	this->apiClient->POST(url.c_str(), content);
	DynamicJsonDocument doc = *this->apiClient->getData();

	const char* accessToken = doc["access_token"];
	const char* expires_in = doc["expires_in"];
	
	this->accessTokenExpiresTime = String(expires_in).toInt()*1000+millis();
	this->accessToken = new String(accessToken);

	this->apiClient->setAuthentication("Bearer " + *this->accessToken);
}


int SpotifyClient::getPlayerInfo(){

	Serial.println("\n\ngetPlayerInfo");
	this->apiClient->GET("/v1/me/player");
	return 0;
}

void SpotifyClient::startControl(){
	Serial.println("\n\nstartControal");
	this->apiClient->disconnet();
	if (!this->apiClient->connect( "api.spotify.com", 443)) {
		Serial.println("connection failed");
		return;
	}
	Serial.println(this->apiClient->isConnected());
}

SpotifyClient::~SpotifyClient(){
	Serial.println("end");
	delete this->apiClient;
	delete this->callbackLink;
	if(this->refresh_token){
		delete refresh_token;
	}
	if(this->accessToken){
		delete accessToken;
	}
} 

