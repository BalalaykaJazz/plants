Скетч для ESP8266. Опрашиваются датчики DHT11 (Температура и влажность; D4) и датчик влажности почвы (A0).

На каждое показание с датчиков формируется свой json следующего формата:

{"timestamp": 2021-02-13T14:14:12Z, "value": 123}

Затем данные отправляются в mqtt брокер в соответствующие топики
