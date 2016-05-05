#include "Logger.h"

#include <sstream>
#include <algorithm>
#include <winsock2.h>
#include <IPHlpApi.h>
#include <rpcdce.h>

#include "curl\curl.h"
#include "rapidjson\document.h"
#include "hash\MD5.h"
#include "hash\sha1.h"

// Sets an event ID.
void LogEvent::SetEventID(std::string eventID)
{
	eventID_ = eventID;
	useEventID_ = true;
}

// Sets an event area in a string.
void LogEvent::SetArea(std::string area)
{
	area_ = area;
	useArea_ = true;
}

// Sets an event location in a Point struct.
void LogEvent::SetLocation(Point location)
{
	location_ = location;
	useLocation_ = true;
}

// Sets an event value.
void LogEvent::SetValue(float value)
{
	value_ = value;
	useValue_ = true;
}

// Gets a string concatenation of the event.
std::string LogEvent::GetString(std::string userID, std::string sessionID, std::string build)
{
	std::stringstream ss;
	ss << "{";
	ss << "\"user_id\": ";
	ss << "\"" << userID << "\"";
	ss << ", ";
	ss << "\"session_id\": ";
	ss << "\"" << sessionID << "\"";
	ss << ", ";
	ss << "\"build\": ";
	ss << "\"" << build << "\"";
	if (useArea_)
	{
		ss << ", ";
		ss << "\"area\": ";
		ss << "\"" << area_ << "\"";
	}
	if (useEventID_)
	{
		ss << ", ";
		ss << "\"event_id\": ";
		ss << "\"" << eventID_ << "\"";
	}
	if (useValue_)
	{
		ss << ", ";
		ss << "\"value\": ";
		ss << value_;
	}
	if (useLocation_)
	{
		ss << ", ";
		ss << "\"x\": ";
		ss << location_.x;
		ss << ", ";
		ss << "\"y\": ";
		ss << location_.y;
		ss << ", ";
		ss << "\"z\": ";
		ss << location_.z;
	}
	ss << "}";
	return ss.str();
}

// The main class for logging events.
Logger::Logger(void)
{
	curl_global_init(CURL_GLOBAL_ALL);

	///////////////////////////////////////////////////////////////////////////
	// ACCOUNT INFO: CHANGE THE VALUES BELOW UPON YOUR NEEDS.
	///////////////////////////////////////////////////////////////////////////
	// The public game key.
	gameKey_ = "2e2571526ab66e75a5973d2216705e9b";
	// The API key.
	apiKey_ = "ke78ca970d90e37ca4365fceefbbcd3f81dzm903";
	// The build version.
	build_ = "1.0a";
	// The user ID.
	SetUserID("userid");
	// The session id.
	SetSessionID("session1");
	///////////////////////////////////////////////////////////////////////////

    // The API version.
	apiVersion_ = "1";
	// A concatenation of all the events.
	gaEvents_ = "[";
}

// Sets a user ID by hashing the MAC address. (No parameter presents)
void Logger::SetUserID(void)
{
	// Get info of network adapters. (up to 16)
	IP_ADAPTER_INFO info[16];
	DWORD dwBufLen = sizeof(info);
	DWORD dwStatus = GetAdaptersInfo(info, &dwBufLen);

	PIP_ADAPTER_INFO pAdapterInfo = info;
	while (pAdapterInfo && pAdapterInfo == 0)
	{
		pAdapterInfo = pAdapterInfo->Next;
	}
	unsigned char hash[20];
	char hexstring[41];
	
    // Get a hash of the MAC address using SHA1.
    sha1::calc(pAdapterInfo->Address, pAdapterInfo->AddressLength, hash);
	sha1::toHexString(hash, hexstring);
	userID_ = hexstring;

	// Get rid of hyphens.
	userID_.erase(std::remove(userID_.begin(), userID_.end(), '-'), userID_.end());
}

// Sets a user ID.
void Logger::SetUserID(std::string userID)
{
	userID_ = userID;
}

// Sets a session ID with a GUID. (No parameter presents)
void Logger::SetSessionID(void)
{
	GUID guid;
	CoCreateGuid(&guid);

	// Turn the GUID into a string.
	RPC_CSTR str;
	UuidToStringA((UUID*)&guid, &str);
	sessionID_ = (LPSTR)str;

	// Get rid of hyphens.
	sessionID_.erase(std::remove(sessionID_.begin(), sessionID_.end(), '-'), sessionID_.end());
}

// Sets a session ID.
void Logger::SetSessionID(std::string sessionID)
{
	sessionID_ = sessionID;
}

