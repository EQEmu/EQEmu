#!/bin/bash
set -e
export NAME="xmltojson"
echo Building Linux
GOOS=linux GOARCH=amd64 go build -o $NAME-linux-x64 main.go
GOOS=linux GOARCH=386 go build -o $NAME-linux-x86 main.go
echo Building Windows
GOOS=windows GOARCH=amd64 go build -o $NAME-windows-x64.exe main.go
GOOS=windows GOARCH=386 go build -o $NAME-windows-x86.exe main.go
#echo Building OSX
#GOOS=darwin GOARCH=amd64 go build -o $NAME-osx-x64 main.go