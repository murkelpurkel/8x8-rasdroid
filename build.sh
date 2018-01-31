#!/bin/bash

gcc -W -Wall -o max7219-8x8 fontmap.c max7219-8x8.c -lwiringPi -lpthread
