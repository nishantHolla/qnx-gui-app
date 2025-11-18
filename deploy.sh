#!/bin/sh

IP=
OUT=./out/main
DEPLOY_PATH=./main
USERNAME=qnxuser

scp $OUT $USERNAME@$IP:$DEPLOY_PATH
