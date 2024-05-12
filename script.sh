#!/bin/bash
#This script downloads all the images from provided list of websites URLs
download_image() {
    echo "$1"
    # Extract filename and decode it, fname//%/\\x replaces % with \x, %b in printf interprests and decodes sequences \xHH (H - hex number)
    fname=$(echo "$1" | rev | cut -d '/' -f 1 | rev)
    fname=$(printf '%b' "${fname//%/\\x}")
    if [[ -f ${fname} ]]
    then
        echo "Image ${fname} already exists"
    else
        echo "Downloading ${fname}..."
        wget -q "$1"
        echo "Download of ${fname} complete."
    fi
}

# export download_image for parallel
export -f download_image
# list of urls:
urls=()

# Loop through all the links, "$@" - all arguments passed to script
for arg in "$@"
do
    # Extract relative/absolute links, [^"] is matching everything but ", grep -o outputs only the matched part of the line
    for link in $(curl -s ${arg} | grep -o '<img src="[^"]*"' | grep -o '"[^"]*"' | grep -o '[^"]*')
    do
        if [[ "$link" == //* ]]
        then
            # Handle protocol-relative URLs
            url="https:${link}"
        elif [[ "$link" == ../* ]]
        then
            # Handle URLs with .. - go back to previous directory in the website, rev - reverse string
            base=$(echo "$arg" | rev | cut -d '/' -f 3- | rev)
            link=$(echo "$link" | grep -o '[^.].*')
            url="${base}${link}"
        elif [[ "$link" == /* ]]
        then
            # Handle URLs starting with /
            base=$(echo "$arg" | grep -oE '(http|https)://[^/]+' )
            url="${base}${link}"
        else
            url="$link"
        fi

        urls+=("$url")
    done
done

# j flag specifies the number of pictures downloaded at the same time
parallel -j 5 download_image ::: "${urls[@]}"