import pandas as pd
import numpy as np

CSV_FILE = "weather-clean.csv"

# --------------------------------------------------
# LOAD CLEANED DATA
# --------------------------------------------------

weather = pd.read_csv(
    CSV_FILE,
    parse_dates=["_time"],
    index_col="_time"
)

weather = weather.sort_index()

print("\n--- COLUMNS ---")
print(weather.columns.tolist())

print("\n--- DATA TYPES ---")
print(weather.dtypes)

# --------------------------------------------------
# DEW POINT
# --------------------------------------------------

# Convert Fahrenheit to Celsius
temp_c = (weather["temperature_f"] - 32.0) * 5.0 / 9.0

# Magnus approximation constants
a = 17.625
b = 243.04

gamma = (
    np.log(weather["humidity"] / 100.0)
    + (a * temp_c) / (b + temp_c)
)

dewpoint_c = (b * gamma) / (a - gamma)

# Convert dew point back to Fahrenheit
weather["dewpoint_f"] = dewpoint_c * 9.0 / 5.0 + 32.0

# --------------------------------------------------
# REGULARIZE TO 1-MINUTE GRID
# --------------------------------------------------

# Your original timestamps drift by a few seconds, so place the
# measurements onto exact 1-minute intervals.
weather_1min = weather.resample("1min").mean()

# Fill only small gaps.
weather_1min = weather_1min.interpolate(
    method="time",
    limit=5
)

# --------------------------------------------------
# TIME-BASED CHANGES
# --------------------------------------------------

# Because the dataset is now exactly one row per minute:
# 60 rows  = 1 hour
# 180 rows = 3 hours

weather_1min["temp_change_1h"] = (
    weather_1min["temperature_f"]
    - weather_1min["temperature_f"].shift(60)
)

weather_1min["temp_change_3h"] = (
    weather_1min["temperature_f"]
    - weather_1min["temperature_f"].shift(180)
)

weather_1min["pressure_change_1h"] = (
    weather_1min["pressure_hpa"]
    - weather_1min["pressure_hpa"].shift(60)
)

weather_1min["pressure_change_3h"] = (
    weather_1min["pressure_hpa"]
    - weather_1min["pressure_hpa"].shift(180)
)

weather_1min["humidity_change_1h"] = (
    weather_1min["humidity"]
    - weather_1min["humidity"].shift(60)
)

weather_1min["humidity_change_3h"] = (
    weather_1min["humidity"]
    - weather_1min["humidity"].shift(180)
)

weather_1min["dewpoint_change_1h"] = (
    weather_1min["dewpoint_f"]
    - weather_1min["dewpoint_f"].shift(60)
)

weather_1min["dewpoint_change_3h"] = (
    weather_1min["dewpoint_f"]
    - weather_1min["dewpoint_f"].shift(180)
)

# --------------------------------------------------
# DAILY SUMMARY
# --------------------------------------------------

daily = weather_1min.resample("1D").agg({
    "temperature_f": ["min", "max", "mean"],
    "humidity": ["min", "max", "mean"],
    "pressure_hpa": ["min", "max", "mean"],
    "dewpoint_f": ["min", "max", "mean"],
    "battery_voltage": ["min", "max", "mean"]
})

print("\n--- DAILY SUMMARY ---")
print(daily)

# --------------------------------------------------
# EXTREMES / INTERESTING CHANGES
# --------------------------------------------------

print("\n--- LARGEST 3-HOUR PRESSURE DROPS ---")
print(
    weather_1min["pressure_change_3h"]
    .dropna()
    .nsmallest(10)
)

print("\n--- LARGEST 3-HOUR PRESSURE RISES ---")
print(
    weather_1min["pressure_change_3h"]
    .dropna()
    .nlargest(10)
)

print("\n--- LARGEST 3-HOUR TEMPERATURE DROPS ---")
print(
    weather_1min["temp_change_3h"]
    .dropna()
    .nsmallest(10)
)

print("\n--- LARGEST 3-HOUR TEMPERATURE RISES ---")
print(
    weather_1min["temp_change_3h"]
    .dropna()
    .nlargest(10)
)

print("\n--- LARGEST 3-HOUR HUMIDITY INCREASES ---")
print(
    weather_1min["humidity_change_3h"]
    .dropna()
    .nlargest(10)
)

print("\n--- LARGEST 3-HOUR HUMIDITY DROPS ---")
print(
    weather_1min["humidity_change_3h"]
    .dropna()
    .nsmallest(10)
)

print("\n--- LARGEST 3-HOUR DEW POINT INCREASES ---")
print(
    weather_1min["dewpoint_change_3h"]
    .dropna()
    .nlargest(10)
)

print("\n--- LARGEST 3-HOUR DEW POINT DROPS ---")
print(
    weather_1min["dewpoint_change_3h"]
    .dropna()
    .nsmallest(10)
)

# --------------------------------------------------
# SAVE RESULTS
# --------------------------------------------------

weather_1min.to_csv("weather-analysis.csv")
daily.to_csv("daily-summary.csv")

print("\nSaved:")
print("  weather-analysis.csv")
print("  daily-summary.csv")
