#!/bin/bash

./"grafana-6.4.4 linux"/bin/grafana-server --homepath ./"grafana-6.4.4 linux" &
./"influxdb-1.7.9-1 linux"/influxd &

exit 0
