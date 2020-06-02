#!/bin/sh
cc main.c util/http.c util/serve.c util/httpheaders.c -o server
