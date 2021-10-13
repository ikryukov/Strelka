#!/bin/bash

cd ../misc/

# download vespa
wget --content-disposition "https://drive.google.com/uc?id=1hfs3smqD-cpFEOMj1lbT3ASkW6cF0KNu"
unzip vespa.zip

#download pica_pica
wget --content-disposition "https://drive.google.com/uc?id=1TkRQrpNmAjy_Vcp1AQNSTORc_52MEVO4"
unzip pica_pica.zip

#download bathroom
wget --content-disposition "https://drive.google.com/uc?id=1e89qGb0nNrujmei50H_U8XfTq4Rie817"
unzip bathroom.zip

# delete .zip
rm -f *.zip
