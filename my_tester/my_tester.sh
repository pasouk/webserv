#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
CONFIG_FILE="$SCRIPT_DIR/confs/tester.conf"
LOG_FILE="$SCRIPT_DIR/server.log"

GREEN="\033[0;32m"
RED="\033[0;31m"
NC="\033[0m" # reset color

TOTAL_TESTS=0
PASSED_TESTS=0

# Kill autre webserv si actif
pkill -f "webserv" 2>/dev/null
sleep 0.3

"$PROJECT_DIR/webserv" $CONFIG_FILE > $LOG_FILE 2>&1 &
SERVER_PID=$! #$! recupere pid du dernier process  lance en background

printf "Lancement du testeur de Fbuyl et Magillet...\n"
printf "Config File choisi : %s\n" "$CONFIG_FILE"
printf "Serveur lancé sur le PID : %d\n" "$SERVER_PID"

sleep 1

run_test() {
    local DESCRIPTION="$1"
    local REQ="$2"
    local EXPECT="$3"

    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    # Envoi  requête et capture de la réponse
    RESPONSE=$(printf "$REQ" | nc -w 2 127.0.0.1 8080)

    # On récupère  la première ligne (status)
    STATUS=$(echo "$RESPONSE" | head -n 1 | tr -d '\r')

    if echo "$STATUS" | grep -q "$EXPECT"; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "${GREEN}Test $TOTAL_TESTS OK:${NC} $DESCRIPTION → $STATUS"
    else
        echo -e "${RED}Test $TOTAL_TESTS FAIL:${NC} $DESCRIPTION → $STATUS"
    fi
}

# vérifie code de statut ET un pattern dans le body
# Utile pour les CGI : prouve que le script a bien tourné et revoiwe la bonne sortie
run_test_body() {
    local DESCRIPTION="$1"
    local REQ="$2"
    local EXPECT_STATUS="$3"
    local EXPECT_BODY="$4"

    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    RESPONSE=$(printf "$REQ" | nc -w 2 127.0.0.1 8080)
    STATUS=$(echo "$RESPONSE" | head -n 1 | tr -d '\r')
    # Le body est après la ligne vide qui sépare headers et body
    BODY=$(echo "$RESPONSE" | awk '/^\r?$/{found=1; next} found{print}')

    if echo "$STATUS" | grep -q "$EXPECT_STATUS" && echo "$BODY" | grep -q "$EXPECT_BODY"; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "${GREEN}Test $TOTAL_TESTS OK:${NC} $DESCRIPTION → $STATUS (body contient '$EXPECT_BODY')"
    elif ! echo "$STATUS" | grep -q "$EXPECT_STATUS"; then
        echo -e "${RED}Test $TOTAL_TESTS FAIL:${NC} $DESCRIPTION → status: $STATUS (attendu: $EXPECT_STATUS)"
    else
        echo -e "${RED}Test $TOTAL_TESTS FAIL:${NC} $DESCRIPTION → $STATUS mais body='$BODY' (attendu: '$EXPECT_BODY')"
    fi
}






#TESTS GET

# Accès simple à la racine, cas normal
run_test "GET /" "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "200 OK"

# Route inexistante : doit retourner 404
run_test "GET /nope" "GET /nope HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "404"

# Répertoire sans / final : le serveur doit rediriger (301)
run_test "GET /directory" "GET /directory HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "301"

# Répertoire avec slash : sert le fichier index configuré (youpi.bad_extension via alias YoupiBanane)
run_test "GET /directory/" "GET /directory/ HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "200"

# Fichier specifique dans le répertoire alias
run_test "GET /directory/youpi.bad_extension" "GET /directory/youpi.bad_extension HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "200"

# Fichier inexistant dans le répertoire alias
run_test "GET /directory/oulalala" "GET /directory/oulalala HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "404"

# Sous-répertoire avec slash
run_test "GET /directory/nop/" "GET /directory/nop/ HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "200"

# Répertoire existant mais sans fichier index (Yeah/ ne contient pas youpi.bad_extension)
run_test "GET /directory/Yeah/" "GET /directory/Yeah/ HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "404"

