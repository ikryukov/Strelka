## LAUNCH GUIDE

**Assets storage:** https://bit.ly/2YTJcTV

### OS X

1. Run:


       brew install wget
       cd scripts/
       sh miscLoader.sh

To zip folder *source* without system files run: 

      zip -r source.zip source -x "/__MACOSX" -x "/.*"
                  
### Windows

1. Download wget http://www.gnu.org/software/wget
2. Add to PATH variables: 
    1. path/git/bin (contains sh.exe) 
    2. path/gnuwin32/bin (contains wget.exe)
3. Run:


    cd scripts/
    sh miscLoader.sh
