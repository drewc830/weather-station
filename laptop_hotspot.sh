#!/usr/bin/env bash
set -euo pipefail

INTERFACE="wlp3s0"
HOTSPOT="WeatherStationDemo"
SSID="Nodal-Demo"
PASSWORD="NodalDemo123"
NORMAL_WIFI="UCO_SECURE"

is_running() {
    nmcli -t -f NAME connection show --active |
        grep -Fxq "$HOTSPOT"
}

create_hotspot() {
    echo "Creating hotspot profile..."

    sudo nmcli connection add \
        type wifi \
        ifname "$INTERFACE" \
        con-name "$HOTSPOT" \
        autoconnect no \
        ssid "$SSID"

    sudo nmcli connection modify "$HOTSPOT" \
        802-11-wireless.mode ap \
        802-11-wireless.band bg \
        802-11-wireless.channel 6 \
        802-11-wireless-security.key-mgmt wpa-psk \
        802-11-wireless-security.psk "$PASSWORD" \
        ipv4.method shared \
        ipv6.method disabled
}

start_hotspot() {
    if is_running; then
        echo "Hotspot is already running."
        return
    fi

    if ! nmcli connection show "$HOTSPOT" >/dev/null 2>&1; then
        create_hotspot
    fi

    echo "Starting hotspot..."
    sudo nmcli connection up "$HOTSPOT"

    echo "Hotspot started."
    echo "SSID: $SSID"
    echo "Laptop address: 10.42.0.1"
}

stop_hotspot() {
    if ! is_running; then
        echo "Hotspot is already stopped."
        return
    fi

    echo "Stopping hotspot..."
    sudo nmcli connection down "$HOTSPOT"

    echo "Reconnecting to $NORMAL_WIFI..."
    sudo nmcli connection up "$NORMAL_WIFI"

    echo "Hotspot stopped."
}

case "${1:-}" in
    start)
        start_hotspot
        ;;
    stop)
        stop_hotspot
        ;;
    status)
        if is_running; then
            echo "Hotspot is running."
        else
            echo "Hotspot is stopped."
        fi
        ;;
    *)
        echo "Usage: $0 {start|stop|status}"
        exit 1
        ;;
esac
