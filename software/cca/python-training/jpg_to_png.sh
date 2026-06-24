#!/bin/zsh
setopt nullglob
for FILE in *.jpg; do
	magick "$FILE" "${FILE%.jpg}.png"
done
