#! /bin/bash
rm Makefile
rm -rf build
qmake6 trinito.pro
make
mv ./bin/trinito .
rm Makefile
qmake6 trinchete.pro
make 
mv ./bin/trinchete .
echo "LISTO YA SE COMPILO CON EXITO o eso creemos"

