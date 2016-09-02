# CanSat by SatNOGS Mumbai and Workshop Bangalore

This repo contains an arduino program which was deployed
on the first satellite made by SatNOGS Mumbai team in
co-ordination with Workshop Bangalore.

## CanSat-Transreceiver

It is a single program which can transmit and receive data 
over nrf24 module (nrF24L01 was used in satellite).

It also reads temperature, altitude and pressure from
BMP180 module.

`seaLevelPressure` variable is configured to use pressure
of Bangalore City as of 1203 AM Sept 3, 2016.

Codes are written and maintained by Ashish Gaikwad <ash.gkwd@gmail.com>
based on [BMP180](http://embedded-lab.com/blog/bmp180/) and 
[nrF24L01](https://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo)
