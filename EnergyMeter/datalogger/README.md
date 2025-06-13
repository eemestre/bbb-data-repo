1 - Ambiente virtual

python3.7 -m venv .env
source .venv/bin/activate
python -m pip install -r requirements.txt


2 - Execução do script

source .venv/bin/activate

Para iniciar o servidor:
```sh
python datalogger.py 192.168.15.82 3333 cmeter_params.txt
```

Conversão do arquivo binário para criação da base de 60Hz. Deve ser indicada a data da primeira amostra registrada no arquivo outputs/channel_raw.dat. Exemplo:

```sh
 python convert_raw.py cmeter_params_raw.txt "/home/fernando/Documents/EnergyMeter/NFS raw.bin" "2020-06-28 19:54:53"
```
