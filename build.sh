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
sudo cp -rf trinito /usr/local/bin/
sudo cp -rf trinchete /usr/local/bin/
sudo cp -rf resources/com.trench.trinity.launcher.svg /usr/share/icons/
sudo cp -rf resources/com.trench.trinity.launcher.desktop /usr/share/applications/
echo "trinity instalado"

