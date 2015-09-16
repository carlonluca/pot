#!/bin/bash

for i in *.h; do
	cat copyright.txt "$i" > "$i.new" && mv "$i.new" "$i"
done
