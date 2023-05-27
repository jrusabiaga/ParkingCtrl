# ParkingCtrl
Sistema de control de aparcamiento utilizando un ESP32, un sensor láser ToF y un módulo GSM-GPRS puede detectar el estado de ocupación de una plaza de aparcamiento cada minuto. Si se produce un cambio en el estado, se activará un MOSFET que encenderá el módulo GPRS. Una vez activo y conectado a la red, enviará un SMS a un número preestablecido para notificar el estado de la plaza de aparcamiento. Luego, el módulo GPRS se desactivará hasta el próximo cambio en el estado de la plaza.

Este sistema permite monitorear de forma periódica el estado de ocupación de la plaza de aparcamiento y enviar notificaciones en tiempo real solo cuando se produce un cambio, lo que ayuda a optimizar el uso del módulo GPRS y minimizar el consumo de energía.
