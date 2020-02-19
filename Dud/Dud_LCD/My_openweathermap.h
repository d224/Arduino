

// The type of data that we want to extract from the page
struct openweathermap_Data {
  double temp;
  unsigned long sunrise_sec;
  unsigned long sunset_sec;
  String  weather_text;
  String icon;
};

extern openweathermap_Data weatherData;
void update_weather() ;
String getMeteoconIcon(String icon);