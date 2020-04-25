#!/bin/sh
gcc main.c util/reshttp.c util/serve.c util/headerlist.c -o server