# Méthode HTTP inconnue : le serveur ne doit pas crasher
run_test "FABRICE /" "FABRICE / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "400"






#TESTS POST

# POST sur une route GET/HEAD only : méthode refusée
run_test "POST /" "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhello" "405"

# POST avec body vide sur la route dédiée : doit être accepté (0 < limite 100)
run_test "POST /post_body 0 octets" "POST /post_body HTTP/1.1\r\nHost: localhost\r\nContent-Type: text/plain\r\nContent-Length: 0\r\nConnection: close\r\n\r\n" "200"

# POST exactement à la limite configurée (client_max_body_size 100) : doit passer
run_test "POST /post_body 100 octets (limite exacte)" "POST /post_body HTTP/1.1\r\nHost: localhost\r\nContent-Type: text/plain\r\nContent-Length: 100\r\nConnection: close\r\n\r\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" "20"

# POST un octet au-dessus de la limite : doit être rejeté (413 Payload Too Large)
run_test "POST /post_body 101 octets (depasse limite)" "POST /post_body HTTP/1.1\r\nHost: localhost\r\nContent-Type: text/plain\r\nContent-Length: 101\r\nConnection: close\r\n\r\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" "413"

# POST sur une route GET/HEAD only : la méthode POST est interdite même avec body valide
run_test "POST /directory/" "POST /directory/ HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhello" "405"

#TEST HEAD (PAS DEMANDE DANS LE SUJET)
#run_test "HEAD /" "HEAD / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "200 OK"
#run_test "HEAD /nope" "HEAD /nope HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "404"






#TESTS DELETE

# DELETE sur /directory/ : seuls GET/HEAD sont autorisés sur cette location
run_test "DELETE /directory/youpi.bad_extension" "DELETE /directory/youpi.bad_extension HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "405"

# DELETE sur une route POST-only : DELETE n'est pas autorisé non plus
run_test "DELETE /post_body" "DELETE /post_body HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "405"

#TESTS DELETE (fonctionnel)
# tester.conf autorise POST sur /api/upload et DELETE sur /
# POST cree un fichier dans uploads/ 
# DELETE /nom supprime uploads/nom 

TESTFILE="to_delete.txt"

# Creation du fichier via POST 
run_test "POST /api/upload (cree $TESTFILE)" "POST /api/upload HTTP/1.1\r\nHost: localhost\r\nContent-Disposition: attachment; filename=$TESTFILE\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhello" "20"

# DELETE du fichier cree 
run_test "DELETE /$TESTFILE (fichier existant)" "DELETE /$TESTFILE HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "200"

# Verification sur disque que le fichier a bien ete supprime
TOTAL_TESTS=$((TOTAL_TESTS + 1))
if [ ! -f "$PROJECT_DIR/uploads/$TESTFILE" ]; then
    PASSED_TESTS=$((PASSED_TESTS + 1))
    echo -e "${GREEN}Test $TOTAL_TESTS OK:${NC} uploads/$TESTFILE bien supprime du disque"
else
    echo -e "${RED}Test $TOTAL_TESTS FAIL:${NC} uploads/$TESTFILE encore present sur disque"
fi

# DELETE une seconde fois : fichier deja supprime --> 404
run_test "DELETE /$TESTFILE (deja supprime)" "DELETE /$TESTFILE HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "404"

# DELETE d'un fichier qui n'a jamais existe---> 404
run_test "DELETE /inexistant.txt" "DELETE /inexistant.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "404"





#TESTS METHODES HTTP NON IMPLEMENTEES
# PUT, PATCH, OPTIONS sont des méthodes HTTP valides mais non requises par le sujet
# Le serveur doit répondre 4xx (405 Method Not Allowed idéalement), jamais 200

# PUT : modification de ressource, non implémenté
run_test "PUT /" "PUT / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\nConnection: close\r\n\r\n" "40"

# PATCH : modification partielle, non implémenté
run_test "PATCH /" "PATCH / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\nConnection: close\r\n\r\n" "40"

# OPTIONS : liste des méthodes disponibles, non implémenté
run_test "OPTIONS /" "OPTIONS / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "40"





