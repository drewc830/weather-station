import pandas as pd
import matplotlib.pyplot as plt

CSV_FILE = "weather-analysis.csv"

# --------------------------------------------------
# LOAD ANALYZED DATA
# --------------------------------------------------

weather = pd.read_csv(
    CSV_FILE,
    parse_dates=["_time"],
    index_col="_time"
)

weather = weather.sort_index()

# --------------------------------------------------
# TEMPERATURE + DEW POINT
# --------------------------------------------------

plt.figure(figsize=(14, 6))

plt.plot(
    weather.index,
    weather["temperature_f"],
    label="Temperature"
)

plt.plot(
    weather.index,
    weather["dewpoint_f"],
    label="Dew Point"
)

plt.title("Temperature and Dew Point — Full Dataset")
plt.xlabel("Time")
plt.ylabel("Temperature (°F)")
plt.legend()
plt.grid(True)
plt.tight_layout()

plt.savefig("temperature_dewpoint.png", dpi=150)
plt.close()

# --------------------------------------------------
# PRESSURE
# --------------------------------------------------

plt.figure(figsize=(14, 6))

plt.plot(
    weather.index,
    weather["pressure_hpa"]
)

plt.title("Atmospheric Pressure — Full Dataset")
plt.xlabel("Time")
plt.ylabel("Pressure (hPa)")
plt.grid(True)
plt.tight_layout()

plt.savefig("pressure.png", dpi=150)
plt.close()

# --------------------------------------------------
# HUMIDITY
# --------------------------------------------------

plt.figure(figsize=(14, 6))

plt.plot(
    weather.index,
    weather["humidity"]
)

plt.title("Relative Humidity — Full Dataset")
plt.xlabel("Time")
plt.ylabel("Relative Humidity (%)")
plt.grid(True)
plt.tight_layout()

plt.savefig("humidity.png", dpi=150)
plt.close()

# --------------------------------------------------
# BATTERY VOLTAGE
# --------------------------------------------------

plt.figure(figsize=(14, 6))

plt.plot(
    weather.index,
    weather["battery_voltage"]
)

plt.title("Battery Voltage — Full Dataset")
plt.xlabel("Time")
plt.ylabel("Battery Voltage (V)")
plt.grid(True)
plt.tight_layout()

plt.savefig("battery_voltage.png", dpi=150)
plt.close()

# --------------------------------------------------
# 3-HOUR PRESSURE CHANGE
# --------------------------------------------------

plt.figure(figsize=(14, 6))

plt.plot(
    weather.index,
    weather["pressure_change_3h"]
)

plt.axhline(0, linewidth=1)

plt.title("3-Hour Pressure Change — Full Dataset")
plt.xlabel("Time")
plt.ylabel("Pressure Change (hPa / 3 h)")
plt.grid(True)
plt.tight_layout()

plt.savefig("pressure_change_3h.png", dpi=150)
plt.close()

# --------------------------------------------------
# 3-HOUR DEW POINT CHANGE
# --------------------------------------------------

plt.figure(figsize=(14, 6))

plt.plot(
    weather.index,
    weather["dewpoint_change_3h"]
)

plt.axhline(0, linewidth=1)

plt.title("3-Hour Dew Point Change — Full Dataset")
plt.xlabel("Time")
plt.ylabel("Dew Point Change (°F / 3 h)")
plt.grid(True)
plt.tight_layout()

plt.savefig("dewpoint_change_3h.png", dpi=150)
plt.close()

print("\nSaved plots:")
print("  temperature_dewpoint.png")
print("  pressure.png")
print("  humidity.png")
print("  battery_voltage.png")
print("  pressure_change_3h.png")
print("  dewpoint_change_3h.png")
