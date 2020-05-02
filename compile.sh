#!/bin/sh
gcc main.c util/http.c util/serve.c util/headerlist.c -o server