#TESTS PROTOCOLE HTTP
# HTTP/1.1 oblige le client à envoyer un header Host → son absence doit retourner 400
run_test "GET / sans header Host" "GET / HTTP/1.1\r\nConnection: close\r\n\r\n" "400"

#  le serveur ne doit jamais servir de fichiers hors de son root
# (on accepte n'importe quel 4xx : 400/403/404, l'important c'est que ce ne soit pas 200)
run_test "GET /../etc/passwd (path traversal)" "GET /../etc/passwd HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "40"

#TESTS CGI
# La location /*.bla (root ./) utilise ./cgi_tester comme interpréteur
# cgi_tester lit CONTENT_LENGTH bytes depuis stdin et les retourne dans la réponse

# POST avec body : cgi_tester lit "helloworld" depuis stdin et le retourne en majuscules
# on vérifie à la fois le status 200 ET que "HELLOWORLD" est bien dans le body
run_test_body "POST /YoupiBanane/youpi.bla (CGI body vérifié)" \
    "POST /YoupiBanane/youpi.bla HTTP/1.1\r\nHost: localhost\r\nContent-Length: 10\r\nConnection: close\r\n\r\nhelloworld" \
    "200" "HELLOWORLD"

# POST avec body vide (Content-Length: 0) : cgi_tester ne lit rien -> 200
run_test "POST /YoupiBanane/youpi.bla (body vide)" \
    "POST /YoupiBanane/youpi.bla HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\nConnection: close\r\n\r\n" \
    "200"

# GET sur la location CGI : seul POST est autorisé --> 405
run_test "GET /YoupiBanane/youpi.bla (methode interdite)" \
    "GET /YoupiBanane/youpi.bla HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" \
    "405"

# Script .bla inexistant : le fichier cible n'existe pas ---> 404
run_test "POST /inexistant.bla (script absent)" \
    "POST /inexistant.bla HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhello" \
    "404"







#TESTS STRESS (siege)
# siege -q -b -c N -r R URL : N clients , R répétitions chacun, sans délai
#-q : quiet, sortie JSON uniquement (pas de progress bar)
#-b : benchmark mode, pas de délai entre les requêtes
# c : concurrency, nombre de clients simultanés
# -r : reps, nombre de requêtes par client
# Critère : failed_transactions = 0 (availability 100%)

# Fonction run_siege : parse le JSON que siege -q produit sur stdout
run_siege() {
    local DESCRIPTION="$1"
    local URL="$2"
    local CONCURRENT="$3"
    local REPS="$4"
    local TOTAL_REQ=$((CONCURRENT * REPS))

    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    RESULT=$(siege -q -b -c "$CONCURRENT" -r "$REPS" "$URL" 2>&1)
    FAILED=$(echo "$RESULT" | grep '"failed_transactions"' | grep -oE '[0-9]+')
    AVAIL=$(echo "$RESULT"  | grep '"availability"'        | grep -oE '[0-9]+\.[0-9]+')

    if [ "$FAILED" = "0" ]; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "${GREEN}Test $TOTAL_TESTS OK:${NC} $DESCRIPTION → ${AVAIL}% availability ($TOTAL_REQ req, ${CONCURRENT} concurrent)"
    else
        echo -e "${RED}Test $TOTAL_TESTS FAIL:${NC} $DESCRIPTION → ${AVAIL}% availability, ${FAILED} failed / $TOTAL_REQ"
    fi
}

# 10 clients × 100 requêtes = 1000 GET sur la racine
run_siege "Stress GET / (1000 req)" "http://127.0.0.1:8080/" 10 100

# 5 clients × 50 requêtes = 250 GET sur un fichier dans un alias (YoupiBanane)
run_siege "Stress GET static file (250 req)" "http://127.0.0.1:8080/directory/youpi.bad_extension" 5 50

# 5 clients × 20 requêtes = 100 GET d'une route inexistante :  serveur doit répondre 404 sans crasher
run_siege "Stress GET 404 (100 req)" "http://127.0.0.1:8080/nope" 5 20

# Vérification post-stress :  serveur est toujours en vie et répond normalement
run_test "Serveur alive apres stress" \
    "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "200"

echo -e "\nTests réussis : $PASSED_TESTS / $TOTAL_TESTS"

kill $SERVER_PID