glslangValidator -H -Os -e main -V -o "%0.spv" "%0"
# spirv-remap -v --do-everything --input "%0.spv" --output .