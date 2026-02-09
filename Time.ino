// time.ino: sends emails
// could also use:
//    struct tm dt;
//    time_t ts = (time_t) result.timestamp;
//    localtime_r(&ts, &dt);

#include <time.h>

unsigned long getEpochTime() {    // seconds since 1/1/1970 12am
	unsigned long epochTime;

  time((time_t *) &epochTime);
	return epochTime;
}

// human readable timestamp. example: Sun Mar 21, 10:30pm
String getTimestamp() {
  struct tm info;
  char buf[32] = {0};       // null terminated empty string
  const char* weekDays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  if (getLocalTime(&info)) {
    int hour = info.tm_hour;
    const char* ampm = (hour >= 12)? "pm" : "am";
    if (hour == 0)      // 12am
      hour = 12;
    else if (hour > 12)
      hour -= 12;

    sprintf(buf, "%s %s %d, %d:%02d%s", weekDays[info.tm_wday], months[info.tm_mon], info.tm_mday, hour, info.tm_min, ampm);
  }

  return String(buf);
}

// number of minutes since midnight, used for disabling opening door at night
int getTimeInMins() {
  struct tm info;
  if (getLocalTime(&info))
    return info.tm_hour*60 + info.tm_min;
  return -1;
}