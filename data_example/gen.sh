echo "# Data Examples"

for d in */;
do
	if [ -d "$d/data" ]; then
    echo "## $d"
		echo '- software: ' $(cat "$d/data/uname")
		echo '- Hardware:'
		echo '```' 
		cat "$d/data/cpuinfo" | head -n 28
		echo '```'
	fi
done
