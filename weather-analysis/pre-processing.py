import pandas as pd

CSV_FILE = "weather-endurance.csv"
OUTPUT_FILE = "weather-clean.csv"

# Load InfluxDB annotated CSV
df = pd.read_csv(
    CSV_FILE,
    comment="#"
)

# Remove any unnamed columns
df = df.loc[:, ~df.columns.str.startswith("Unnamed")]

# Show which fields exist in the export
print("\n--- FIELDS FOUND ---")
print(df["_field"].value_counts())

# Parse timestamps
df["_time"] = pd.to_datetime(
    df["_time"],
    format="ISO8601",
    errors="coerce",
    utc=True
)

# Remove repeated header rows / invalid timestamp rows
df = df.dropna(subset=["_time"])

# Convert values to numeric
df["_value"] = pd.to_numeric(
    df["_value"],
    errors="coerce"
)

# Remove invalid values
df = df.dropna(subset=["_value"])

# Reshape from long format:
# time | field | value
#
# into wide format:
# time | temperature | humidity | pressure | etc.
weather = (
    df.pivot_table(
        index="_time",
        columns="_field",
        values="_value",
        aggfunc="first"
    )
    .sort_index()
)

# Remove pandas' column-axis label
weather.columns.name = None

print("\n--- COLUMNS ---")
print(weather.columns.tolist())

print("\n--- DATA TYPES ---")
print(weather.dtypes)

print("\n--- SHAPE ---")
print(weather.shape)

# Save cleaned dataset
weather.to_csv(OUTPUT_FILE)

print(f"\nSaved cleaned dataset to: {OUTPUT_FILE}")
