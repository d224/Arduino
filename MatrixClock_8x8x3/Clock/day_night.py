import math
from datetime import datetime, date, time as dtime, timedelta, timezone
from config import LATITUDE, LONGITUDE

def _day_of_year(dt_date):
    return dt_date.timetuple().tm_yday

def _calc_sun_event_utc_minutes(dt_date, lat, lon, is_sunrise):
    zenith = 90.833
    n = _day_of_year(dt_date)
    lng_hour = lon / 15.0

    if is_sunrise:
        t = n + ((6 - lng_hour) / 24.0)
    else:
        t = n + ((18 - lng_hour) / 24.0)

    m = (0.9856 * t) - 3.289
    l = m + (1.916 * math.sin(math.radians(m))) + (0.020 * math.sin(math.radians(2 * m))) + 282.634
    l %= 360.0

    ra = math.degrees(math.atan(0.91764 * math.tan(math.radians(l))))
    ra %= 360.0

    l_quadrant = (math.floor(l / 90.0)) * 90.0
    ra_quadrant = (math.floor(ra / 90.0)) * 90.0
    ra = ra + (l_quadrant - ra_quadrant)
    ra_hours = ra / 15.0

    sin_dec = 0.39782 * math.sin(math.radians(l))
    cos_dec = math.cos(math.asin(sin_dec))

    cos_h = (
        math.cos(math.radians(zenith)) - (sin_dec * math.sin(math.radians(lat)))
    ) / (cos_dec * math.cos(math.radians(lat)))

    if cos_h > 1:
        return None, "sun_never_rises"
    if cos_h < -1:
        return None, "sun_never_sets"

    if is_sunrise:
        h = 360.0 - math.degrees(math.acos(cos_h))
    else:
        h = math.degrees(math.acos(cos_h))

    h_hours = h / 15.0
    t_local = h_hours + ra_hours - (0.06571 * t) - 6.622
    ut = (t_local - lng_hour) % 24.0

    return ut * 60.0, None

def compute_sunrise_sunset_local(dt_date, lat, lon, local_tz):
    sunrise_minutes_utc, sunrise_state = _calc_sun_event_utc_minutes(dt_date, lat, lon, True)
    sunset_minutes_utc, sunset_state = _calc_sun_event_utc_minutes(dt_date, lat, lon, False)

    if sunrise_state == "sun_never_rises" and sunset_state == "sun_never_rises":
        return None, None, "night_all_day"

    if sunrise_state == "sun_never_sets" and sunset_state == "sun_never_sets":
        return None, None, "day_all_day"

    sunrise_local = None
    sunset_local = None

    if sunrise_minutes_utc is not None:
        sunrise_utc = datetime.combine(dt_date, dtime.min, tzinfo=timezone.utc) + timedelta(minutes=sunrise_minutes_utc)
        sunrise_local = sunrise_utc.astimezone(local_tz)

    if sunset_minutes_utc is not None:
        sunset_utc = datetime.combine(dt_date, dtime.min, tzinfo=timezone.utc) + timedelta(minutes=sunset_minutes_utc)
        sunset_local = sunset_utc.astimezone(local_tz)

    return sunrise_local, sunset_local, None

def is_daytime(now, lat=LATITUDE, lon=LONGITUDE):
    local_tz = now.tzinfo or datetime.now().astimezone().tzinfo
    sunrise, sunset, polar_state = compute_sunrise_sunset_local(now.date(), lat, lon, local_tz)

    if polar_state == "day_all_day":
        return True
    if polar_state == "night_all_day":
        return False

    if sunrise is None or sunset is None:
        return True

    return sunrise <= now < sunset

def get_command(now, lat=LATITUDE, lon=LONGITUDE):
    suffix = "-D" if is_daytime(now, lat, lon) else "-N"
    return now.strftime("%H:%M") + suffix
