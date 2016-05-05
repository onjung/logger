logger
======================

# 1. What is this?
**[GA](http://gameanalytics.com)** is a very powerful analytics tool specialized for both mobile and pc games. GA, however, is currently more focused on iOS & Android games, and not putting much effort on PC games. The API they provide for PC games only suports VS2012+, while there are still so many PC games, especially MMO RPG games, that are maintained by using VS2010 or older. This logger API has been built based on the existing API in hopes of suporting older versions of VS family, and fully tested on VS2005 and VS2008.

****
# 2. How to use?
* Fill the following info in the constructor
```
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
```
