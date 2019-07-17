# Remove these files since they are provided by obmc-intel-targets
SYSTEMD_SERVICE_${PN}_remove += " obmc-power-reset-on@.target"
SYSTEMD_SERVICE_${PN}_remove += " obmc-host-reset@.target"
SYSTEMD_SERVICE_${PN}_remove += " obmc-chassis-powerreset@.target"

