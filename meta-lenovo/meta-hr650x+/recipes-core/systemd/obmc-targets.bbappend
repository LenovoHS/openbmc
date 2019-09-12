# Remove these files since they are not needed on hr650x+ 
SYSTEMD_SERVICE_${PN}_remove += " obmc-power-reset-on@.target"
SYSTEMD_SERVICE_${PN}_remove += " obmc-host-reset@.target"
SYSTEMD_SERVICE_${PN}_remove += " obmc-chassis-powerreset@.target"
