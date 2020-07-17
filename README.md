# cuentaPersonas-AutoContenido-Joaquin-

## Ejemplos
### InfluxDB
```
InfluxData measurement ("Personas");
  measurement.addTag("device", DISPOSITIVO); // por defecto "d1"
  measurement.addTag("sensor", SENSOR_OBSTRUCCION); // por defecto "reflectivo"
  measurement.addTag("accion", "entrar"); // entrar o salir
  measurement.addValue("value", 1); //Puede ser 1, 0 o -1 segun que paso. -1 es que salio alguien y 0 es error detectado.
```

En terminos de base de datos, hay una tabla "Personas" con un campo "value" con los posibles valores 1, 0 y -1. 

El resto de etiquetas son para filtrar si hay mas de una placa/sensor
