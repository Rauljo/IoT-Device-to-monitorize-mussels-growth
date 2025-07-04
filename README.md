# IoT-Device-to-monitorize-mussels-growth
This repository contains all the files that were necessary to create an IoT device capable of supervising and monitoring the growth of mussels in a pan, based on the sinking distance of the pan.

## Código LoraWAN

Se puede acceder al código utilizado en la placa de desarrollo de Heltec en el archivo codigo_sin_licencias.ino (se han eliminado las referencias a las claves de uso del dispositivo dentro de TTN por seguridad) en la carpeta final_code_without_license

## Procesamiento de datos

En los archivos node-red-flow.json y grafana.json se puede acceder al flujo creado en Node-RED y al dashboard creado en Grafana respectivamente. El sistema complejo utiliza MQTT desde TTN, se envían los datos desde Node-RED a influxdb y estos se leen para visualizarlos en grafana. 

![nodered](https://github.com/user-attachments/assets/6f0918a0-5dae-4ef5-a575-c72a3967741b)
![Cuadro de mando](https://github.com/user-attachments/assets/5cefb23b-d9eb-43d8-8d49-08b42276b445)


## Modelo 3D de la caja

El archivo "3D Model.skp" contiene el modelo 3D generado ah hoc para el sistema:


![3d cerrado](https://github.com/user-attachments/assets/840f5161-bb32-43cd-abab-4ae7a2babfd8)
![3d_por_partes](https://github.com/user-attachments/assets/af7e0a68-9c74-49de-b1e5-28ce628e061b)
![3dseccion](https://github.com/user-attachments/assets/3cf6e199-afca-49e2-9471-e60ee57151e2)

## Consumos sistema

El archivo Energy_Consumption.ppk2 contiene los datos de consumos leídos por el Power Profiler Kit II de Nordic.

![consumo esp32](https://github.com/user-attachments/assets/23a9b2a3-c2f1-4c24-a410-ddff9f65edc7)

## Acceso a los datos de influxdb

El código de python Obtain_data_from_influx_in_pytho.py es un ejemplo de acceso a los datos de influxdb para mostrar un histograma de los datos de distancia capturados en los últimos 2 días. 

