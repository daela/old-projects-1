#!/bin/bash
export WINEPREFIX="$HOME/.wine_bot"
wine ~/.wine_bot/drive_c/Program\ Files/Ventrilo/Ventrilo.exe -cSERVER_IP:SERVER_PORT:SERVER_PASSWORD
