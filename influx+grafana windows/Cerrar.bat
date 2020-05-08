echo "Cerrando servidores..."
Taskkill /IM grafana-server.exe
Taskkill /IM influxd.exe

timeout 5
echo "Forzando salida por si alguno no responde..."
Taskkill /IM grafana-server.exe /F
Taskkill /IM influxd.exe /F
