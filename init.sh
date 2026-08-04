rm -r vv
mkdir vv
find . -type f -name "source*" -exec wget -i {} -P vv \;
cd vv
sed -i 's/https\:\/\/cdn\.jsdelivr\.net\/gh\/pytorch\/pytorch@main\/torch\/utils\/viz/\./g' memory_viz.html 
echo "globalThis.MemoryView = MemoryView;" >> MemoryViz.js 