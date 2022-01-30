# ws2812-clock

## fetures

* Display Time `HH:mm:ss` (24h format)
* Display custom text

## setup

1. Connect to the `ws2812-clock` wifi with the password `pR0j3K+10n+v`
2. open a browser with and put this in the url `192.168.4.1`
3. configure
4. have fun!

## reopen settings

1. start pressing `BOOT` button in start animation
2. stop pressing button when `config` is apearing
3. you can now folow the steps from **setup** again

## OTA

1. go into `settings.hpp` set `ENABLE_OTA` to `1`
2. go into `platformio.ini` and uncomment `upload_*` lines and paste in your clocks ip address (the ip is printed to Serial output)

## MQTT

### Setup

1. go into setup
2. go into `parameters`
3. configure
4. click on save
5. restart clock

### Commands

`<command>[command]...`

#### m
set custom text

`m<text lenght> <text>`

#### t
set transition id

`t<id>`

#### r
set ring effect for transition

`r<id>`

#### c
set color effect for transition

`c<id>`

#### i
set inner effect for transition

`i<id>`

#### e
start transition

`e`
