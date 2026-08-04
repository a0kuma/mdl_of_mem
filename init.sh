rm -r vv
mkdir vv
find . -type f -name "source*" -exec wget -i {} -P vv \;
cd vv
sed -i 's/https\:\/\/cdn\.jsdelivr\.net\/gh\/pytorch\/pytorch@main\/torch\/utils\/viz/\./g' memory_viz.html 
echo "globalThis.MemoryView = MemoryView;" >> MemoryViz.js 
echo "globalThis.snapshot_cache = snapshot_cache;" >> MemoryViz.js
echo "globalThis.snapshot_select = snapshot_select;" >> MemoryViz.js
echo 'globalThis.process_alloc_data = process_alloc_data;' >> MemoryViz.js
cp ../vselect.js .
sed -i 's#</body>#<script src="./vselect.js"></script>\n</body>#' memory_viz.html