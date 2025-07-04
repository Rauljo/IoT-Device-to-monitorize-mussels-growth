from influxdb import InfluxDBClient
import pandas as pd
import matplotlib.pyplot as plt

# Parámetros de conexión a InfluxDB
host = "192.168.68.53"
port = 8086
database = "ttn_data"

# Crear cliente
client = InfluxDBClient(host=host, port=port, database=database)

# Consulta InfluxQL (ajusta el nombre de medición y campo)
query = "SELECT distance_cm FROM distance_readings WHERE time > now() - 48h"

# Ejecutar consulta y obtener resultados
result = client.query(query)

# Convertir a DataFrame
points = list(result.get_points())
df = pd.DataFrame(points)

# Mostrar histograma de los valores
plt.figure(figsize=(8, 5))
plt.hist(df["distance_cm"], bins=20, color="skyblue", edgecolor="black")
plt.title("Histograma de distancias")
plt.xlabel("Distancia (cm)")
plt.ylabel("Frecuencia")
plt.grid(True)
plt.show()
