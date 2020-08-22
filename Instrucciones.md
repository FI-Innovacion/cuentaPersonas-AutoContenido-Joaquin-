# INSTRUCCIONES:

## LINUX: ------------
Configuracion primer uso linux:
1. Entrar a la carpeta "influx+grafana linux" (Preferiblemente por terminal, para asegurarse que hace lo correcto)
2. Ejecutar "EJECUTAR_PRIMERO.sh"

Usar en linux:
1. Entrar a la carpeta "influx+grafana linux"
2. Ejecutar "Iniciar.sh"
    Se iniciarán los servicios de grafana e influx al mismo tiempo.
3. Para cerrar los servicios, ejecutar "Cerrar.sh" (Si solo cierra la ventana, los servicios quedan funcionando de fondo)


## WINDOWS: ----------------
Configuracion primer uso windows:
1. Entrar a la carpeta "influx+grafana windows"
2. Ejecutar "EJECUTAR_PRIMERO.bat"

Usar en linux:
1. Entrar a la carpeta "influx+grafana windows"
2. Ejecutar "Iniciar.bat"
    Se iniciarán los servicios de grafana e influx al mismo tiempo.
3. Para cerrar los servicios, ejecutar "Cerrar.bat" (Si solo cierra la ventana, los servicios quedan funcionando de fondo)


### Credenciales
Datos para el uso de grafana e influx:---------------------------------------------------------

Nombres definidos:

En grafana:
Administrador no configurado, es "admin" con contraseña "admin". Grafana permitirá cambiar la contraseña en el primer ingreso. Configurar usuarios a gusto.

En influx:
Usuario de solo lectura: "usuario" con contraseña "mypass". Tiene permisos de lectura sobre la base de datos "datos"
Existe el administrador por defecto "admin" contraseña "admin" tambien. Puede modificarse desde el CLI de influx (influx o influx.exe)

Influxdb está configurado para usar puerto 8086, por defecto. Grafana busca a influx en la misma máquina, en localhost.8086
Se puede acceder a grafana desde el navegador, ingresando a localhost:3000

Para mayores instrucciones sobre configuracion y administracion, ver la documentacion oficial de grafana y de influxdb

### Algunas ayudas

Dar permisos a los scripts para ejecutar
```
cd influx+grafana linux
chmod +x Iniciar.sh Cerrar.sh EJECUTAR_PRIMERO.sh ./influxdb-1.7.9-1\ linux/* grafana-6.4.4\ linux/bin/*
```

Nota: 
Usa la libreria ESP8266 Influxdb de Tobias Schürg, version 1.3.0, una libreria wifimanager modificada (incluida en este repo), y la version 2.4.1 de la libreria de tarjeta esp8266 por ESP8266 community