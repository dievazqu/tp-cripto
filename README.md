# TP Cripto

Para compilar:
1. Ir a la carpeta *tp-cripto*
2. Ejecutar *make*

Para extraer:
```bash
./stegowav -extract -p wav_path -out out_file -steg (LSB1 | LSB4 | LSBE) [-a (aes128 | aes192 | aes256 | des)] [-m (cbc | ofb | cfb | ecb)] [-pass password]
```
Ejemplo
```bash
./stegowav -extract -p files/LosAusentes/clocks7a.wav -steg LSB4 -out output/video -a des -m ofb -pass descubrilo
```

Para embeber:
```bash
./stegowav -embed -p wav_path -in file_to_embed -out wav_out_file -steg (LSB1 | LSB4 | LSBE) [-a (aes128 | aes192 | aes256 | des)] [-m (cbc | ofb | cfb | ecb)] [-pass password]
```
Ejemplo
```bash
./stegowav -embed -in src/stegowav.c -p files/LosAusentes/clocks7a.wav -out output/salida.wav -steg LSB1
```

