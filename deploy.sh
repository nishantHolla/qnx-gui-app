#!/bin/sh

IP=192.168.29.162
OUT=./out/main
DEPLOY_PATH=./main
USERNAME=qnxuser

scp $OUT $USERNAME@$IP:$DEPLOY_PATH
