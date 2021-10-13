#!/bin/bash

cd ../misc/

# download vespa
wget "https://drive.google.com/uc?id=1hfs3smqD-cpFEOMj1lbT3ASkW6cF0KNu" --no-check-certificate -O vespa.zip
unzip vespa.zip

#download pica_pica
wget "https://drive.google.com/uc?id=1TkRQrpNmAjy_Vcp1AQNSTORc_52MEVO4" --no-check-certificate -O pica_pica.zip
unzip pica_pica.zip

#download bathroom
wget "https://drive.google.com/uc?id=1e89qGb0nNrujmei50H_U8XfTq4Rie817" --no-check-certificate -O bathroom.zip
unzip bathroom.zip

# delete .zip
rm -f *.zip
