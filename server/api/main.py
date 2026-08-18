import os

from fastapi import FastAPI
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
from pydantic import BaseModel

app = FastAPI()

influx_url = os.environ["INFLUX_URL"]
influx_token = os.environ["INFLUX_TOKEN"]
influx_org = os.environ["INFLUX_ORG"]
influx_bucket = os.environ["INFLUX_BUCKET"]


class weatherReading(BaseModel):
	temperature: float
	humidity: float
	pressure: float
	rssi: int
	battery_voltage: float
	battery_percent: float

import os

from fastapi import FastAPI
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
from pydantic import BaseModel

app = FastAPI()

influx_url = os.environ["INFLUX_URL"]
influx_token = os.environ["INFLUX_TOKEN"]
influx_org = os.environ["INFLUX_ORG"]
influx_bucket = os.environ["INFLUX_BUCKET"]


class WeatherReading(BaseModel):
    temperature: float
    humidity: float
    pressure: float
    rssi: int
    battery_voltage: float
    battery_percent: float


@app.get("/health")
def health():
    return {"status": "ok"}


@app.post("/weather")
def receive_weather(reading: WeatherReading):
    point = (
        Point("weather")
        .field("temperature", reading.temperature)
        .field("humidity", reading.humidity)
        .field("pressure", reading.pressure)
        .field("rssi", reading.rssi)
        .field("battery_voltage", reading.battery_voltage)
        .field("battery_percent", reading.battery_percent)
    )

    with InfluxDBClient(
        url=influx_url,
        token=influx_token,
        org=influx_org,
    ) as client:
        write_api = client.write_api(write_options=SYNCHRONOUS)
        write_api.write(bucket=influx_bucket, org=influx_org, record=point)

    return {"status": "ok"}
