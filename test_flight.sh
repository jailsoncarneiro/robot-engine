#!/bin/bash
echo "=== INICIANDO TESTE DE VOO ==="

send_cmd() {
    echo "$1" | nc -u -w0 127.0.0.1 9002
    echo ">> Comando enviado: $1"
}

# 1. Decola e espera 4 segundos para estabilizar no ar
send_cmd "TAKEOFF"
sleep 4

# 2. Faz o Backflip com o drone já pairando calmo
send_cmd "BACKFLIP"
sleep 4

# 3. Mata o drone no ar (Kill Switch)
# send_cmd "KILL"
# sleep 3

# 4. Tenta o Turtle Mode no chão para desvirar
send_cmd "TURTLE"
sleep 3

# 5. Manda decolar de novo para ver se o Turtle funcionou!
send_cmd "TAKEOFF"
sleep 3

send_cmd "LAND"
echo "=== TESTE FINALIZADO ==="