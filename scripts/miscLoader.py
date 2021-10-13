import gdown
import zipfile
import os

def unZip(filename):
    with zipfile.ZipFile(filename, "r") as zip_ref:
        zip_ref.extractall(projectDir + "misc/")


def download(url, output):
    gdown.download(url, output, quiet=False)
    unZip(output)
    os.remove(output)


projectDir = os.pardir + "/"
try:
    os.mkdir(projectDir + "misc")
except:
    pass

# download vespa
# https://drive.google.com/file/d/1hfs3smqD-cpFEOMj1lbT3ASkW6cF0KNu/view?usp=sharing
url = 'https://drive.google.com/uc?id=1hfs3smqD-cpFEOMj1lbT3ASkW6cF0KNu'
output = projectDir + "misc/vespa.zip"
download(url, output)

# download pica-pica
# https://drive.google.com/file/d/1TkRQrpNmAjy_Vcp1AQNSTORc_52MEVO4/view?usp=sharing
url = 'https://drive.google.com/uc?id=1TkRQrpNmAjy_Vcp1AQNSTORc_52MEVO4'
output = projectDir + "misc/pica-pica.zip"
download(url, output)

# download bathroom
# https://drive.google.com/file/d/1e89qGb0nNrujmei50H_U8XfTq4Rie817/view?usp=sharing
url = 'https://drive.google.com/uc?id=1e89qGb0nNrujmei50H_U8XfTq4Rie817'
output = projectDir + "misc/bathroom.zip"
download(url, output)
