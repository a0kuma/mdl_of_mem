2029  echo https://raw.githubusercontent.com/pytorch/pytorch/refs/heads/main/torch/utils/viz/MemoryViz.js > source.txt
 2030  ls
 2031  cat source.txt 
 2032  echo https://raw.githubusercontent.com/pytorch/pytorch.github.io/refs/heads/site/memory_viz.html > source_html.txt
 2033  echo {cat source*}
 2034  echo ${cat source*}
 2035  ls
 2036  find . -type f -name source* 
 2037  find . -type f -name 'source* '
 2038  ls
 2039  find . -type f -name 'source*'
 2040  wget -i source.txt 
 2041  wget -i source_html.txt 
 2042  cat *.html
 2043  sed -i 's/https\:\/\/cdn\.jsdelivr\.net\/gh\/pytorch\/pytorch@main\/torch\/utils\/viz\//\./g' memory_viz.html 
 2044  cat *.html
 2045  rm memory_viz.html 
 2046  wget -i source_html.txt 
 2047  sed -i 's/https\:\/\/cdn\.jsdelivr\.net\/gh\/pytorch\/pytorch@main\/torch\/utils\/viz/\./g' memory_viz.html 
 2048  cat *.html
 2049  echo "globalThis.MemoryView = MemoryView;" >> MemoryViz.js 
 2050  cat MemoryViz.js 
 2051  git add -A
 2052  git commit -m vv
 2053  git push
 2054  history