// Adds an event to the events string.
void Logger::AddLogEvent(LogEvent ev)
{
	char rbg = gaEvents_[gaEvents_.size() - 1];

	// If the events string is not empty.
	if (rbg != '[')
	{
		// Appends the events string.
		gaEvents_ += ", ";
	}

	// Add the event to the events string.
	gaEvents_ += ev.GetString(userID_, sessionID_, build_);
}

// Submits the events string.
void Logger::SubmitLogEvents(void)
{
	CURL * curl = curl_easy_init();
	if (curl)
	{
		// Close the events string bracket.
		gaEvents_ += "]";

		// The event category.
		std::string category = "default";

		// The URL to send the events to.
		std::string url = "http://api.gameanalytics.com/" + apiVersion_ + "/" + gameKey_ + "/" + category;

		// Your secret key obtained from the website.
		std::string secretKey = "da23a845c0938712fc403b4a285c766cdc388c702";

		// Hash the header.
		std::string header = gaEvents_ + secretKey;
		std::string auth = "Authorization:" + md5(header);

		// Add the header to a struct.
		struct curl_slist *chunk = NULL;
		chunk = curl_slist_append(chunk, auth.c_str());

		// Set the URL of the request.
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		// Set the string of events.
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, gaEvents_.c_str());

		// Set the header.
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

		// Set request mode to POST.
		curl_easy_setopt(curl, CURLOPT_POST, 1);

		// Send the request.
		CURLcode res = curl_easy_perform(curl);

		// Check the response.
		if (res != CURLE_OK)
		{
			std::cout << "ERROR: Curl did not work. " << curl_easy_strerror(res) << std::endl;
		}

		// Cleanup the object.
		curl_easy_cleanup(curl);
		curl_slist_free_all(chunk);
	}

	// Open a new events string bracket.
	gaEvents_ = "[";
}

// Gets a heatmap for the area.
void Logger::LoadHeatmap(std::string area, std::string eventID)
{
	CURL * curl = curl_easy_init();
	if (curl)
	{
		// Request URL for a heatmap.
		std::string url = "http://data-api.gameanalytics.com/heatmap";

		// Specify the area and the event ID.
		// Inquire multiple events by using "|". 
		//   e.g. "Death:Enemy|Death:Fall".
		std::string requestInfo = "game_key=" + gameKey_ + "&event_ids=" + eventID + "&area=" + area;

		// Concatenate the request with the URL.
		url += "/?";
		url += requestInfo;

		// Hash the header.
		std::string header = requestInfo + apiKey_;
		std::string auth = "Authorization:" + md5(header);

		// Add the header to a struct.
		struct curl_slist *chunk = NULL;
		chunk = curl_slist_append(chunk, auth.c_str());

		// Set the URL of the request.
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		// Set the header.
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		
		// Set the callback.
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DataToString);

		std::string result;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

		// Send the request.
		CURLcode res = curl_easy_perform(curl);

		// Check the response.
		if (res != CURLE_OK)
		{
			std::cout << "ERROR: Curl did not work." << curl_easy_strerror(res) << std::endl;
		}

		// Parse the received JSON data.
		rapidjson::Document doc;
		doc.Parse<0>(result.c_str());
		rapidjson::Value & x = doc["x"];
		rapidjson::Value & y = doc["y"];
		rapidjson::Value & z = doc["z"];
		rapidjson::Value & value = doc["value"];

		std::deque<std::pair<Point, int>> points;

		// Add x, y, and z in a Point struct and value in an int.
		for (unsigned i = 0; i < x.Size(); ++i)
		{
			points.push_back(std::pair<Point, int>(Point(x[i].GetDouble(), y[i].GetDouble(), z[i].GetDouble()), value[i].GetInt()));
		}

		// Insert the heatmap.
		heatmaps_.insert(std::pair<std::string, std::deque<std::pair<Point, int>>>(area, points));

		// Cleanup the object.
		curl_easy_cleanup(curl);
		curl_slist_free_all(chunk);
	}
}

// Gets a heatmap from the area if exists; returns false otherwise.
bool Logger::GetHeatmap(std::string area, std::deque<std::pair<Point, int>>& points)
{
	std::map<std::string, std::deque<std::pair<Point, int>>>::iterator it = heatmaps_.find(area);
	if (it == heatmaps_.end())
		return false;

	points = *it;
	return true;
}

// Callback to take returned data string.
int Logger::DataToString(char *data, size_t size, size_t nmemb, std::string * result)
{
	int count = 0;
	if (result)
	{
		result->append(data, size * nmemb);
		count = size * nmemb;
	}
	return count;
}
