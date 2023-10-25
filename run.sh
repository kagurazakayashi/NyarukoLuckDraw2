# sudo apt install gcc libcurl4-gnutls-dev -y
rm -f ./draw
gcc -o draw draw.c -lcurl
chmod +x ./draw
./draw --about --url "https://blockchain.info/q/latesthash"