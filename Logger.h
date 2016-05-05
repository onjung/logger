#include <string.h>
#include <deque>
#include <map>
#include <utility>

struct Point
{
	Point(void) : x(0.f), y(0.f), z(0.f) {};
	Point(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {};
	float x;
	float y;
	float z;
};

class LogEvent
{
    friend class Logger;
public:
    LogEvent() : useArea_(false), useEventID_(false), useValue_(false), useLocation_(false) {};

    void SetArea(std::string area);
    void SetEventID(std::string eventID);
    void SetValue(float value);
    void SetLocation(Point location);
private:
    std::string GetString(std::string userID, std::string sessionID, std::string build);

    bool useArea_;
    bool useEventID_;
    bool useValue_;
    bool useLocation_;

    std::string area_;
    std::string eventID_;
    float value_;
    Point location_;
};

class Logger
{
public:
    Logger(void);
    void AddLogEvent(LogEvent ev);
    void SubmitLogEvents(void);
    void LoadHeatmap(std::string area, std::string eventID);
    bool GetHeatmap(std::string area, std::deque<std::pair<Point, int>>& points);
	static int DataToString(char *data, size_t size, size_t nmemb, std::string * result);
private:
    void SetUserID(void);
    void SetSessionID(void);
	void SetUserID(std::string userID);
	void SetSessionID(std::string session_id);
    
    std::string apiVersion_;
    std::string gameKey_;
    std::string apiKey_;
    std::string gaEvents_;
    std::string userID_;
    std::string sessionID_;
	std::string build_;

    std::map<std::string, std::deque<std::pair<Point, int>>> heatmaps_;
};

