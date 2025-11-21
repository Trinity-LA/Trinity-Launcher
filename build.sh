#! /bin/bash
qmake trinito.pro 
make
rm Makefile
qmake trinchete.pro 
make 
echo "LISTO YA SE COMPILO CON EXITO"

